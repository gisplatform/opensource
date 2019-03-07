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
 * \file trpc-server.h
 *
 * \author Andrei Fadeev
 * \date 18.03.2014
 * \brief исходный файл класса сервера удалённых вызовов процедур через tcp сокет.
 *
*/

#include "trpc-server.h"
#include "gp-rpc-common.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <string.h>


enum { PROP_O, PROP_URI, PROP_THREADS_NUM, PROP_DATA_SIZE, PROP_DATA_TIMEOUT, PROP_AUTH, PROP_ACL, PROP_MANAGER };


typedef struct TRpcServerPriv {

  gchar           *uri;                        // Адрес сервера для подключения.

  GpRpcAuth *gp_rpc_auth;                  // Аутентификация.
  GpRpcServerAclCallback gp_rpc_acl;           // Проверка доступа.
  GpRpcManager *gp_rpc_manager;               // Функции и объекты.

  GpRpcData **gp_rpc_data;                  // Буферы данных.
  gint            *gp_rpc_data_used;             // Признак использования буфера.

  guint            data_size;                  // Максимальный размер передаваемых данных.
  gdouble          data_timeout;               // Таймаут при передаче данных.

  guint            threads_num;                // Число рабочих потоков.

  GSocket         *socket;                     // Рабочий сокет.
  GSocketService  *listener;

  GThread         *control;                    // Поток управления, для MainLoop.

  volatile guint   prev_client_id;             // Идентификатор последнего, поключившегося клиента.

  volatile gint    started;                    // Признак запуска потока управления.
  volatile gint    close;                      // Завершение работы.

} TRpcServerPriv;

#define TRPC_SERVER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_TRPC_SERVER, TRpcServerPriv ) )


static void trpc_server_interface_init( GpRpcServerInterface *iface );
static void trpc_server_initable_iface_init( gpointer g_iface, gpointer iface_data );
static void trpc_server_class_init( TRpcServerClass *klass );
static void trpc_server_set_property( TRpcServer *trpc, guint prop_id, const GValue *value, GParamSpec *pspec );
static gboolean trpc_server_initable_init( GInitable *initable, GCancellable *cancellable, GError **error );
static void trpc_server_finalize( TRpcServer *trpc );
static gpointer tcp_control( gpointer data );
static gboolean tcp_transporter( GThreadedSocketService *service, GSocketConnection *connection, GObject *source_object, gpointer data );


G_DEFINE_TYPE_EXTENDED( TRpcServer, trpc_server, G_TYPE_INITIALLY_UNOWNED, 0,
    G_IMPLEMENT_INTERFACE( G_TYPE_INITABLE, trpc_server_initable_iface_init )
    G_IMPLEMENT_INTERFACE( G_TYPE_RPC_SERVER, trpc_server_interface_init ) );


static void trpc_server_init( TRpcServer *trpc )
{
  TRpcServerPriv *priv = TRPC_SERVER_GET_PRIVATE( trpc );
  priv->socket = NULL;
}


static void trpc_server_class_init( TRpcServerClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = trpc_server_set_property;
  this_class->finalize = trpc_server_finalize;

  g_object_class_install_property( this_class, PROP_URI,
    g_param_spec_string( "uri", "Uri", "RPC uri", "", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_THREADS_NUM,
    g_param_spec_uint( "threads-num", "Threads num", "RPC threads number", 1, G_MAXUINT, GP_RPC_DEFAULT_THREADS_NUM, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_DATA_SIZE,
    g_param_spec_uint( "data-size", "Data size", "RPC data size buffer", 0, G_MAXUINT, GP_RPC_DEFAULT_DATA_SIZE, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_DATA_TIMEOUT,
    g_param_spec_double( "data-timeout", "Data timeout", "RPC data timeout", 0.1, G_MAXINT, GP_RPC_DEFAULT_DATA_TIMEOUT, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_AUTH,
    g_param_spec_pointer( "auth", "Auth", "RPC authentication object", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_ACL,
    g_param_spec_pointer( "acl", "Acl", "RPC ACL object", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_MANAGER,
    g_param_spec_pointer( "manager", "Manager", "RPC manager object", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof( TRpcServerPriv ) );

}


static void trpc_server_set_property( TRpcServer *trpc, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  TRpcServerPriv *priv = TRPC_SERVER_GET_PRIVATE( trpc );

  switch ( prop_id )
    {

    case PROP_URI:
      priv->uri = g_value_dup_string( value );
      break;

    case PROP_THREADS_NUM:
      priv->threads_num = g_value_get_uint( value );
      break;

    case PROP_DATA_SIZE:
      priv->data_size = g_value_get_uint( value );
      break;

    case PROP_DATA_TIMEOUT:
      priv->data_timeout = g_value_get_double( value );
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
      G_OBJECT_WARN_INVALID_PROPERTY_ID( trpc, prop_id, pspec );
      break;

    }

}


static gboolean trpc_server_initable_init( GInitable *initable, GCancellable *cancellable, GError **error )
{

  g_return_val_if_fail( cancellable == NULL, FALSE );
  g_return_val_if_fail( IS_TRPC_SERVER( initable ), FALSE );

  TRpcServer *trpc = TRPC_SERVER( initable );

  TRpcServerPriv *priv = TRPC_SERVER_GET_PRIVATE( trpc );

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
  priv->socket = NULL;
  priv->listener = NULL;
  priv->started = FALSE;
  priv->close = FALSE;

  // Проверяем типы объектов.
  if( priv->gp_rpc_auth != NULL && !g_type_is_a( G_OBJECT_TYPE( priv->gp_rpc_auth), G_TYPE_RPC_AUTH ) )
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Internal RPC auth parameter error."));
    goto trpc_server_constructor_fail;
  }

  if( priv->gp_rpc_manager == NULL || !g_type_is_a( G_OBJECT_TYPE( priv->gp_rpc_manager), G_TYPE_RPC_MANAGER ) )
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Internal RPC manager parameter error."));
    goto trpc_server_constructor_fail;
  }

  // Проверяем uri.
  if(!g_pattern_match_simple("tcp", gp_rpc_get_transport_type(priv->uri)))
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Failed to find protocol name in URI."));
    goto trpc_server_constructor_fail;
  }

  if(address == NULL)
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Failed to get IP port from URI."));
    goto trpc_server_constructor_fail;
  }

  if(port == 0)
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Failed to get IP port from URI."));
    goto trpc_server_constructor_fail;
  }

  // Создаем рабочий сокет и связываем его с рабочим портом.
  resolver = g_resolver_get_default();
  inet_addresses = g_resolver_lookup_by_name(resolver, address, NULL, &tmp_error);
  if(G_UNLIKELY(tmp_error)) goto trpc_server_constructor_fail;

  socket_address = g_inet_socket_address_new(inet_addresses->data, port);

  priv->socket = g_socket_new(g_socket_address_get_family( socket_address ), G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, &tmp_error);
  if(G_UNLIKELY(tmp_error)) goto trpc_server_constructor_fail;

  g_socket_bind( priv->socket, socket_address, TRUE, &tmp_error);
  if(G_UNLIKELY(tmp_error)) goto trpc_server_constructor_fail;

  g_socket_listen( priv->socket, &tmp_error);
  if(G_UNLIKELY(tmp_error)) goto trpc_server_constructor_fail;

  g_socket_set_blocking(priv->socket, FALSE);

  // Буферы данных;
  priv->gp_rpc_data = g_malloc( priv->threads_num * sizeof( GpRpcData * ) );
  priv->gp_rpc_data_used = g_malloc( priv->threads_num * sizeof( gint ) );
  for( i = 0; i < priv->threads_num; i++ )
    {
    gpointer ibuffer = g_malloc( priv->data_size + GP_RPC_HEADER_SIZE );
    gpointer obuffer = g_malloc( priv->data_size + GP_RPC_HEADER_SIZE );
    priv->gp_rpc_data[i] = gp_rpc_data_new (priv->data_size + GP_RPC_HEADER_SIZE, GP_RPC_HEADER_SIZE, ibuffer, obuffer);
    priv->gp_rpc_data_used[i] = FALSE;
    }

  // Запуск потока управления.
  priv->control = g_thread_new( "tcp server", tcp_control, priv );
  while( !g_atomic_int_get( &priv->started ) && !g_atomic_int_get( &priv->close ) ) g_usleep( 100000 );

  if(!g_atomic_int_get(&priv->started))
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("RPC control thread failed."));
    goto trpc_server_constructor_fail;
  }

  result = TRUE;

  trpc_server_constructor_fail:
    if(G_UNLIKELY(tmp_error)) g_propagate_prefixed_error(error, tmp_error, _("Failed to create RPC server: "));
    g_clear_object(&socket_address);
    if( inet_addresses != NULL ) g_resolver_free_addresses( inet_addresses );
    g_clear_object(&resolver);
    g_free( address );

  return result;

}


static void trpc_server_finalize( TRpcServer *trpc )
{

  TRpcServerPriv *priv = TRPC_SERVER_GET_PRIVATE( trpc );

  guint i;

  g_atomic_int_set( &priv->close, TRUE );

  if( priv->control != NULL )
    g_thread_join( priv->control );

  if( priv->listener != NULL )
    g_object_unref( priv->listener );

  if( priv->socket != NULL )
    g_object_unref( priv->socket );

  if( priv->gp_rpc_data != NULL )
    {
    for( i = 0; i < priv->threads_num; i++ )
      {
      g_free(gp_rpc_data_get_header (priv->gp_rpc_data[i], GP_RPC_DATA_INPUT) );
      g_free(gp_rpc_data_get_header (priv->gp_rpc_data[i], GP_RPC_DATA_OUTPUT) );
      g_object_unref( priv->gp_rpc_data[i] );
      }
    g_free( priv->gp_rpc_data);
    g_free( priv->gp_rpc_data_used);
    }

  g_free( priv->uri );

  G_OBJECT_CLASS( trpc_server_parent_class )->finalize( trpc );

}


// Управляющий поток для MainLoop.
static gpointer tcp_control( gpointer data )
{

  TRpcServerPriv *priv = data;

  GMainContext *context = g_main_context_get_thread_default();

  // Запускаем потоки обработки входящих данных.
  priv->listener = G_SOCKET_SERVICE( g_threaded_socket_service_new( priv->threads_num ) );
  if( priv->listener == NULL )
    { g_atomic_int_set( &priv->close, TRUE ); g_warning( "trpc_server: g_threaded_socket_service_new failed" ); return NULL; }

  g_signal_connect( G_OBJECT( priv->listener ), "run", G_CALLBACK( tcp_transporter ), priv );

  if( !g_socket_listener_add_socket( G_SOCKET_LISTENER( priv->listener ), priv->socket, NULL, NULL ) )
    { g_atomic_int_set( &priv->close, TRUE ); g_warning( "trpc_server: g_socket_listener_add_socket failed" ); return NULL; }

  g_atomic_int_set( &priv->started, TRUE );

  // Обработка событий для g_threaded_socket_service.
  while( g_atomic_int_get( &priv->close ) != TRUE )
    {
    g_usleep( 100000 );
    while( g_main_context_iteration( context, FALSE ) );
    }

  g_atomic_int_set( &priv->started, FALSE );

  return NULL;

}


// Поток приема RPC запросов от клиента.
static gboolean tcp_transporter( GThreadedSocketService *service, GSocketConnection *connection, GObject *source_object, gpointer data )
{

  TRpcServerPriv *priv = data;

  volatile gint *gp_rpc_data_used = NULL;
  guint gp_rpc_data_id;
  GpRpcData *gp_rpc_data = NULL;
  GError *err = NULL;

  guint buffer_size;
  gpointer ibuffer;
  gpointer obuffer;
  GpRpcHeader *oheader;
  GpRpcHeader *iheader;

  GSocket *socket = g_socket_connection_get_socket( connection );

  gssize transmitted;
  gsize buffer_pointer;
  GTimer *timer = g_timer_new();
  GpRpcExecStatus exec_status;

  // Ищем свободный буфер данных.
  for( gp_rpc_data_id = 0; gp_rpc_data_id < priv->threads_num; gp_rpc_data_id++ )
    {
    gp_rpc_data_used = &priv->gp_rpc_data_used[gp_rpc_data_id];
    if( g_atomic_int_compare_and_exchange(gp_rpc_data_used, FALSE, TRUE ) )
      break;
    }
  if( gp_rpc_data_id == priv->threads_num )
    {
    g_warning( "trpc_server: can't find data buffer" );
    goto tcp_transporter_close;
    }

  gp_rpc_data = priv->gp_rpc_data[gp_rpc_data_id];

  gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_INPUT, 0);
  gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);

  iheader = ibuffer = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_INPUT);
  oheader = obuffer = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_OUTPUT);

  oheader->magic = GUINT32_TO_BE(GP_RPC_MAGIC);
  oheader->version = GUINT32_TO_BE(GP_RPC_VERSION);

  while( g_atomic_int_get( &priv->close ) != TRUE )
    {

    // Считываем запрос клиента.
    buffer_size = priv->data_size + GP_RPC_HEADER_SIZE;
    buffer_pointer = 0;
    g_timer_start( timer );

    // Считываем заголовок запроса.
    while( buffer_pointer < GP_RPC_HEADER_SIZE )
      {

      if( !g_socket_condition_timed_wait( socket, G_IO_IN, 100000, NULL, NULL ) ) continue;

      transmitted = g_socket_receive(socket, ibuffer + buffer_pointer, buffer_size - buffer_pointer, NULL, &err);
      if(err) { g_warning("trpc_server: g_socket_receive header failed: %s", err->message); goto tcp_transporter_close; }

      buffer_pointer += transmitted;

      if( g_timer_elapsed( timer, NULL ) > priv->data_timeout )
        {
        g_warning( "trpc_server: data receive timeout" );
        goto tcp_transporter_close;
        }

      }

    if( GUINT32_FROM_BE( iheader->size ) > buffer_size )
      {
      g_warning( "trpc_server: request too large" );
      goto tcp_transporter_close;
      }

    // Считываем тело запроса.
    while( buffer_pointer < GUINT32_FROM_BE( iheader->size ) )
      {

      if( !g_socket_condition_timed_wait( socket, G_IO_IN, 100000, NULL, NULL ) ) continue;

      transmitted = g_socket_receive( socket, ibuffer + buffer_pointer, buffer_size - buffer_pointer, NULL, &err );
      if(err) { g_warning("trpc_server: g_socket_receive body failed: %s", err->message); goto tcp_transporter_close; }

      buffer_pointer += transmitted;

      if( g_timer_elapsed( timer, NULL ) > priv->data_timeout )
        {
        g_warning( "trpc_server: data receive timeout" );
        goto tcp_transporter_close;
        }

      }

      gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_INPUT, buffer_pointer - GP_RPC_HEADER_SIZE);

    // Выполнение запроса.
    exec_status = gp_rpc_server_user_exec (gp_rpc_data, priv->gp_rpc_auth, priv->gp_rpc_acl, priv->gp_rpc_manager);
    if( exec_status == GP_RPC_EXEC_FAIL) { g_warning( "trpc_server: server_exec failed" ); goto tcp_transporter_close; }

    if(iheader->client_id != G_MAXUINT32)
      oheader->client_id = iheader->client_id;
    else
      oheader->client_id = GUINT32_TO_BE(g_atomic_int_add(&priv->prev_client_id, 1));

    // Отправка ответа.
    buffer_size = GUINT32_FROM_BE( oheader->size );
    buffer_pointer = 0;
    g_timer_start( timer );

    while( TRUE )
      {

      if( !g_socket_condition_timed_wait( socket, G_IO_OUT, 100000, NULL, NULL ) ) continue;

      transmitted = g_socket_send( socket, obuffer + buffer_pointer, buffer_size - buffer_pointer, NULL, NULL );
      if( transmitted <= 0 ) { g_warning( "trpc_server: g_socket_send failed" ); goto tcp_transporter_close; }

      buffer_pointer += transmitted;
      if( buffer_pointer >= buffer_size ) break;

      if( g_timer_elapsed( timer, NULL ) > priv->data_timeout )
        {
        g_warning( "trpc_server: data send timeout" );
        goto tcp_transporter_close;
        }

      }

    if( exec_status == GP_RPC_EXEC_CLOSE) break;

    }

tcp_transporter_close:

  g_clear_error(&err);
  g_timer_destroy( timer );

  if( gp_rpc_data != NULL )
    {
      gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_INPUT, 0);
      gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);
    }

  if( gp_rpc_data_id != priv->threads_num && gp_rpc_data_used)
    g_atomic_int_set(gp_rpc_data_used, FALSE );

  return TRUE;

}


gchar *trpc_server_get_self_uri( GpRpcServer *trpc )
{

  TRpcServerPriv *priv = TRPC_SERVER_GET_PRIVATE( trpc );

  GInetAddress *inet_address;
  GSocketAddress *socket_address;

  GSocketFamily family;
  gchar *ip;
  guint16 port;
  gchar *address = NULL;

  if( g_atomic_int_get( &priv->started ) == FALSE ) { g_warning( "trpc_server: server not started" ); return NULL; }

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

static void trpc_server_initable_iface_init( gpointer g_iface, gpointer iface_data )
{

  GInitableIface *iface = (GInitableIface *)g_iface;
  iface->init = trpc_server_initable_init;

}

static void trpc_server_interface_init( GpRpcServerInterface *iface )
{

  iface->get_self_uri = trpc_server_get_self_uri;

}
