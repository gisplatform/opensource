/*
 * GPRPC - rpc (remote procedure call) library, this library is part of GRTL.
 *
 * Copyright 2010 Andrei Fadeev
 *
 * This file is part of GRTL.
 *
 * GRTL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GRTL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GRTL. If not, see <http://www.gnu.org/licenses/>.
 *
*/

/*!
 * \file urpc.c
 *
 * \author Andrei Fadeev
 * \date 20.03.2009
 * \brief Исходный файл класса удалённых вызовов процедур через udp сокет.
 *
*/

#include "urpc.h"
#include "gp-rpc-common.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <math.h> //< Для ceil().
#include <string.h>


enum { PROP_O, PROP_URI, PROP_AUTH, PROP_TIMEOUT, PROP_RESTART, PROP_DATA_SIZE };


typedef struct URpcPriv {

  gchar       *uri;                        // Адрес сервера для подключения.

  GMutex       lock;                       // Блокировка.
  gboolean     lock_mutex;

  GSocket     *socket;                     // Рабочий сокет.
  GpRpcAuth *gp_rpc_auth;                  // Аутентификация.
  GpRpcData *gp_rpc_data;                  // RPC данные.

  gpointer     ibuffer;                    // Буфер входящих данных.
  gpointer     obuffer;                    // Буфер исходящих данных.

  GpRpcHeader   *iheader;                    // Заголовок входящих пакетов.
  GpRpcHeader   *oheader;                    // Заголовок исходящих пакетов.

  GTimer      *timer;                      // Таймер.
  gdouble      timeout;                    // Таймаут.
  guint        restart;                    // Число попыток выполнения запроса.

  guint32      session;                    // Идентификатор сессии.
  guint32      sequence;                   // Идентификатор запроса.

} URpcPriv;

#define URPC_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_URPC, URpcPriv ) )


static void urpc_interface_init( GpRpcInterface *iface );
static void urpc_initable_iface_init( gpointer g_iface, gpointer iface_data );
static void urpc_class_init( URpcClass *klass );
static void urpc_set_property( URpc *urpc, guint prop_id, const GValue *value, GParamSpec *pspec );
static gboolean urpc_initable_init( GInitable *initable, GCancellable *cancellable, GError **error );
static void urpc_finalize( URpc *urpc );
static gboolean urpc_exchange(URpcPriv *priv, GError **error);

GpRpcData *urpc_lock( GpRpc *urpc );
gboolean urpc_exec(GpRpc *urpc, guint32 proc_id, guint32 obj_id, GError **error);
void urpc_unlock( GpRpc *urpc );


G_DEFINE_TYPE_EXTENDED( URpc, urpc, G_TYPE_INITIALLY_UNOWNED, 0,
    G_IMPLEMENT_INTERFACE( G_TYPE_INITABLE, urpc_initable_iface_init )
    G_IMPLEMENT_INTERFACE( G_TYPE_RPC, urpc_interface_init ) );

static void urpc_init( URpc *urpc )
{
  URpcPriv *priv = URPC_GET_PRIVATE( urpc );
  priv->socket = NULL;
}


static void urpc_class_init( URpcClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = urpc_set_property;
  this_class->finalize = urpc_finalize;

  g_object_class_install_property( this_class, PROP_URI,
    g_param_spec_string( "uri", "Uri", "RPC uri", "", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_AUTH,
    g_param_spec_pointer( "auth", "Auth", "RPC authentication object", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_TIMEOUT,
    g_param_spec_double( "exec-timeout", "Exec timeout", "RPC execution timeout", 0.1, G_MAXINT,
                         GP_RPC_DEFAULT_EXEC_TIMEOUT, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_RESTART,
    g_param_spec_uint( "restart", "Request attempts", "Number of RPC request attempts", 1, G_MAXUINT,
                         GP_RPC_DEFAULT_RESTART, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_DATA_SIZE,
    g_param_spec_uint( "data-size", "Data size", "RPC data size buffer", 0, GP_RPC_DEFAULT_DATA_SIZE,
                       GP_RPC_DEFAULT_DATA_SIZE, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof( URpcPriv ) );

}


static void urpc_set_property( URpc *urpc, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  URpcPriv *priv = URPC_GET_PRIVATE( urpc );

  switch ( prop_id )
    {

    case PROP_URI:
      priv->uri = g_value_dup_string( value );
      break;

    case PROP_AUTH:
      priv->gp_rpc_auth = g_value_get_pointer( value );
      break;

    case PROP_TIMEOUT:
      priv->timeout = g_value_get_double( value );
      break;

    case PROP_RESTART:
      priv->restart = g_value_get_uint( value );
      break;

    case PROP_DATA_SIZE:
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( urpc, prop_id, pspec );
      break;

    }

}


static gboolean urpc_initable_init( GInitable *initable, GCancellable *cancellable, GError **error )
{
  g_return_val_if_fail( cancellable == NULL, FALSE );
  g_return_val_if_fail( IS_URPC( initable ), FALSE );

  URpc *urpc = URPC( initable );

  URpcPriv *priv = URPC_GET_PRIVATE( urpc );

  gchar *address = gp_rpc_get_address (priv->uri);
  guint16 port = gp_rpc_get_port (priv->uri);

  GError *tmp_error = NULL;
  GResolver *resolver = NULL;
  GList *inet_addresses = NULL;
  GSocketAddress *socket_address = NULL;
  gboolean result = FALSE;

  // Защита от повторного вызова.
  if( priv->socket != NULL ) return TRUE;

  // Обнуляем внутренние переменные.
  priv->socket = NULL;
  priv->ibuffer = NULL;
  priv->obuffer = NULL;
  priv->timer = g_timer_new();
  priv->session = 0;
  priv->sequence = 1;
  priv->lock_mutex = FALSE;
  g_mutex_init( &priv->lock );

  // Проверяем типы объектов.
  if( priv->gp_rpc_auth != NULL && !g_type_is_a( G_OBJECT_TYPE( priv->gp_rpc_auth), G_TYPE_RPC_AUTH ) )
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Wrong internal auth object."));
    goto urpc_constructor_fail;
  }

  // Проверяем uri.
  if(!g_pattern_match_simple("udp", gp_rpc_get_transport_type(priv->uri)))
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("URI type is not UDP."));
    goto urpc_constructor_fail;
  }
  if(address == NULL || port == 0)
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Fail to get address and port from URI."));
    goto urpc_constructor_fail;
  }

  // Создаем рабочий сокет и подключаемся к серверу.
  resolver = g_resolver_get_default();
  inet_addresses = g_resolver_lookup_by_name(resolver, address, NULL, &tmp_error);
  if(G_UNLIKELY(tmp_error))
    goto urpc_constructor_fail;
  socket_address = g_inet_socket_address_new( inet_addresses->data, port );

  priv->socket = g_socket_new(g_socket_address_get_family(socket_address), G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_UDP, &tmp_error);
  if(G_UNLIKELY(tmp_error))
    goto urpc_constructor_fail;
  g_socket_set_timeout(priv->socket, ceil(priv->timeout));

  g_socket_connect(priv->socket, socket_address, NULL, &tmp_error);
  if(G_UNLIKELY(tmp_error))
    goto urpc_constructor_fail;

  g_socket_set_blocking( priv->socket, FALSE );

  // Буферы приема и передачи
  priv->ibuffer = g_malloc(GP_RPC_DEFAULT_BUFFER_SIZE );
  priv->obuffer = g_malloc(GP_RPC_DEFAULT_BUFFER_SIZE );
  priv->iheader = priv->ibuffer;
  priv->oheader = priv->obuffer;

  priv->gp_rpc_data = gp_rpc_data_new (GP_RPC_DEFAULT_BUFFER_SIZE, GP_RPC_HEADER_SIZE, priv->ibuffer, priv->obuffer);

  priv->oheader->magic = GUINT32_TO_BE(GP_RPC_MAGIC);
  priv->oheader->version = GUINT32_TO_BE(GP_RPC_VERSION);
  priv->oheader->client_id = G_MAXUINT32;

  if(urpc_lock(GP_RPC(urpc)) == NULL)
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Failed to lock RPC."));
    goto urpc_constructor_fail;
  }

  // Выясняем возможности сервера.
  gp_rpc_data_set_uint32 (priv->gp_rpc_data, GP_RPC_PARAM_PROC, GP_RPC_PROC_GET_CAP);
  urpc_exchange(priv, &tmp_error);
  if(G_UNLIKELY(tmp_error))
    goto urpc_constructor_fail_with_unlock;

  g_message("Server assigned id '%u' for us.", GUINT32_FROM_BE(priv->oheader->client_id));

  // Аутентификация.
  if(gp_rpc_data_get_uint32 (priv->gp_rpc_data, GP_RPC_PARAM_NEED_AUTH) == TRUE )
  {

    GpRpcAuthStatus auth_status;

    if(priv->gp_rpc_auth == NULL)
    {
      g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Authentication required."));
      goto urpc_constructor_fail_with_unlock;
    }

    while( TRUE )
    {

      auth_status = gp_rpc_auth_check (priv->gp_rpc_auth, priv->gp_rpc_data);

      if( auth_status == GP_RPC_AUTH_IN_PROGRESS || auth_status == GP_RPC_AUTH_NOT_AUTHENTICATED)
      {
        if( auth_status == GP_RPC_AUTH_IN_PROGRESS)
          { priv->session = GUINT32_FROM_BE( priv->iheader->session ); priv->oheader->session = priv->iheader->session; }

        gp_rpc_data_set_data_size (priv->gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);
        priv->oheader->sequence = GUINT32_TO_BE( priv->sequence );
        gp_rpc_data_set_uint32 (priv->gp_rpc_data, GP_RPC_PARAM_PROC, GP_RPC_PROC_AUTHENTICATE);
        gp_rpc_auth_authenticate (priv->gp_rpc_auth, priv->gp_rpc_data);

        urpc_exchange(priv, &tmp_error);
        if(G_UNLIKELY(tmp_error))
          goto urpc_constructor_fail_with_unlock;
      }
      else break;
    }

    // Ошибка аутентификации
    if(auth_status != GP_RPC_AUTH_AUTHENTICATED)
    {
      g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Authentication required."));
      goto urpc_constructor_fail_with_unlock;
    }
  }

  urpc_unlock( GP_RPC( urpc ) );

  result = TRUE;
  goto urpc_constructor_ok;

urpc_constructor_fail_with_unlock:
  urpc_unlock( GP_RPC( urpc ) );

urpc_constructor_fail:
  g_propagate_error(error, tmp_error);
  g_clear_object(&priv->socket);

urpc_constructor_ok:
  g_clear_object(&socket_address);
  if( inet_addresses != NULL ) g_resolver_free_addresses( inet_addresses );
  g_clear_object(&resolver);
  g_free(address);

  return result;
}


static void urpc_finalize( URpc *urpc )
{
  URpcPriv *priv = URPC_GET_PRIVATE( urpc );

  if( priv->socket != NULL )
    {
    if( urpc_lock( GP_RPC( urpc ) ) != NULL )
      {
      urpc_exec( GP_RPC( urpc ), GP_RPC_PROC_LOGOUT, 0, NULL );
      urpc_unlock( GP_RPC( urpc ) );
      }
    }

  if( priv->socket != NULL )
    g_object_unref( priv->socket );

  g_timer_destroy( priv->timer );

  if( priv->gp_rpc_data != NULL )
    g_object_unref( priv->gp_rpc_data);

  g_free( priv->ibuffer );
  g_free( priv->obuffer );
  g_free( priv->uri );

  if( g_atomic_int_get( &priv->lock_mutex ) )
    g_mutex_unlock( &priv->lock );
  g_mutex_clear( &priv->lock );

  G_OBJECT_CLASS( urpc_parent_class )->finalize( urpc );

}


static gboolean urpc_exchange(URpcPriv *priv, GError **error)
{
  gssize send;
  gssize recieved;
  guint sent_requests = 0;
  guint restart = priv->restart;

  GError *tmp_error = NULL;

  g_timer_start( priv->timer );

  // Первое сообщение шлем однократно, без повторов.
  // Иначе серверу невозможно определить это новый клиент,
  // либо тот же самый послал повторный запрос.
  if(G_UNLIKELY(priv->sequence == 1))
    restart = 1;

  gdouble restart_period = priv->timeout / restart;

  // Отправляем запрос.
  send = gp_rpc_data_get_data_size (priv->gp_rpc_data, GP_RPC_DATA_OUTPUT) + GP_RPC_HEADER_SIZE;
  priv->oheader->size = GUINT32_TO_BE( send );

  while(TRUE)
  {
    while(g_timer_elapsed(priv->timer, NULL) >= restart_period * sent_requests)
    {
      if(sent_requests < restart)
      {
        if(g_socket_send( priv->socket, priv->obuffer, send, NULL, &tmp_error) < 0 )
        {
          g_propagate_prefixed_error(error, tmp_error, _("Socket send failed: "));
          return FALSE;
        }
      }
      else
      {
        g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_TIMEOUT, "UDP exchange timeout.");
        return FALSE;
      }

      sent_requests++;

      if(G_UNLIKELY(sent_requests > 1))
        g_warning("urpc: attempt #%u sequence %u.", sent_requests, priv->sequence);
    }

    // Проверка наличия входных данных.
    if( !g_socket_condition_timed_wait( priv->socket, G_IO_IN, 100000, NULL, NULL ) )
      continue;

    // Чтение пакета.
    recieved = g_socket_receive( priv->socket, priv->ibuffer, GP_RPC_DEFAULT_BUFFER_SIZE, NULL, &tmp_error );
    if( recieved < 0 )
    {
      g_propagate_prefixed_error(error, tmp_error, _("Socket receive failed: "));
      return FALSE;
    }

    if(recieved < GP_RPC_HEADER_SIZE) continue;

    gp_rpc_data_set_data_size(priv->gp_rpc_data, GP_RPC_DATA_INPUT, recieved - GP_RPC_HEADER_SIZE);

    // Проверка заголовка.
    gp_rpc_client_check_header(priv->gp_rpc_data, priv->sequence, &tmp_error);
    if(G_UNLIKELY(tmp_error != NULL))
    {
      const gchar *error_prefix = _("Check of RPC header failed: " );

      if(!g_error_matches(tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_NOT_FOUND))
      {
        g_warning("%s%s", error_prefix, tmp_error->message);
        g_clear_error(&tmp_error);
        continue;
      }
      else
      {
        g_propagate_prefixed_error(error, tmp_error, error_prefix);

        // Счетчик запросов и ID клиента.
        priv->sequence++;
        priv->oheader->client_id = priv->iheader->client_id;

        return FALSE;
      }
    }

    // Счетчик запросов и ID клиента.
    priv->sequence++;
    priv->oheader->client_id = priv->iheader->client_id;

    return TRUE;
  }
}


GpRpcData *urpc_lock( GpRpc *urpc )
{

  URpcPriv *priv = URPC_GET_PRIVATE( urpc );

  if( priv->socket == NULL ) { g_warning( "urpc: client not connected" ); return NULL; }

  g_mutex_lock( &priv->lock );
  g_atomic_int_set( &priv->lock_mutex, TRUE );

  priv->oheader->session = GUINT32_TO_BE( priv->session );
  priv->oheader->sequence = GUINT32_TO_BE( priv->sequence );

  return priv->gp_rpc_data;

}


gboolean urpc_exec(GpRpc *urpc, guint32 proc_id, guint32 obj_id, GError **error)
{
  GError *tmp_error = NULL;
  URpcPriv *priv = URPC_GET_PRIVATE( urpc );

  if(priv->socket == NULL)
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("RPC client is not connected."));
    return FALSE;
  }

  gp_rpc_data_set_uint32 (priv->gp_rpc_data, GP_RPC_PARAM_PROC, proc_id);
  gp_rpc_data_set_uint32 (priv->gp_rpc_data, GP_RPC_PARAM_OBJ, obj_id);

  if( priv->gp_rpc_auth != NULL )
    if( !gp_rpc_auth_authenticate (priv->gp_rpc_auth, priv->gp_rpc_data) )
    {
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Can't authenticate."));
      goto urpc_exec_fail;
    }

  urpc_exchange(priv, &tmp_error);
  if(tmp_error)
  {
    g_propagate_prefixed_error(error, tmp_error, _("Can't execute RPC request: "));

    if(g_error_matches(*error, GP_RPC_ERROR, GP_RPC_ERROR_NOT_FOUND))
      goto urpc_exec_soft_fail;
    else
      goto urpc_exec_fail;
  }

  if( priv->gp_rpc_auth != NULL )
    if(gp_rpc_auth_check (priv->gp_rpc_auth, priv->gp_rpc_data) != GP_RPC_AUTH_AUTHENTICATED)
    {
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Authentication failed."));
      goto urpc_exec_fail;
    }

  return TRUE;

urpc_exec_fail:
  g_clear_object(&priv->socket);

urpc_exec_soft_fail:

  return FALSE;
}


void urpc_unlock( GpRpc *urpc )
{

  URpcPriv *priv = URPC_GET_PRIVATE( urpc );

  if( priv->socket == NULL ) { g_warning( "urpc: client not connected" ); return; }

  gp_rpc_data_set_data_size (priv->gp_rpc_data, GP_RPC_DATA_INPUT, 0);
  gp_rpc_data_set_data_size (priv->gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);

  g_mutex_unlock( &priv->lock );
  g_atomic_int_set( &priv->lock_mutex, FALSE );

}


gboolean urpc_connected( GpRpc *urpc )
{

  URpcPriv *priv = URPC_GET_PRIVATE( urpc );

  return priv->socket ? TRUE : FALSE;

}


gchar *urpc_get_self_uri( GpRpc *urpc )
{

  URpcPriv *priv = URPC_GET_PRIVATE( urpc );

  GInetAddress *inet_address;
  GSocketAddress *socket_address;

  GSocketFamily family;
  gchar *ip;
  guint16 port;
  gchar *address = NULL;

  if( priv->socket == NULL ) { g_warning( "urpc: client not connected" ); return NULL; }

  socket_address = g_socket_get_local_address( priv->socket, NULL );
  if( socket_address != NULL ) inet_address = g_inet_socket_address_get_address( G_INET_SOCKET_ADDRESS( socket_address ) );
  else return NULL;

  family = g_socket_address_get_family( socket_address );
  ip = g_inet_address_to_string( inet_address );
  port = g_inet_socket_address_get_port( G_INET_SOCKET_ADDRESS( socket_address ) );

  if( family == G_SOCKET_FAMILY_IPV4 ) address = g_strdup_printf( "udp://%s:%d", ip, port );
  if( family == G_SOCKET_FAMILY_IPV6 ) address = g_strdup_printf( "udp://[%s]:%d", ip, port );

  g_free( ip );
  g_object_unref( socket_address );

  return address;

}


gchar *urpc_get_server_uri( GpRpc *urpc )
{

  URpcPriv *priv = URPC_GET_PRIVATE( urpc );

  GInetAddress *inet_address;
  GSocketAddress *socket_address;

  GSocketFamily family;
  gchar *ip;
  guint16 port;
  gchar *address = NULL;

  if( priv->socket == NULL ) { g_warning( "urpc: client not connected" ); return NULL; }

  socket_address = g_socket_get_remote_address( priv->socket, NULL );
  if( socket_address != NULL ) inet_address = g_inet_socket_address_get_address( G_INET_SOCKET_ADDRESS( socket_address ) );
  else return NULL;

  family = g_socket_address_get_family( socket_address );
  ip = g_inet_address_to_string( inet_address );
  port = g_inet_socket_address_get_port( G_INET_SOCKET_ADDRESS( socket_address ) );

  if( family == G_SOCKET_FAMILY_IPV4 ) address = g_strdup_printf( "udp://%s:%d", ip, port );
  if( family == G_SOCKET_FAMILY_IPV6 ) address = g_strdup_printf( "udp://[%s]:%d", ip, port );

  g_free( ip );
  g_object_unref( socket_address );

  return address;

}

static void urpc_initable_iface_init( gpointer g_iface, gpointer iface_data )
{

  GInitableIface *iface = (GInitableIface *)g_iface;
  iface->init = urpc_initable_init;

}

static void urpc_interface_init( GpRpcInterface *iface )
{

  iface->lock = urpc_lock;
  iface->exec = urpc_exec;
  iface->unlock = urpc_unlock;
  iface->connected = urpc_connected;
  iface->get_self_uri = urpc_get_self_uri;
  iface->get_server_uri = urpc_get_server_uri;

}
