/*
 * GPRPC - rpc (remote procedure call) library, this library is part of GRTL.
 *
 * Copyright 2014 Andrei Fadeev
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
 * \file urpc-server.h
 *
 * \author Andrei Fadeev
 * \date 07.03.2014
 * \brief исходный файл класса сервера удалённых вызовов процедур через udp сокет.
 *
*/

#include "urpc-server.h"
#include "gp-rpc-common.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <string.h>


enum { PROP_O, PROP_URI, PROP_THREADS_NUM, PROP_DATA_SIZE, PROP_DATA_TIMEOUT, PROP_AUTH, PROP_ACL, PROP_MANAGER };


typedef struct URpcServerPriv {

  gchar       *uri;                        // Адрес сервера для подключения.

  GpRpcAuth *gp_rpc_auth;                  // Аутентификация.
  GpRpcServerAclCallback gp_rpc_acl;       // Проверка доступа.
  GpRpcManager *gp_rpc_manager;               // Функции и объекты.

  guint        threads_num;                // Число рабочих потоков.
  GThread    **threads;                    // Рабочие потоки.

  GSocket     *socket;                     // Рабочий сокет.
  GMutex       socket_mutex;               // Блокировка при приеме.

  GArray      *clients;                    // Массив сохраненных номеров посылок от клиентов.
  GMutex       clients_mutex;              // Мьютекст для массиву номеров посылок от клиентов.

  volatile gint close;                     // Завершение работы.
  volatile guint started;                  // Число запущенных потоков.

} URpcServerPriv;

#define URPC_SERVER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_URPC_SERVER, URpcServerPriv ) )


static void urpc_server_interface_init( GpRpcServerInterface *iface );
static void urpc_server_initable_iface_init( gpointer g_iface, gpointer iface_data );
static void urpc_server_class_init( URpcServerClass *klass );
static void urpc_server_set_property( URpcServer *urpc, guint prop_id, const GValue *value, GParamSpec *pspec );
static gboolean urpc_server_initable_init( GInitable *initable, GCancellable *cancellable, GError **error );
static void urpc_server_finalize( URpcServer *urpc );
static gpointer udp_transporter( gpointer data );

G_DEFINE_TYPE_EXTENDED( URpcServer, urpc_server, G_TYPE_INITIALLY_UNOWNED, 0,
    G_IMPLEMENT_INTERFACE( G_TYPE_INITABLE, urpc_server_initable_iface_init )
    G_IMPLEMENT_INTERFACE( G_TYPE_RPC_SERVER, urpc_server_interface_init ) );

static void urpc_server_init( URpcServer *urpc )
{
  URpcServerPriv *priv = URPC_SERVER_GET_PRIVATE( urpc );
  priv->socket = NULL;
}

static void urpc_server_class_init( URpcServerClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = urpc_server_set_property;
  this_class->finalize = urpc_server_finalize;

  g_object_class_install_property( this_class, PROP_URI,
    g_param_spec_string( "uri", "Uri", "RPC uri", "", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_THREADS_NUM,
    g_param_spec_uint( "threads-num", "Threads num", "RPC threads number", 1, G_MAXUINT, GP_RPC_DEFAULT_THREADS_NUM, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_DATA_SIZE,
    g_param_spec_uint( "data-size", "Data size", "RPC data size buffer", 0, GP_RPC_DEFAULT_DATA_SIZE,
                       GP_RPC_DEFAULT_DATA_SIZE, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_DATA_TIMEOUT,
    g_param_spec_double( "data-timeout", "Data timeout", "RPC data timeout", 0.1, G_MAXINT, GP_RPC_DEFAULT_DATA_TIMEOUT, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_AUTH,
    g_param_spec_pointer( "auth", "Auth", "RPC authentication object", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_ACL,
    g_param_spec_pointer( "acl", "Acl", "RPC ACL object", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_MANAGER,
    g_param_spec_pointer( "manager", "Manager", "RPC manager object", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof( URpcServerPriv ) );

}


static void urpc_server_set_property( URpcServer *urpc, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  URpcServerPriv *priv = URPC_SERVER_GET_PRIVATE( urpc );

  switch ( prop_id )
    {

    case PROP_URI:
      priv->uri = g_value_dup_string( value );
      break;

    case PROP_THREADS_NUM:
      priv->threads_num = g_value_get_uint( value );
      break;

    case PROP_DATA_SIZE:
      break;

    case PROP_DATA_TIMEOUT:
      break;

    case PROP_AUTH:
      priv->gp_rpc_auth = g_value_get_pointer( value );
      break;

    case PROP_ACL:
      priv->gp_rpc_acl = g_value_get_pointer( value );
      break;

    case PROP_MANAGER:
      priv->gp_rpc_manager = g_value_get_pointer( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( urpc, prop_id, pspec );
      break;

    }

}


static gboolean urpc_server_initable_init( GInitable *initable, GCancellable *cancellable, GError **error )
{

  g_return_val_if_fail( cancellable == NULL, FALSE );
  g_return_val_if_fail( IS_URPC_SERVER( initable ), FALSE );

  URpcServer *urpc = URPC_SERVER( initable );

  URpcServerPriv *priv = URPC_SERVER_GET_PRIVATE( urpc );

  gchar *address = gp_rpc_get_address (priv->uri);
  guint16 port = gp_rpc_get_port (priv->uri);

  GResolver *resolver = NULL;
  GList *inet_addresses = NULL;
  GSocketAddress *socket_address = NULL;
  GError *tmp_error = NULL;
  gboolean result = FALSE;

  guint i;

  // Защита от повторного вызова.
  if( priv->socket != NULL ) return TRUE;

  // Обнуляем внутренние переменные.
  priv->threads = NULL;
  priv->socket = NULL;
  priv->close = FALSE;
  g_mutex_init( &priv->socket_mutex );
  priv->clients = g_array_new(FALSE, TRUE, sizeof(guint32));
  g_mutex_init( &priv->clients_mutex );

  // Проверяем типы объектов.
  if( priv->gp_rpc_auth != NULL && !g_type_is_a( G_OBJECT_TYPE( priv->gp_rpc_auth), G_TYPE_RPC_AUTH ) )
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Internal RPC auth parameter error."));
    goto urpc_server_constructor_fail;
  }

  if( priv->gp_rpc_manager == NULL || !g_type_is_a( G_OBJECT_TYPE( priv->gp_rpc_manager), G_TYPE_RPC_MANAGER ) )
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Internal RPC manager parameter error."));
    goto urpc_server_constructor_fail;
  }

  // Проверяем uri.
  if(!g_pattern_match_simple( "udp", gp_rpc_get_transport_type (priv->uri)))
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Failed to find protocol name in URI."));
    goto urpc_server_constructor_fail;
  }

  if(address == NULL)
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Failed to get IP address from URI."));
    goto urpc_server_constructor_fail;
  }

  if(port == 0)
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Failed to get IP port from URI."));
    goto urpc_server_constructor_fail;
  }

  // Создаем рабочий сокет и связываем его с рабочим портом.
  resolver = g_resolver_get_default();
  inet_addresses = g_resolver_lookup_by_name(resolver, address, NULL, &tmp_error);
  if(G_UNLIKELY(tmp_error)) goto urpc_server_constructor_fail;

  socket_address = g_inet_socket_address_new(inet_addresses->data, port);

  priv->socket = g_socket_new(g_socket_address_get_family( socket_address ), G_SOCKET_TYPE_DATAGRAM, G_SOCKET_PROTOCOL_UDP, &tmp_error);
  if(G_UNLIKELY(tmp_error)) goto urpc_server_constructor_fail;

  g_socket_bind( priv->socket, socket_address, FALSE, &tmp_error);
  if(G_UNLIKELY(tmp_error)) goto urpc_server_constructor_fail;

  g_socket_set_blocking( priv->socket, FALSE );

  // Запускаем потоки обработки входящих данных.
  priv->started = 0;
  priv->threads = g_malloc0( priv->threads_num * sizeof( GThread* ) );
  for( i = 0; i < priv->threads_num; i++ )
    priv->threads[ i ] = g_thread_new( "udp server", udp_transporter, priv );

  // Ожидаем запуска всех потоков.
  while( g_atomic_int_get( &priv->started ) != priv->threads_num ) g_usleep( 100 );

  result = TRUE;

  urpc_server_constructor_fail:
    if(G_UNLIKELY(tmp_error)) g_propagate_prefixed_error(error, tmp_error, _("Failed to create RPC server: "));
    if( socket_address != NULL ) g_object_unref( socket_address );
    if( inet_addresses != NULL ) g_resolver_free_addresses( inet_addresses );
    if( resolver != NULL ) g_object_unref( resolver );
    g_free( address );

  return result;

}


static void urpc_server_finalize( URpcServer *urpc )
{
  URpcServerPriv *priv = URPC_SERVER_GET_PRIVATE( urpc );

  gint i;

  g_atomic_int_set( &priv->close, TRUE );

  if( priv->threads != NULL )
    for( i = 0; i < priv->threads_num; i++ )
      g_thread_join( priv->threads[i] );

  g_free( priv->threads );

  if( priv->socket != NULL )
    g_object_unref( priv->socket );

  g_mutex_clear(&priv->socket_mutex);
  g_array_free(priv->clients, TRUE);
  g_mutex_clear(&priv->clients_mutex);

  g_free( priv->uri );

  G_OBJECT_CLASS( urpc_server_parent_class )->finalize( urpc );

}


// Поток приема RPC запросов от клиента.
static gpointer udp_transporter( gpointer data )
{

  URpcServerPriv *priv = data;

  gpointer ibuffer = g_malloc(GP_RPC_DEFAULT_BUFFER_SIZE );
  gpointer obuffer = g_malloc(GP_RPC_DEFAULT_BUFFER_SIZE );

  GpRpcHeader *iheader = ibuffer;
  GpRpcHeader *oheader = obuffer;

  GpRpcData *gp_rpc_data = gp_rpc_data_new (GP_RPC_DEFAULT_BUFFER_SIZE, GP_RPC_HEADER_SIZE, ibuffer, obuffer);

  GSocketAddress *client_address;
  gssize recieved;
  GError *error = NULL;

  guint32 sequence, client_id;

  oheader->magic = GUINT32_TO_BE(GP_RPC_MAGIC);
  oheader->version = GUINT32_TO_BE(GP_RPC_VERSION);

  g_atomic_int_add( &priv->started, 1 );

  while( g_atomic_int_get( &priv->close ) != TRUE )
  {
    // Проверка наличия входных данных.
    if( !g_socket_condition_timed_wait( priv->socket, G_IO_IN, 100000, NULL, NULL ) )
      continue;

    // Чтение пакета.
    g_mutex_lock( &priv->socket_mutex );
    recieved = g_socket_receive_from( priv->socket, &client_address, ibuffer, GP_RPC_DEFAULT_BUFFER_SIZE, NULL, &error );
    g_mutex_unlock( &priv->socket_mutex );
    if(recieved < 0 && !g_error_matches(error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK))
      { g_error_free( error ); g_atomic_int_set( &priv->close, TRUE ); g_warning( "urpc_server: g_socket_receive failed" ); break; }
    else if( error != NULL ) { g_error_free( error ); error = NULL; continue; }
    if( recieved < GP_RPC_HEADER_SIZE ) continue;
      gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_INPUT, recieved - GP_RPC_HEADER_SIZE);

    sequence = GUINT32_FROM_BE(iheader->sequence);
    client_id = GUINT32_FROM_BE(iheader->client_id);

    g_mutex_lock(&priv->clients_mutex);

    if(G_UNLIKELY(client_id == G_MAXUINT32)) //< Новый клиент.
    {
      client_id = priv->clients->len;
      oheader->client_id = GUINT32_TO_BE(priv->clients->len);
      g_array_set_size(priv->clients, priv->clients->len + 1);
    }
    else //< Старый клиент.
      oheader->client_id = iheader->client_id;


    // Клиент с ошибочным id, которого нет в массиве клиентов.
    if(G_UNLIKELY(priv->clients->len == 0 || client_id > priv->clients->len - 1))
    {
      g_mutex_unlock(&priv->clients_mutex);

      g_critical("urpc_server: came client_id %u (but we have only %u connected clients)",
        client_id, priv->clients->len);

      // Может дело в версии протокола?
      if((GUINT32_FROM_BE(iheader->version) >> 16 ) != (GP_RPC_VERSION >>16))
        g_warning("GPRPC version mismatch, server version %u.%u, but came  %u.%u",
          GP_RPC_VERSION >> 16,                    GP_RPC_VERSION & 0xFFFF,
          GUINT32_FROM_BE(iheader->version) >> 16, GUINT32_FROM_BE(iheader->version) & 0xFFFF);

      continue;
    }

    // В лучшем случае мы можем дать ответ на предшествующий запрос, более старые -- отбрасываем.
    if(sequence < g_array_index(priv->clients, guint32, client_id))
    {
      g_warning("urpc_server: sequence %u came after %u", sequence, g_array_index(priv->clients, guint32, client_id));
      g_mutex_unlock(&priv->clients_mutex);
      continue;
    }


    // Выполнение запроса.
    // Нужно выполнять только новые запросы, на предыдущие уже есть готовый ответ.
    if(sequence > g_array_index(priv->clients, guint32, client_id))
    {
      g_array_index(priv->clients, guint32, client_id) = sequence;
      g_mutex_unlock(&priv->clients_mutex);

      if(gp_rpc_server_user_exec (gp_rpc_data, priv->gp_rpc_auth, priv->gp_rpc_acl, priv->gp_rpc_manager) == GP_RPC_EXEC_FAIL)
        { g_warning( "urpc_server: server_exec failed" ); continue; }
    }
    else
      g_mutex_unlock(&priv->clients_mutex);


    // Отправка ответа.
    g_socket_send_to( priv->socket, client_address, obuffer, GUINT32_FROM_BE( oheader->size ), NULL, NULL );
    g_object_unref( client_address );
  }

  g_object_unref(gp_rpc_data);
  g_free( ibuffer );
  g_free( obuffer );

  return NULL;

}


gchar *urpc_server_get_self_uri( GpRpcServer *urpc )
{

  URpcServerPriv *priv = URPC_SERVER_GET_PRIVATE( urpc );

  GInetAddress *inet_address;
  GSocketAddress *socket_address;

  GSocketFamily family;
  gchar *ip;
  guint16 port;
  gchar *address = NULL;

  if( g_atomic_int_get( &priv->close ) == TRUE ) { g_warning( "urpc_server: server not started" ); return NULL; }

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

static void urpc_server_initable_iface_init( gpointer g_iface, gpointer iface_data )
{

  GInitableIface *iface = (GInitableIface *)g_iface;
  iface->init = urpc_server_initable_init;

}

static void urpc_server_interface_init( GpRpcServerInterface *iface )
{

  iface->get_self_uri = urpc_server_get_self_uri;

}
