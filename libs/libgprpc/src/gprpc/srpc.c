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
 * \file srpc.c
 *
 * \author Andrei Fadeev
 * \date 20.03.2009
 * \brief Исходный файл класса удалённых вызовов процедур через разделяемую память.
 *
*/

#include "srpc.h"
#include "srpc-common.h"
#include "gp-rpc-common.h"

#include <gio/gio.h>
#include <glib/gi18n-lib.h>
#include <string.h>


enum { PROP_O, PROP_URI, PROP_AUTH, PROP_TIMEOUT, PROP_RESTART, PROP_DATA_SIZE };


typedef struct SRpcPriv {

  gchar          *uri;                       // Адрес сервера для подключения.

  GMutex          lock;                      // Блокировка.

  SRpcControl    *control;                   // Управляющая структура SRPC;
  GpRpcAuth      *gp_rpc_auth;               // Аутентификация.

  SRpcTransport  *transport;                 // Транспортный сегмент сервера.

  guint32         session;                   // Идентификатор сессии.
  guint32         big_endian_client_id;      // Идентификатор клиента в формате big endian.
  guint32         sequence;                  // Идентификатор запроса.

  volatile gint   failed;                    // Признак ошибки.

} SRpcPriv;

#define SRPC_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_SRPC, SRpcPriv ) )


static void srpc_interface_init( GpRpcInterface *iface );
static void srpc_initable_iface_init( gpointer g_iface, gpointer iface_data );
static void srpc_class_init( SRpcClass *klass );
static void srpc_set_property( SRpc *srpc, guint prop_id, const GValue *value, GParamSpec *pspec );
static gboolean srpc_initable_init( GInitable *initable, GCancellable *cancellable, GError **error );
static void srpc_finalize( SRpc *srpc );
static gboolean srpc_exchange(SRpcPriv *priv, GError **error);

GpRpcData *srpc_lock( GpRpc *srpc );
gboolean srpc_exec(GpRpc *srpc, guint32 proc_id, guint32 obj_id, GError **error);
void srpc_unlock( GpRpc *srpc );


G_DEFINE_TYPE_EXTENDED( SRpc, srpc, G_TYPE_INITIALLY_UNOWNED, 0,
    G_IMPLEMENT_INTERFACE( G_TYPE_INITABLE, srpc_initable_iface_init )
    G_IMPLEMENT_INTERFACE( G_TYPE_RPC, srpc_interface_init ) );

static void srpc_init( SRpc *srpc )
{
  SRpcPriv *priv = SRPC_GET_PRIVATE( srpc );
  priv->control = NULL;
}


static void srpc_class_init( SRpcClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = srpc_set_property;
  this_class->finalize = srpc_finalize;

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

  g_type_class_add_private( klass, sizeof( SRpcPriv ) );

}


static void srpc_set_property( SRpc *srpc, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  SRpcPriv *priv = SRPC_GET_PRIVATE( srpc );

  switch ( prop_id )
    {

    case PROP_URI:
      priv->uri = g_value_dup_string( value );
      break;

    case PROP_AUTH:
      priv->gp_rpc_auth = g_value_get_pointer( value );
      break;

    case PROP_TIMEOUT:
      break;

    case PROP_RESTART:
      break;

    case PROP_DATA_SIZE:
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( srpc, prop_id, pspec );
      break;

    }

}

static gboolean srpc_initable_init( GInitable *initable, GCancellable *cancellable, GError **error )
{
  g_return_val_if_fail( cancellable == NULL, FALSE );
  g_return_val_if_fail( IS_SRPC( initable ), FALSE );

  GError *tmp_error = NULL;
  SRpc *srpc = SRPC(initable);
  SRpcPriv *priv = SRPC_GET_PRIVATE(srpc);

  // Защита от повторного вызова.
  if( priv->control != NULL ) return TRUE;

  // Обнуляем внутренние переменные.
  priv->control = NULL;
  priv->transport = NULL;
  priv->big_endian_client_id = G_MAXUINT32;
  priv->session = 0;
  priv->sequence = 1;
  priv->failed = FALSE;
  g_mutex_init( &priv->lock );

  // Проверяем типы объектов.
  if( priv->gp_rpc_auth != NULL && !g_type_is_a( G_OBJECT_TYPE( priv->gp_rpc_auth), G_TYPE_RPC_AUTH ) )
    { g_warning( "auth parameter is not a G_TYPE_RPC_AUTH" ); goto srpc_constructor_fail; }

  // Проверяем uri.
  if( !g_pattern_match_simple( "shm", gp_rpc_get_transport_type (priv->uri) ) ) goto srpc_constructor_fail;

  // Подключаемся к области общей памяти и семафорам.
  priv->control = srpc_open_control( priv->uri );
  if( priv->control == NULL ) { g_warning( "srpc: srpc_open_control failed" ); goto srpc_constructor_fail; }

  if( srpc_lock( GP_RPC( srpc ) ) == NULL ) { g_warning( "srpc: srpc_lock failed" ); goto srpc_constructor_fail; }

  // Выясняем возможности сервера.
  gp_rpc_data_set_uint32 (priv->transport->gp_rpc_data, GP_RPC_PARAM_PROC, GP_RPC_PROC_GET_CAP);
  srpc_exchange(priv, &tmp_error);
  if(G_UNLIKELY(tmp_error))
    goto srpc_constructor_fail_with_unlock;

  // Аутентификация.
  if(gp_rpc_data_get_uint32 (priv->transport->gp_rpc_data, GP_RPC_PARAM_NEED_AUTH) == TRUE )
    {

    GpRpcHeader *iheader = gp_rpc_data_get_header (priv->transport->gp_rpc_data, GP_RPC_DATA_INPUT);
    GpRpcHeader *oheader = gp_rpc_data_get_header (priv->transport->gp_rpc_data, GP_RPC_DATA_OUTPUT);
    GpRpcAuthStatus auth_status;

    if( priv->gp_rpc_auth == NULL )
      { g_warning( "srpc: authentication required" ); goto srpc_constructor_fail_with_unlock; }

    while( TRUE )
    {
      auth_status = gp_rpc_auth_check (priv->gp_rpc_auth, priv->transport->gp_rpc_data);

      if(auth_status == GP_RPC_AUTH_IN_PROGRESS || auth_status == GP_RPC_AUTH_NOT_AUTHENTICATED)
      {
        if(auth_status == GP_RPC_AUTH_IN_PROGRESS)
        {
          priv->session = GUINT32_FROM_BE( iheader->session );
          oheader->session = iheader->session;
        }

        gp_rpc_data_set_data_size (priv->transport->gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);
        oheader->sequence = GUINT32_TO_BE( priv->sequence );
        gp_rpc_data_set_uint32 (priv->transport->gp_rpc_data, GP_RPC_PARAM_PROC, GP_RPC_PROC_AUTHENTICATE);
        gp_rpc_auth_authenticate (priv->gp_rpc_auth, priv->transport->gp_rpc_data);

        srpc_exchange(priv, &tmp_error);
        if(G_UNLIKELY(tmp_error))
          goto srpc_constructor_fail_with_unlock;
      }
      else
        break;
    }

    // Ошибка аутентификации
    if( auth_status != GP_RPC_AUTH_AUTHENTICATED) { g_warning( "srpc: authentication failed" ); goto srpc_constructor_fail_with_unlock; }

    }

  srpc_unlock( GP_RPC( srpc ) );

  return TRUE;

srpc_constructor_fail_with_unlock:
  srpc_unlock(GP_RPC(srpc));

srpc_constructor_fail:
  {
    const gchar *error_message = _("Failed to create RPC client");

    if(tmp_error)
      g_propagate_prefixed_error(error, tmp_error, "%s: ", error_message);
    else
      g_set_error_literal(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, error_message);
  }

  priv->failed = TRUE;
  return FALSE;
}


static void srpc_finalize( SRpc *srpc )
{

  SRpcPriv *priv = SRPC_GET_PRIVATE( srpc );

  if( !priv->failed )
    {
    if( srpc_lock( GP_RPC( srpc ) ) != NULL )
      {
      srpc_exec( GP_RPC( srpc ), GP_RPC_PROC_LOGOUT, 0, NULL );
      srpc_unlock( GP_RPC( srpc ) );
      }
    }

  if( priv->control != NULL )
    srpc_close_control( priv->control );

  g_free( priv->uri );

  g_mutex_clear( &priv->lock );

  G_OBJECT_CLASS( srpc_parent_class )->finalize( srpc );

}


static gboolean srpc_exchange(SRpcPriv *priv, GError **error)
{

  guint32 request_size;
  SRpcSemStatus sem_status;
  GError *tmp_error = NULL;

  GpRpcHeader *iheader = gp_rpc_data_get_header (priv->transport->gp_rpc_data, GP_RPC_DATA_INPUT);
  GpRpcHeader *oheader = gp_rpc_data_get_header (priv->transport->gp_rpc_data, GP_RPC_DATA_OUTPUT);

  // Отправляем запрос.
  request_size = gp_rpc_data_get_data_size (priv->transport->gp_rpc_data, GP_RPC_DATA_OUTPUT) + GP_RPC_HEADER_SIZE;
  oheader->client_id = priv->big_endian_client_id;
  oheader->size = GUINT32_TO_BE( request_size );
  srpc_sem_post( priv->transport->start );

  while( TRUE )
    {

    sem_status = srpc_sem_wait( priv->transport->stop, 1.0 );

    if(sem_status == SRPC_SEM_FAIL)
    {
      g_atomic_int_set( &priv->failed, TRUE );
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("RPC stop semaphore error."));
      return FALSE;
    }

    if( sem_status == SRPC_SEM_TIMEOUT ) continue;

    break;

    }

  gp_rpc_data_set_data_size (priv->transport->gp_rpc_data, GP_RPC_DATA_INPUT, GUINT32_FROM_BE(iheader->size) -
                                                                            GP_RPC_HEADER_SIZE);

  // Проверка заголовка.
  gp_rpc_client_check_header (priv->transport->gp_rpc_data, priv->sequence, &tmp_error);
  if(G_UNLIKELY(tmp_error))
  {
    g_propagate_prefixed_error(error, tmp_error, _("Check of RPC header failed: " ));

    if(g_error_matches(tmp_error, GP_RPC_ERROR, GP_RPC_ERROR_NOT_FOUND))
    {
      // Счетчик запросов и ID клиента.
      priv->sequence++;
      priv->big_endian_client_id = iheader->client_id;
    }

    return FALSE;
  }

  // Счетчик запросов и ID клиента.
  priv->sequence++;
  priv->big_endian_client_id = iheader->client_id;

  return TRUE;

}


GpRpcData *srpc_lock( GpRpc *srpc )
{

  SRpcPriv *priv = SRPC_GET_PRIVATE( srpc );

  guint gp_rpc_data_id;
  SRpcSemStatus sem_status;

  GpRpcHeader *oheader;

  if( priv->failed ) { g_warning( "srpc: client failed" ); return NULL; }

  g_mutex_lock( &priv->lock );

  while( TRUE )
    {

    sem_status = srpc_sem_wait( priv->control->access, 1.0 );
    if( sem_status == SRPC_SEM_FAIL )
      {
      g_atomic_int_set( &priv->failed, TRUE );
      g_warning( "srpc: srpc_sem_wait( access ) failed" );
      return NULL;
      }
    if( sem_status == SRPC_SEM_TIMEOUT ) continue;

    break;

    }

  for( gp_rpc_data_id = 0; gp_rpc_data_id < priv->control->threads_num; gp_rpc_data_id++ )
    if( srpc_sem_try( priv->control->transports[gp_rpc_data_id]->used ) )
      break;
  if( gp_rpc_data_id == priv->control->threads_num )
    {
    g_warning( "srpc: can't find transport" );
    return NULL;
    }

  priv->transport = priv->control->transports[gp_rpc_data_id];

  oheader = gp_rpc_data_get_header (priv->transport->gp_rpc_data, GP_RPC_DATA_OUTPUT);
  oheader->magic = GUINT32_TO_BE(GP_RPC_MAGIC);
  oheader->version = GUINT32_TO_BE(GP_RPC_VERSION);
  oheader->session = GUINT32_TO_BE( priv->session );
  oheader->sequence = GUINT32_TO_BE( priv->sequence );

  return priv->transport->gp_rpc_data;

}


gboolean srpc_exec(GpRpc *srpc, guint32 proc_id, guint32 obj_id, GError **error)
{
  GError *tmp_error = NULL;
  SRpcPriv *priv = SRPC_GET_PRIVATE( srpc );

  if(priv->failed || priv->transport == NULL)
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("RPC client is not connected."));
    return FALSE;
  }

  gp_rpc_data_set_uint32 (priv->transport->gp_rpc_data, GP_RPC_PARAM_PROC, proc_id);
  gp_rpc_data_set_uint32 (priv->transport->gp_rpc_data, GP_RPC_PARAM_OBJ, obj_id);

  if( priv->gp_rpc_auth != NULL )
    if( !gp_rpc_auth_authenticate (priv->gp_rpc_auth, priv->transport->gp_rpc_data) )
    {
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Can't authenticate."));
      goto srpc_exec_fail;
    }

  srpc_exchange(priv, &tmp_error);
  if(tmp_error)
  {
    g_propagate_prefixed_error(error, tmp_error, _("Can't execute RPC request: "));

    if(g_error_matches(*error, GP_RPC_ERROR, GP_RPC_ERROR_NOT_FOUND))
      goto srpc_exec_soft_fail;
    else
      goto srpc_exec_fail;
  }

  if( priv->gp_rpc_auth != NULL )
    if(gp_rpc_auth_check (priv->gp_rpc_auth, priv->transport->gp_rpc_data) != GP_RPC_AUTH_AUTHENTICATED)
    {
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_AUTH_FAILED, _("Authentication failed."));
      goto srpc_exec_fail;
    }

  return TRUE;

  srpc_exec_fail:
    priv->failed = TRUE;

  srpc_exec_soft_fail:

  return FALSE;

}


void srpc_unlock( GpRpc *srpc )
{

  SRpcPriv *priv = SRPC_GET_PRIVATE( srpc );

  if( priv->failed ) { g_warning( "srpc: client failed" ); return; }

  gp_rpc_data_set_data_size (priv->transport->gp_rpc_data, GP_RPC_DATA_INPUT, 0);
  gp_rpc_data_set_data_size (priv->transport->gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);

  srpc_sem_post( priv->transport->used );
  srpc_sem_post( priv->control->access );

  priv->transport = NULL;

  g_mutex_unlock( &priv->lock );

}


gboolean srpc_connected( GpRpc *srpc )
{

  SRpcPriv *priv = SRPC_GET_PRIVATE( srpc );

  return priv->failed ? FALSE : TRUE;

}


gchar *srpc_get_self_uri( GpRpc *srpc )
{

  SRpcPriv *priv = SRPC_GET_PRIVATE( srpc );

  if( priv->failed ) { g_warning( "srpc: client failed" ); return NULL; }

  return g_strdup( priv->uri );

}


gchar *srpc_get_server_uri( GpRpc *srpc )
{

  SRpcPriv *priv = SRPC_GET_PRIVATE( srpc );

  if( priv->failed ) { g_warning( "srpc: client failed" ); return NULL; }

  return g_strdup( priv->uri );

}

static void srpc_initable_iface_init( gpointer g_iface, gpointer iface_data )
{

  GInitableIface *iface = (GInitableIface *)g_iface;
  iface->init = srpc_initable_init;

}

static void srpc_interface_init( GpRpcInterface *iface )
{

  iface->lock = srpc_lock;
  iface->exec = srpc_exec;
  iface->unlock = srpc_unlock;
  iface->connected = srpc_connected;
  iface->get_self_uri = srpc_get_self_uri;
  iface->get_server_uri = srpc_get_server_uri;

}
