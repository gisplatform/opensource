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
 * \file gp-rpc-common.c
 *
 * \author Andrei Fadeev
 * \date 26.02.2014
 * \brief Исходный файл общих функций GPRPC.
 *
*/

#include "gp-rpc-common.h"
#include <glib/gi18n-lib.h>


gchar *gp_rpc_get_transport_type (const gchar *uri)
{

  if( g_pattern_match_simple( "udp://*", uri ) ) return "udp";
  if( g_pattern_match_simple( "tcp://*", uri ) ) return "tcp";
  if( g_pattern_match_simple( "shm://*", uri ) ) return "shm";

  return NULL;

}


gchar *gp_rpc_get_address (const gchar *uri)
{

  gchar **address_parts = NULL;
  gchar  *address = NULL;
  gchar  *transport = gp_rpc_get_transport_type (uri);

  uri += 6;
  if( transport == NULL ) return NULL;
  if( g_pattern_match_simple( "shm", transport ) ) return g_strdup( uri );

  // Разбираем строку адреса.
  if( uri[0] == '[' ) // IPV6
  {
    address_parts = g_strsplit( uri, "]:", 0 );
    address = g_strdup( address_parts[0] + 1 );
    g_strfreev( address_parts );
  }
  else // IPV4
  {
    address_parts = g_strsplit( uri, ":", 0 );
    address = g_strdup( address_parts[0] );
    g_strfreev( address_parts );
  }

  return address;

}


guint16 gp_rpc_get_port (const gchar *uri)
{

  gchar **address_parts = NULL;
  gchar  *port_str = NULL;
  guint64 port;
  gchar  *transport = gp_rpc_get_transport_type (uri);

  uri += 6;
  if( transport == NULL || g_pattern_match_simple( "shm", transport ) ) return 0;

  // Разбираем строку адреса.
  if( uri[0] == '[' ) // IPV6
  {
    address_parts = g_strsplit( uri, "]:", 0 );
    if(address_parts[0]) port_str = g_strdup( address_parts[1] );
    g_strfreev( address_parts );
  }
  else // IPV4
  {
    address_parts = g_strsplit( uri, ":", 0 );
    if(address_parts[0]) port_str = g_strdup( address_parts[1] );
    g_strfreev( address_parts );
  }

  if( port_str == NULL ) return 0;

  // Преобразуем порт в число и проверяем его диапазон.
  port = g_ascii_strtoull( port_str, NULL, 10 );
  g_free( port_str );
  if( port == 0 || ( port > G_MAXUINT16 )) return 0;

  return port;

}


GpRpcExecStatus gp_rpc_server_user_exec (GpRpcData *gp_rpc_data, GpRpcAuth *gp_rpc_auth, GpRpcServerAclCallback gp_rpc_acl,
                                         GpRpcManager *gp_rpc_manager)
{

  GpRpcHeader *iheader = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_INPUT);
  GpRpcHeader *oheader = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_OUTPUT);

  GpRpcAuthStatus auth_status = GP_RPC_AUTH_FAILED;
  GpRpcExecStatus exec_status = GP_RPC_EXEC_OK;

  guint32  proc_id, obj_id;
  GpRpcServerCallback proc;
  gpointer obj = NULL;

  guint32 response_size;

  // Неверный размер заголовка.
  if(gp_rpc_data_get_header_size (gp_rpc_data) != GP_RPC_HEADER_SIZE ) { g_warning( "gp_rpc_server_exec: wrong header size" ); return GP_RPC_EXEC_FAIL; }

  // Неверный идентификатор пакета.
  if( GUINT32_FROM_BE( iheader->magic ) != GP_RPC_MAGIC ) { g_warning( "gp_rpc_server_exec: unknown gp_rpc packet" ); return GP_RPC_EXEC_FAIL; }

  // Неверная версия GPRPC.
  if( ( GUINT32_FROM_BE( iheader->version ) >> 16 ) != (GP_RPC_VERSION >> 16 ) )
  {
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_VERSION_MISMATCH);
    g_warning( "GPRPC version mismatch, server version %u.%u != client version  %u.%u",
               GP_RPC_VERSION >> 16, GP_RPC_VERSION & 0xFFFF,
               GUINT32_FROM_BE( iheader->version ) >> 16, GUINT32_FROM_BE( iheader->version ) & 0xFFFF );
    goto gp_rpc_server_exec_send;
  }

  // Неверный размер данных.
  if( GUINT32_FROM_BE( iheader->size ) != gp_rpc_data_get_data_size (gp_rpc_data, GP_RPC_DATA_INPUT) +
                                          GP_RPC_HEADER_SIZE )
  { g_warning( "gp_rpc_server_exec: wrong data size" ); return GP_RPC_EXEC_FAIL; }

  // Проверка границ буфера данных.
  if( !gp_rpc_data_validate (gp_rpc_data, GP_RPC_DATA_INPUT) )
  { g_warning( "gp_rpc_server_exec: data buffer error" ); return GP_RPC_EXEC_FAIL; }

  proc_id = gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_PARAM_PROC);

  // Идентификатор функции не может быть равным 0.
  if( proc_id == 0 ) { g_warning( "gp_rpc_server_exec: unknown proc id" ); return GP_RPC_EXEC_FAIL; }

  // Обнуляем исходящий буффер данных.
  gp_rpc_data_set_data_size (gp_rpc_data, GP_RPC_DATA_OUTPUT, 0);

  // Для всех функций кроме GP_RPC_PROC_GET_CAP требуется аутентификация (если включена).
  if( proc_id != GP_RPC_PROC_GET_CAP && gp_rpc_auth)
    auth_status = gp_rpc_auth_check (gp_rpc_auth, gp_rpc_data);
  if( gp_rpc_auth == NULL ) auth_status = GP_RPC_AUTH_AUTHENTICATED;

  // Системные функции.
  switch( proc_id )
  {

    // Возможности сервера.
    case GP_RPC_PROC_GET_CAP:
      if( gp_rpc_auth != NULL ) gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_NEED_AUTH, TRUE);
      gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_OK);
      goto gp_rpc_server_exec_send;
      break;

      // Аутентификация.
    case GP_RPC_PROC_AUTHENTICATE:
      if( gp_rpc_auth != NULL )
      {
        switch( auth_status )
        {
          case GP_RPC_AUTH_NOT_AUTHENTICATED:
          case GP_RPC_AUTH_IN_PROGRESS:
          case GP_RPC_AUTH_AUTHENTICATED:
            if(gp_rpc_auth_authenticate (gp_rpc_auth, gp_rpc_data) )
              gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_OK);
            else
              gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_FAIL);
            break;
          case GP_RPC_AUTH_NOT_SUPPORTED:
            gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_AUTH_NOT_SUPPORTED);
            break;
          default:
            gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_FAIL);
            break;
        }
      }
      else
        gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_AUTH_NOT_SUPPORTED);
      goto gp_rpc_server_exec_send;
      break;

      // Отключение от сервера.
    case GP_RPC_PROC_LOGOUT:
      if( auth_status == GP_RPC_AUTH_AUTHENTICATED)
      {
        if( gp_rpc_auth != NULL )
        {
          gp_rpc_auth_authenticate (gp_rpc_auth, gp_rpc_data);
          gp_rpc_auth_clean_session (gp_rpc_auth, gp_rpc_data);
        }
        exec_status = GP_RPC_EXEC_CLOSE;
      }
      gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_OK);
      goto gp_rpc_server_exec_send;
      break;

  }

  // Получаем указатель на вызываемую функцию.
  proc = gp_rpc_manager_get_proc (gp_rpc_manager, proc_id);
  if( !proc )
  {
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_NO_PROC);
    g_warning( "gp_rpc_server_exec: no such procedure" );
    goto gp_rpc_server_exec_send;
  }

  // Получаем указатель на вызываемый объект.
  obj_id  = gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_PARAM_OBJ);
  if( obj_id )
  {
    obj = gp_rpc_manager_get_obj (gp_rpc_manager, obj_id);
    if( obj == NULL )
    {
      gp_rpc_manager_release_proc (gp_rpc_manager, proc_id);
      gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_NO_OBJ);
      g_warning( "gp_rpc_server_exec: no such object" );
      goto gp_rpc_server_exec_send;
    }
  }

  // Проверка аутентификации.
  if( auth_status != GP_RPC_AUTH_AUTHENTICATED)
  { g_warning( "gp_rpc_server_exec: authentication failed" ); return GP_RPC_EXEC_FAIL; }

  if( gp_rpc_auth != NULL && gp_rpc_acl != NULL )
  if( !gp_rpc_acl (gp_rpc_manager, proc_id, obj_id, gp_rpc_auth_get_user_data (gp_rpc_auth, gp_rpc_data) ) )
  {
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_ACCESS_DENIED);
    g_warning( "gp_rpc_server_exec: access denied" );
    goto gp_rpc_server_exec_send;
  }

  // Выполняем запрошенную функцию.
  if( proc(gp_rpc_manager, gp_rpc_data, obj ) )
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_OK);
  else
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS, GP_RPC_STATUS_FAIL);

  gp_rpc_manager_release_proc (gp_rpc_manager, proc_id);
  if( obj != NULL ) gp_rpc_manager_release_obj (gp_rpc_manager, obj_id);

  // Аутентификация ответа.
  if( gp_rpc_auth != NULL  && !gp_rpc_auth_authenticate (gp_rpc_auth, gp_rpc_data) )
  { g_warning( "gp_rpc_server_exec: can't authenticate response" ); return GP_RPC_EXEC_FAIL; }

  gp_rpc_server_exec_send:
  oheader->sequence = iheader->sequence;
  response_size = gp_rpc_data_get_data_size (gp_rpc_data, GP_RPC_DATA_OUTPUT) + GP_RPC_HEADER_SIZE;
  oheader->size = GUINT32_TO_BE( response_size );

  return exec_status;

}


gboolean gp_rpc_client_check_header (GpRpcData *gp_rpc_data, guint32 sequence, GError **error)
{
  GpRpcHeader *iheader = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_INPUT);
  guint32 status;

  // Неверный размер заголовка.
  if(gp_rpc_data_get_header_size(gp_rpc_data) != GP_RPC_HEADER_SIZE)
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Wrong RPC header size"));
    return FALSE;
  }

  // Неверный идентификатор пакета.
  if( GUINT32_FROM_BE( iheader->magic ) != GP_RPC_MAGIC )
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Unknown RPC packet"));
    return FALSE;
  }

  // Неверный размер данных.
  if( GUINT32_FROM_BE( iheader->size ) != gp_rpc_data_get_data_size (gp_rpc_data, GP_RPC_DATA_INPUT) + GP_RPC_HEADER_SIZE )
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("Wrong RPC data size"));
    return FALSE;
  }

  // Проверка счетчика запросов.
  if( GUINT32_FROM_BE( iheader->sequence ) != sequence )
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, "%s (%s %u != %s %u)",
      _("RPC sequence mismatch"), _("received"), GUINT32_FROM_BE(iheader->sequence), _("needed"), sequence);
    return FALSE;
  }

  // Проверка границ буфера данных.
  if( !gp_rpc_data_validate (gp_rpc_data, GP_RPC_DATA_INPUT) )
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("RPC data buffer error"));
    return FALSE;
  }

  // Проверка статуса выполнения.
  status = gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_PARAM_STATUS);
  if( status == GP_RPC_STATUS_VERSION_MISMATCH || ( GUINT32_FROM_BE( iheader->version ) >> 16 ) != (GP_RPC_VERSION >> 16 ) )
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, "%s, %s %u.%u != %s %u.%u",
      _("PRPC version mismatch"),
      _("client version"), GP_RPC_VERSION >> 16, GP_RPC_VERSION & 0xFFFF,
      _("server version"), GUINT32_FROM_BE( iheader->version ) >> 16, GUINT32_FROM_BE( iheader->version ) & 0xFFFF );
    return FALSE;
  }

  if( status == GP_RPC_STATUS_OK ) return TRUE;

  switch( status )
  {

    case GP_RPC_STATUS_AUTH_NOT_SUPPORTED:
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("RPC authentication method not supported"));
      break;

    case GP_RPC_STATUS_NOT_AUTHENTICATED:
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("RPC client was not authenticated"));
      break;

    case GP_RPC_STATUS_ACCESS_DENIED:
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_FAILED, _("RPC Access denied"));
      break;

    case GP_RPC_STATUS_NO_PROC:
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_NOT_FOUND, _("Unknown RPC procedure"));
      break;

    case GP_RPC_STATUS_NO_OBJ:
      g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_NOT_FOUND, _("Unknown RPC object"));
      break;

  }

  return FALSE;

}

/**
 * gp_rpc_error_quark:
 *
 * Returns: #GQuark для использование в исключениях libgprpc.
 **/
GQuark gp_rpc_error_quark(void)
{
  return g_quark_from_static_string("gprpc-error");
}

