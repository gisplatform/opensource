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
 * \file srpc-server.h
 *
 * \author Andrei Fadeev
 * \date 07.03.2014
 * \brief исходный файл класса сервера удалённых вызовов процедур через разделяемую память.
 *
*/

#include "srpc-server.h"
#include "srpc-common.h"
#include "gp-rpc-common.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <string.h>


enum { PROP_O, PROP_URI, PROP_THREADS_NUM, PROP_DATA_SIZE, PROP_DATA_TIMEOUT, PROP_AUTH, PROP_ACL, PROP_MANAGER };


typedef struct SRpcServerPriv {

  gchar          *uri;                   // Адрес сервера для подключения.

  GpRpcAuth      *gp_rpc_auth;           // Аутентификация.
  GpRpcServerAclCallback gp_rpc_acl;     // Проверка доступа.
  GpRpcManager   *gp_rpc_manager;        // Функции и объекты.

  guint           data_size;             // Максимальный размер передаваемых данных.

  guint           threads_num;           // Число рабочих потоков.
  GThread       **threads;               // Рабочие потоки.

  SRpcControl    *control;               // Управляющая структура SRPC;

  volatile guint   prev_client_id;       // Идентификатор последнего, поключившегося клиента.

  volatile gint   close;                 // Завершение работы.
  volatile guint  started;               // Число запущенных потоков.

} SRpcServerPriv;

#define SRPC_SERVER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_SRPC_SERVER, SRpcServerPriv ) )


static void srpc_server_interface_init( GpRpcServerInterface *iface );
static void srpc_server_initable_iface_init( gpointer g_iface, gpointer iface_data );
static void srpc_server_class_init( SRpcServerClass *klass );
static void srpc_server_set_property( SRpcServer *srpc, guint prop_id, const GValue *value, GParamSpec *pspec );
static gboolean srpc_server_initable_init( GInitable *initable, GCancellable *cancellable, GError **error );
static void srpc_server_finalize( SRpcServer *srpc );
static gpointer shm_transporter( gpointer data );


G_DEFINE_TYPE_EXTENDED( SRpcServer, srpc_server, G_TYPE_INITIALLY_UNOWNED, 0,
    G_IMPLEMENT_INTERFACE( G_TYPE_INITABLE, srpc_server_initable_iface_init )
    G_IMPLEMENT_INTERFACE( G_TYPE_RPC_SERVER, srpc_server_interface_init ) );


static void srpc_server_init( SRpcServer *srpc )
{
  SRpcServerPriv *priv = SRPC_SERVER_GET_PRIVATE( srpc );
  priv->control = NULL;
}


static void srpc_server_class_init( SRpcServerClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = srpc_server_set_property;
  this_class->finalize = srpc_server_finalize;

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

  g_type_class_add_private( klass, sizeof( SRpcServerPriv ) );

}


static void srpc_server_set_property( SRpcServer *srpc, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  SRpcServerPriv *priv = SRPC_SERVER_GET_PRIVATE( srpc );

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
      G_OBJECT_WARN_INVALID_PROPERTY_ID( srpc, prop_id, pspec );
      break;

    }

}

static gboolean srpc_server_initable_init( GInitable *initable, GCancellable *cancellable, GError **error )
{

  g_return_val_if_fail( cancellable == NULL, FALSE );
  g_return_val_if_fail( IS_SRPC_SERVER( initable ), FALSE );

  SRpcServer *srpc = SRPC_SERVER( initable );

  SRpcServerPriv *priv = SRPC_SERVER_GET_PRIVATE( srpc );

  GError *tmp_error = NULL;
  guint i;

  // Защита от повторного вызова.
  if( priv->control != NULL ) return TRUE;

  // Обнуляем внутренние переменные.
  priv->threads = NULL;
  priv->control = NULL;
  priv->close = FALSE;

  // Проверяем типы объектов.
  if( priv->gp_rpc_auth != NULL && !g_type_is_a( G_OBJECT_TYPE( priv->gp_rpc_auth), G_TYPE_RPC_AUTH ) )
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Internal RPC auth parameter error."));
    goto srpc_server_constructor_fail;
  }

  if( priv->gp_rpc_manager == NULL || !g_type_is_a( G_OBJECT_TYPE( priv->gp_rpc_manager), G_TYPE_RPC_MANAGER ) )
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Internal RPC manager parameter error."));
    goto srpc_server_constructor_fail;
  }

  // Проверяем uri.
  if(!g_pattern_match_simple("shm", gp_rpc_get_transport_type(priv->uri)))
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Failed to find protocol name in URI."));
    goto srpc_server_constructor_fail;
  }

  // Создаем области общей памяти и семафоры.
  priv->control = srpc_create_control( priv->uri, priv->threads_num, priv->data_size + GP_RPC_HEADER_SIZE );
  if( priv->control == NULL )
  {
    g_set_error(&tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Failed to create RPC control object."));
    goto srpc_server_constructor_fail;
  }

  // Запускаем потоки обработки входящих данных.
  priv->started = 0;
  priv->threads = g_malloc( priv->threads_num * sizeof( GThread* ) );
  for( i = 0; i < priv->threads_num; i++ )
    priv->threads[i] = g_thread_new( "shm server", shm_transporter, priv );

  // Ожидаем запуска всех потоков.
  while( g_atomic_int_get( &priv->started ) != priv->threads_num ) g_usleep( 100 );

  return TRUE;

  srpc_server_constructor_fail:
    g_propagate_prefixed_error(error, tmp_error, _("Failed to create RPC server: "));
    return FALSE;

}


static void srpc_server_finalize( SRpcServer *srpc )
{

  SRpcServerPriv *priv = SRPC_SERVER_GET_PRIVATE( srpc );

  gint i;

  g_atomic_int_set( &priv->close, TRUE );

  if( priv->threads )
    for( i = 0; i < priv->threads_num; i++ )
      g_thread_join( priv->threads[i] );

  g_free( priv->threads );

  g_free( priv->uri );

  srpc_close_control( priv->control );

  G_OBJECT_CLASS( srpc_server_parent_class )->finalize( srpc );

}


// Поток приема RPC запросов от клиента.
static gpointer shm_transporter( gpointer data )
{

  SRpcServerPriv *priv = data;

  guint32 thread_id = g_atomic_int_add( &priv->started, 1 );
  GpRpcData *gp_rpc_data = priv->control->transports[ thread_id ]->gp_rpc_data;

  SEM_TYPE start = priv->control->transports[ thread_id ]->start;
  SEM_TYPE stop = priv->control->transports[ thread_id ]->stop;
  SRpcSemStatus sem_status;

  gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_INPUT, 0);
  gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);

  GpRpcHeader *iheader = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_INPUT);
  GpRpcHeader *oheader = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_OUTPUT);

  oheader->magic = GUINT32_TO_BE(GP_RPC_MAGIC);
  oheader->version = GUINT32_TO_BE(GP_RPC_VERSION);

  while( g_atomic_int_get( &priv->close ) != TRUE )
    {

    // Ожидаем запрос от клиента на выполнение.
    sem_status = srpc_sem_wait( start, 0.1 );
    if( sem_status == SRPC_SEM_FAIL ) { g_atomic_int_set( &priv->close, TRUE ); g_warning( "srpc_server: srpc_sem_wait( start ) failed" ); break; }
    if( sem_status == SRPC_SEM_TIMEOUT ) continue;
      gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_INPUT, GUINT32_FROM_BE(iheader->size) - GP_RPC_HEADER_SIZE);

    // Выполнение запроса.
      gp_rpc_server_user_exec (gp_rpc_data, priv->gp_rpc_auth, priv->gp_rpc_acl, priv->gp_rpc_manager);

    if(iheader->client_id != G_MAXUINT32)
      oheader->client_id = iheader->client_id;
    else
      oheader->client_id = GUINT32_TO_BE(g_atomic_int_add(&priv->prev_client_id, 1));

    // Сигнализируем о завершении выполнения запроса.
    srpc_sem_post( stop );

    }

  gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_INPUT, 0);
  gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);

  return NULL;

}


gchar *srpc_server_get_self_uri( GpRpcServer *srpc )
{

  SRpcServerPriv *priv = SRPC_SERVER_GET_PRIVATE( srpc );

  return g_strdup( priv->uri );

}

static void srpc_server_initable_iface_init( gpointer g_iface, gpointer iface_data )
{

  GInitableIface *iface = (GInitableIface *)g_iface;
  iface->init = srpc_server_initable_init;

}

static void srpc_server_interface_init( GpRpcServerInterface *iface )
{

  iface->get_self_uri = srpc_server_get_self_uri;

}
