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
 * \file trpc.c
 *
 * \author Andrei Fadeev
 * \date 20.03.2009
 * \brief Исходный файл класса удалённых вызовов процедур через tcp сокет.
 *
*/

#include "trpc.h"
#include "gp-rpc-common.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <math.h> //< Для ceil().
#include <string.h>


enum { PROP_O, PROP_URI, PROP_AUTH, PROP_TIMEOUT, PROP_RESTART, PROP_DATA_SIZE };


typedef struct TRpcPriv {

  gchar       *uri;                        // Адрес сервера для подключения.

  GMutex       lock;                       // Блокировка.

  GSocket     *socket;                     // Рабочий сокет.
  GpRpcAuth   *gp_rpc_auth;                // Аутентификация.
  GpRpcData   *gp_rpc_data;                // RPC данные.

  guint        data_size;                  // Максимальный размер передаваемых данных.

  gpointer     ibuffer;                    // Буфер входящих данных.
  gpointer     obuffer;                    // Буфер исходящих данных.

  GpRpcHeader   *iheader;                    // Заголовок входящих пакетов.
  GpRpcHeader   *oheader;                    // Заголовок исходящих пакетов.

  GTimer      *timer;                      // Таймер.
  gdouble      timeout;                    // Таймаут.

  guint32      session;                    // Идентификатор сессии.
  guint32      sequence;                   // Идентификатор запроса.

} TRpcPriv;

#define TRPC_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_TRPC, TRpcPriv ) )


static void trpc_interface_init( GpRpcInterface *iface );
static void trpc_initable_iface_init( gpointer g_iface, gpointer iface_data );
static void trpc_class_init( TRpcClass *klass );
static void trpc_set_property( TRpc *trpc, guint prop_id, const GValue *value, GParamSpec *pspec );
static gboolean trpc_initable_init( GInitable *initable, GCancellable *cancellable, GError **error );
static void trpc_finalize( TRpc *trpc );
static gboolean trpc_exchange(TRpcPriv *priv, GError **error);

GpRpcData *trpc_lock( GpRpc *trpc );
gboolean trpc_exec(GpRpc *trpc, guint32 proc_id, guint32 obj_id, GError **error);
void trpc_unlock( GpRpc *trpc );

G_DEFINE_TYPE_EXTENDED( TRpc, trpc, G_TYPE_INITIALLY_UNOWNED, 0,
    G_IMPLEMENT_INTERFACE( G_TYPE_INITABLE, trpc_initable_iface_init )
    G_IMPLEMENT_INTERFACE( G_TYPE_RPC, trpc_interface_init ) );


static void trpc_init( TRpc *trpc )
{
  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );
  priv->socket = NULL;
}


static void trpc_class_init( TRpcClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = trpc_set_property;
  this_class->finalize = trpc_finalize;

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
    g_param_spec_uint( "data-size", "Data size", "RPC data size buffer", 0, G_MAXUINT, GP_RPC_DEFAULT_DATA_SIZE, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof( TRpcPriv ) );

}


static void trpc_set_property( TRpc *trpc, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );

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
      break;

    case PROP_DATA_SIZE:
      priv->data_size = g_value_get_uint( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( trpc, prop_id, pspec );
      break;

    }

}


static gboolean trpc_initable_init( GInitable *initable, GCancellable *cancellable, GError **error )
{

  g_return_val_if_fail( cancellable == NULL, FALSE );
  g_return_val_if_fail( IS_TRPC( initable ), FALSE );

  TRpc *trpc = TRPC( initable );

  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );

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
  g_mutex_init( &priv->lock );

  // Проверяем типы объектов.
  if(priv->gp_rpc_auth != NULL && !g_type_is_a(G_OBJECT_TYPE(priv->gp_rpc_auth), G_TYPE_RPC_AUTH))
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Wrong internal auth object."));
    goto trpc_constructor_fail;
  }

  // Проверяем uri.
  if(!g_pattern_match_simple("tcp", gp_rpc_get_transport_type(priv->uri)))
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("URI type is not TCP."));
    goto trpc_constructor_fail;
  }
  if(address == NULL || port == 0)
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Fail to get address and port from URI."));
    goto trpc_constructor_fail;
  }

  // Создаем рабочий сокет и подключаемся к серверу.
  resolver = g_resolver_get_default();
  inet_addresses = g_resolver_lookup_by_name(resolver, address, NULL, &tmp_error);
  if(G_UNLIKELY(tmp_error))
    goto trpc_constructor_fail;
  socket_address = g_inet_socket_address_new( inet_addresses->data, port );

  priv->socket = g_socket_new(g_socket_address_get_family(socket_address), G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, &tmp_error);
  if(G_UNLIKELY(tmp_error))
    goto trpc_constructor_fail;
  g_socket_set_timeout(priv->socket, ceil(priv->timeout));

  if(!g_socket_connect(priv->socket, socket_address, NULL, &tmp_error))
  if(G_UNLIKELY(tmp_error))
    goto trpc_constructor_fail;

  g_socket_set_blocking( priv->socket, FALSE );

  // Буферы приема и передачи
  priv->ibuffer = g_malloc( priv->data_size + GP_RPC_HEADER_SIZE );
  priv->obuffer = g_malloc( priv->data_size + GP_RPC_HEADER_SIZE );
  priv->iheader = priv->ibuffer;
  priv->oheader = priv->obuffer;

  priv->gp_rpc_data = gp_rpc_data_new (priv->data_size + GP_RPC_HEADER_SIZE, GP_RPC_HEADER_SIZE, priv->ibuffer,
                                     priv->obuffer);

  priv->oheader->magic = GUINT32_TO_BE(GP_RPC_MAGIC);
  priv->oheader->version = GUINT32_TO_BE(GP_RPC_VERSION);
  priv->oheader->client_id = G_MAXUINT32;

  if(trpc_lock(GP_RPC(trpc)) == NULL)
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Failed to lock RPC."));
    goto trpc_constructor_fail;
  }

  // Выясняем возможности сервера.
  gp_rpc_data_set_uint32 (priv->gp_rpc_data, GP_RPC_PARAM_PROC, GP_RPC_PROC_GET_CAP);
  trpc_exchange(priv, &tmp_error);
  if(G_UNLIKELY(tmp_error))
    goto trpc_constructor_fail_with_unlock;

  // Аутентификация.
  if(gp_rpc_data_get_uint32 (priv->gp_rpc_data, GP_RPC_PARAM_NEED_AUTH) == TRUE )
  {

    GpRpcAuthStatus auth_status;

    if( priv->gp_rpc_auth == NULL )
    {
      g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Authentication required."));
      goto trpc_constructor_fail_with_unlock;
    }

    while( TRUE )
    {

      auth_status = gp_rpc_auth_check (priv->gp_rpc_auth, priv->gp_rpc_data);

      if(auth_status == GP_RPC_AUTH_IN_PROGRESS || auth_status == GP_RPC_AUTH_NOT_AUTHENTICATED)
      {
        if(auth_status == GP_RPC_AUTH_IN_PROGRESS)
        {
          priv->session = GUINT32_FROM_BE( priv->iheader->session );
          priv->oheader->session = priv->iheader->session;
        }

        gp_rpc_data_set_data_size (priv->gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);
        priv->oheader->sequence = GUINT32_TO_BE( priv->sequence );
        gp_rpc_data_set_uint32 (priv->gp_rpc_data, GP_RPC_PARAM_PROC, GP_RPC_PROC_AUTHENTICATE);
        gp_rpc_auth_authenticate (priv->gp_rpc_auth, priv->gp_rpc_data);

        trpc_exchange(priv, &tmp_error);
        if(G_UNLIKELY(tmp_error))
          goto trpc_constructor_fail_with_unlock;
      }
      else
        break;
    }

    // Ошибка аутентификации
    if(auth_status != GP_RPC_AUTH_AUTHENTICATED)
    {
      g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Authentication required."));
      goto trpc_constructor_fail_with_unlock;
    }
  }

  trpc_unlock( GP_RPC( trpc ) );

  result = TRUE;
  goto trpc_constructor_ok;

trpc_constructor_fail_with_unlock:
  trpc_unlock(GP_RPC(trpc));

trpc_constructor_fail:
  g_propagate_error(error, tmp_error);
  g_clear_object(&priv->socket);

trpc_constructor_ok:
  if( socket_address != NULL ) g_object_unref( socket_address );
  if( inet_addresses != NULL ) g_resolver_free_addresses( inet_addresses );
  if( resolver != NULL ) g_object_unref( resolver );
  g_free( address );

  return result;

}


static void trpc_finalize( TRpc *trpc )
{

  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );

  if( priv->socket != NULL )
    {
    if( trpc_lock( GP_RPC( trpc ) ) != NULL )
      {
      trpc_exec( GP_RPC( trpc ), GP_RPC_PROC_LOGOUT, 0, NULL );
      trpc_unlock( GP_RPC( trpc ) );
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

  g_mutex_clear( &priv->lock );

  G_OBJECT_CLASS( trpc_parent_class )->finalize( trpc );

}


static gboolean trpc_exchange(TRpcPriv *priv, GError **error)
{

  guint buffer_size;
  gssize transmitted;
  gsize buffer_pointer;
  GError *tmp_error = NULL;

  g_timer_start( priv->timer );

  // Отправляем запрос.
  buffer_size = gp_rpc_data_get_data_size (priv->gp_rpc_data, GP_RPC_DATA_OUTPUT) + GP_RPC_HEADER_SIZE;
  buffer_pointer = 0;
  priv->oheader->size = GUINT32_TO_BE( buffer_size );

  while( buffer_pointer < buffer_size )
  {
    if( g_socket_condition_timed_wait( priv->socket, G_IO_OUT | G_IO_ERR | G_IO_HUP, 100000, NULL, NULL ) )
    {
      transmitted = g_socket_send(priv->socket, priv->obuffer + buffer_pointer, buffer_size - buffer_pointer, NULL, &tmp_error);
      if(G_UNLIKELY(tmp_error))
      {
        g_propagate_prefixed_error(error, tmp_error, _("Failed to send RPC data: "));
        return FALSE;
      }

      buffer_pointer += transmitted;
    }

    if( g_timer_elapsed( priv->timer, NULL ) > priv->timeout )
    {
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_TIMEOUT, _("RPC data send timeout."));
      return FALSE;
    }
  }

  // Считываем ответ сервера.
  buffer_size = priv->data_size + GP_RPC_HEADER_SIZE;
  buffer_pointer = 0;

  // Считываем заголовок ответа.
  while( buffer_pointer < GP_RPC_HEADER_SIZE )
  {
    if( g_socket_condition_timed_wait( priv->socket, G_IO_IN | G_IO_ERR | G_IO_HUP, 100000, NULL, NULL ) )
    {
      transmitted = g_socket_receive(priv->socket, priv->ibuffer + buffer_pointer, buffer_size - buffer_pointer, NULL, &tmp_error);
      if(tmp_error)
      {
        g_propagate_prefixed_error(error, tmp_error, _("Failed to receive RPC header: "));
        return FALSE;
      }

      buffer_pointer += transmitted;
    }

    if(g_timer_elapsed( priv->timer, NULL ) > priv->timeout)
    {
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_TIMEOUT, _("RPC header receive timeout."));
      return FALSE;
    }
  }

  if(GUINT32_FROM_BE( priv->iheader->size ) > buffer_size)
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("RPC response too large."));
    return FALSE;
  }

  // Считываем тело ответа.
  while( buffer_pointer < GUINT32_FROM_BE( priv->iheader->size ) )
  {
    if( g_socket_condition_timed_wait( priv->socket, G_IO_IN | G_IO_ERR | G_IO_HUP, 100000, NULL, NULL ) )
    {
      transmitted = g_socket_receive(priv->socket, priv->ibuffer + buffer_pointer, buffer_size - buffer_pointer, NULL, &tmp_error);
      if(tmp_error)
      {
        g_propagate_prefixed_error(error, tmp_error, _("Failed to receive RPC data: "));
        return FALSE;
      }

      buffer_pointer += transmitted;
    }

    if(g_timer_elapsed( priv->timer, NULL ) > priv->timeout)
    {
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_TIMEOUT, _("RPC data receive timeout."));
      return FALSE;
    }
  }

  gp_rpc_data_set_data_size (priv->gp_rpc_data, GP_RPC_DATA_INPUT, buffer_pointer - GP_RPC_HEADER_SIZE);

  // Проверка заголовка.
  gp_rpc_client_check_header(priv->gp_rpc_data, priv->sequence, &tmp_error);
  if(G_UNLIKELY(tmp_error))
  {
    g_propagate_prefixed_error(error, tmp_error, _("Check of RPC header failed: " ));

    if(g_error_matches(tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_NOT_FOUND))
    {
      // Счетчик запросов и ID клиента.
      priv->sequence++;
      priv->oheader->client_id = priv->iheader->client_id;
    }

    return FALSE;
  }

  // Счетчик запросов и ID клиента.
  priv->sequence++;
  priv->oheader->client_id = priv->iheader->client_id;

  return TRUE;
}


GpRpcData *trpc_lock( GpRpc *trpc )
{

  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );

  if( priv->socket == NULL || !g_socket_is_connected( priv->socket ) ) { g_warning( "trpc: client not connected" ); return NULL; }

  g_mutex_lock( &priv->lock );

  priv->oheader->session = GUINT32_TO_BE( priv->session );
  priv->oheader->sequence = GUINT32_TO_BE( priv->sequence );

  return priv->gp_rpc_data;

}


gboolean trpc_exec(GpRpc *trpc, guint32 proc_id, guint32 obj_id, GError **error)
{
  GError *tmp_error = NULL;
  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );

  if(priv->socket == NULL || !g_socket_is_connected(priv->socket))
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
        goto trpc_exec_fail;
      }

  trpc_exchange( priv, &tmp_error);
  if(tmp_error)
  {
    g_propagate_prefixed_error(error, tmp_error, _("Can't execute RPC request: "));

    if(g_error_matches(*error, GP_RPC_ERROR, GP_RPC_ERROR_NOT_FOUND))
      goto trpc_exec_soft_fail;
    else
      goto trpc_exec_fail;
  }

  if( priv->gp_rpc_auth != NULL )
    if(gp_rpc_auth_check (priv->gp_rpc_auth, priv->gp_rpc_data) != GP_RPC_AUTH_AUTHENTICATED)
    {
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Authentication failed."));
      goto trpc_exec_fail;
    }

  return TRUE;


trpc_exec_fail:
  g_clear_object(&priv->socket);

trpc_exec_soft_fail:

  return FALSE;
}


void trpc_unlock( GpRpc *trpc )
{

  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );

  if( priv->socket == NULL || !g_socket_is_connected( priv->socket ) ) { g_warning( "trpc: client not connected" ); return; }

  gp_rpc_data_set_data_size (priv->gp_rpc_data, GP_RPC_DATA_INPUT, 0);
  gp_rpc_data_set_data_size (priv->gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);

  g_mutex_unlock( &priv->lock );

}


gboolean trpc_connected( GpRpc *trpc )
{

  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );

  return priv->socket ? TRUE : FALSE;

}


gchar *trpc_get_self_uri( GpRpc *trpc )
{

  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );

  GInetAddress *inet_address;
  GSocketAddress *socket_address;

  GSocketFamily family;
  gchar *ip;
  guint16 port;
  gchar *address = NULL;

  if( priv->socket == NULL ) { g_warning( "trpc: client not connected" ); return NULL; }

  socket_address = g_socket_get_local_address( priv->socket, NULL );
  if( socket_address != NULL ) inet_address = g_inet_socket_address_get_address( G_INET_SOCKET_ADDRESS( socket_address ) );
  else return NULL;

  family = g_socket_address_get_family( socket_address );
  ip = g_inet_address_to_string( inet_address );
  port = g_inet_socket_address_get_port( G_INET_SOCKET_ADDRESS( socket_address ) );

  if( family == G_SOCKET_FAMILY_IPV4 ) address = g_strdup_printf( "tcp://%s:%d", ip, port );
  if( family == G_SOCKET_FAMILY_IPV6 ) address = g_strdup_printf( "tcp://[%s]:%d", ip, port );

  g_free( ip );
  g_object_unref( socket_address );

  return address;

}


gchar *trpc_get_server_uri( GpRpc *trpc )
{

  TRpcPriv *priv = TRPC_GET_PRIVATE( trpc );

  GInetAddress *inet_address;
  GSocketAddress *socket_address;

  GSocketFamily family;
  gchar *ip;
  guint16 port;
  gchar *address = NULL;

  if( priv->socket == NULL ) { g_warning( "trpc: client not connected" ); return NULL; }

  socket_address = g_socket_get_remote_address( priv->socket, NULL );
  if( socket_address != NULL ) inet_address = g_inet_socket_address_get_address( G_INET_SOCKET_ADDRESS( socket_address ) );
  else return NULL;

  family = g_socket_address_get_family( socket_address );
  ip = g_inet_address_to_string( inet_address );
  port = g_inet_socket_address_get_port( G_INET_SOCKET_ADDRESS( socket_address ) );

  if( family == G_SOCKET_FAMILY_IPV4 ) address = g_strdup_printf( "tcp://%s:%d", ip, port );
  if( family == G_SOCKET_FAMILY_IPV6 ) address = g_strdup_printf( "tcp://[%s]:%d", ip, port );

  g_free( ip );
  g_object_unref( socket_address );

  return address;

}

static void trpc_initable_iface_init( gpointer g_iface, gpointer iface_data )
{

  GInitableIface *iface = (GInitableIface *)g_iface;
  iface->init = trpc_initable_init;

}

static void trpc_interface_init( GpRpcInterface *iface )
{

  iface->lock = trpc_lock;
  iface->exec = trpc_exec;
  iface->unlock = trpc_unlock;
  iface->connected = trpc_connected;
  iface->get_self_uri = trpc_get_self_uri;
  iface->get_server_uri = trpc_get_server_uri;

}
