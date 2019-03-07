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
 * \file gp-rpc-auth-cram.c
 *
 * \author Andrei Fadeev
 * \date 20.02.2014
 * \brief Исходный файл класса взаимной аутентификации на сервере по механизму запрос-ответ.
 *
*/

#include "gp-rpc-auth-cram-common.h"
#include "gp-rpc-auth-cram.h"

#include "gp-rpc-common.h"
#include <string.h>


enum { PROP_O, PROP_KEY };

typedef struct GpRpcAuthCramPriv {

  guchar      *key;                        // Ключ аутентификации.
  gsize        key_size;                   // Размер ключа аутентификации.

  guint32      hash_type;                  // Тип хэш функции.
  GHmac       *hmac;                       // Хэш функция.

  guint32      chalenge_size;              // Размер строки запроса.
  guchar      *server_chalenge;            // Строка запроса сервера.
  guchar      *client_chalenge;            // Строка запроса клиента.

  guint32      status;                     // Текущий статус аутентификации.

} GpRpcAuthCramPriv;

#define GP_RPC_AUTH_CRAM_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_RPC_AUTH_CRAM, GpRpcAuthCramPriv ) )


static void gp_rpc_auth_cram_interface_init (GpRpcAuthInterface *iface);
static void gp_rpc_auth_cram_class_init (GpRpcAuthCramClass *klass);
static void gp_rpc_auth_cram_set_property (GpRpcAuthCram *gp_rpc_auth_cram, guint prop_id, const GValue *value,
                                           GParamSpec *pspec);
static GObject*gp_rpc_auth_cram_constructor (GType g_type, guint n_construct_properties,
                                             GObjectConstructParam *construct_properties);
static void gp_rpc_auth_cram_finalize (GpRpcAuthCram *gp_rpc_auth_cram);


G_DEFINE_TYPE_WITH_CODE(GpRpcAuthCram, gp_rpc_auth_cram, G_TYPE_INITIALLY_UNOWNED,
    G_IMPLEMENT_INTERFACE (G_TYPE_RPC_AUTH, gp_rpc_auth_cram_interface_init));

static void gp_rpc_auth_cram_init (GpRpcAuthCram *gp_rpc_auth_cram){ ; }


static void gp_rpc_auth_cram_class_init (GpRpcAuthCramClass *klass)
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = (void*) gp_rpc_auth_cram_set_property;
  this_class->constructor = (void*) gp_rpc_auth_cram_constructor;
  this_class->finalize = (void*) gp_rpc_auth_cram_finalize;

  g_object_class_install_property( this_class, PROP_KEY,
                                   g_param_spec_pointer( "key", "Key ", "Authentication key", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof(GpRpcAuthCramPriv) );

}


static void gp_rpc_auth_cram_set_property (GpRpcAuthCram *gp_rpc_auth_cram, guint prop_id, const GValue *value,
                                           GParamSpec *pspec)
{

  GpRpcAuthCramPriv *priv = GP_RPC_AUTH_CRAM_GET_PRIVATE(gp_rpc_auth_cram);

  switch ( prop_id )
  {

    case PROP_KEY:
      priv->key = g_base64_decode( g_value_get_pointer( value ), &priv->key_size );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gp_rpc_auth_cram, prop_id, pspec );
      break;

  }

}


static GObject*gp_rpc_auth_cram_constructor (GType g_type, guint n_construct_properties,
                                             GObjectConstructParam *construct_properties)
{

  GObject *gp_rpc_auth_cram = G_OBJECT_CLASS( gp_rpc_auth_cram_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( gp_rpc_auth_cram == NULL ) return NULL;

  GpRpcAuthCramPriv *priv = GP_RPC_AUTH_CRAM_GET_PRIVATE(gp_rpc_auth_cram);

  // Обнуляем внутренние переменные.
  priv->hash_type = GP_RPC_AUTH_CRAM_HASH_UNKNOWN;
  priv->hmac = NULL;

  priv->server_chalenge = NULL;
  priv->client_chalenge = NULL;
  priv->chalenge_size = 0;

  priv->status = GP_RPC_AUTH_CRAM_STATUS_ZERO;

  // Проверяем размер ключа.
  if( priv->key == NULL || priv->key_size < GP_RPC_AUTH_CRAM_MIN_KEY_SIZE || priv->key_size >
                                                                             GP_RPC_AUTH_CRAM_MAX_KEY_SIZE )
  { g_object_unref(gp_rpc_auth_cram); g_warning( "gp_rpc_auth_cram: wrong key size" ); return NULL; }

  return gp_rpc_auth_cram;

}


static void gp_rpc_auth_cram_finalize (GpRpcAuthCram *gp_rpc_auth_cram)
{

  GpRpcAuthCramPriv *priv = GP_RPC_AUTH_CRAM_GET_PRIVATE(gp_rpc_auth_cram);

  if( priv->hmac != NULL )
    g_hmac_unref( priv->hmac );

  if( priv->key != NULL )
    memset( priv->key, 0, priv->key_size );

  g_free( priv->key );
  g_free( priv->client_chalenge );
  g_free( priv->server_chalenge );

  G_OBJECT_CLASS( gp_rpc_auth_cram_parent_class )->finalize(gp_rpc_auth_cram);

}


GpRpcAuthCram *gp_rpc_auth_cram_new (const gchar *key64)
{

  return g_object_new( G_TYPE_RPC_AUTH_CRAM, "key", key64, NULL );

}


GpRpcAuthStatus gp_rpc_auth_cram_check (GpRpcAuth *gp_rpc_auth_cram, GpRpcData *gp_rpc_data)
{

  GpRpcAuthCramPriv *priv = GP_RPC_AUTH_CRAM_GET_PRIVATE(gp_rpc_auth_cram);

  GpRpcHeader *iheader = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_INPUT);

  guchar *server_chalenge; guint32 server_chalenge_size;
  guchar *server_response; guint32 server_response_size;

  // Аутентификация уже проверена, можно посылать следующий запрос.
  if( priv->status == GP_RPC_AUTH_CRAM_STATUS_CHECKED ) return GP_RPC_AUTH_AUTHENTICATED;

  // Начальное состояние, аутентифиакция не проводилась.
  if( priv->status == GP_RPC_AUTH_CRAM_STATUS_ZERO ) return GP_RPC_AUTH_NOT_AUTHENTICATED;

  // Проверка ответов сервера.
  if( priv->status == GP_RPC_AUTH_CRAM_STATUS_BEGIN || priv->status == GP_RPC_AUTH_CRAM_STATUS_AUTH )
  {

    guint32 server_status;

    // Не совпадает тип аутентификации.
    if(gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_PARAM_AUTH_TYPE) != GP_RPC_AUTH_CRAM_TYPE_ID )
    {
      priv->status = GP_RPC_AUTH_CRAM_STATUS_FAIL;
      g_warning( "gp_rpc_auth_cram: unknown authentication type" );
      return GP_RPC_AUTH_NOT_SUPPORTED;
    }

    server_status = gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_STATUS);

    // Если сервер не прислал строку запроса или идентификатор сессии - ошибка аутентификации.
    server_chalenge = gp_rpc_data_get (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_CHALENGE_S, &server_chalenge_size);
    if( server_chalenge == NULL || server_chalenge_size == 0 || iheader->session == 0 )
    {
      priv->status = GP_RPC_AUTH_CRAM_STATUS_FAIL;
      g_warning( "gp_rpc_auth_cram: incomplete authentication response" );
      return GP_RPC_AUTH_FAILED;
    }

    // Ответ сервера на получение строки запроса.
    if( server_status == GP_RPC_AUTH_CRAM_STATUS_ONLY_ONE )
    {

      guint32 server_hash;
      GChecksumType hash_type;

      // Размер строки должен быть кратен sizeof( guint32 ).
      // Если строка уже была!? - ошибка аутентификации.
      if( ( server_chalenge_size % sizeof( guint32 ) ) || priv->server_chalenge || priv->hmac )
      {
        priv->status = GP_RPC_AUTH_CRAM_STATUS_FAIL;
        g_warning( "gp_rpc_auth_cram: wrong chalenge size" );
        return GP_RPC_AUTH_FAILED;
      }

      // Тип хэш функции.
      server_hash = gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_HASH);
      if( server_hash == GP_RPC_AUTH_CRAM_HASH_MD5 ) hash_type = G_CHECKSUM_MD5;
      else if( server_hash == GP_RPC_AUTH_CRAM_HASH_SHA1 ) hash_type = G_CHECKSUM_SHA1;
      else if( server_hash == GP_RPC_AUTH_CRAM_HASH_SHA256 ) hash_type = G_CHECKSUM_SHA256;
      else { priv->status = GP_RPC_AUTH_CRAM_STATUS_FAIL; g_warning( "gp_rpc_auth_cram: unknown hash type" ); return GP_RPC_AUTH_NOT_SUPPORTED; }

      // Создаем объект для расчета хэша.
      priv->hmac = g_hmac_new( hash_type, priv->key, priv->key_size );

      // Строка запроса сервера.
      priv->server_chalenge = g_memdup( server_chalenge, server_chalenge_size );
      priv->client_chalenge = g_malloc( server_chalenge_size );
      priv->chalenge_size = server_chalenge_size;

      priv->status = GP_RPC_AUTH_CRAM_STATUS_AUTH;
      return GP_RPC_AUTH_IN_PROGRESS;

    }

    // Ответ сервера на аутентификацию.
    if( server_status == GP_RPC_AUTH_CRAM_STATUS_RESPONSE )
    {

      GHmac   *hmac;
      guchar   hmac_buffer[GP_RPC_AUTH_CRAM_MAX_HASH_SIZE ];
      gsize    hmac_size = sizeof( hmac_buffer );
      gpointer data_to_hmac = gp_rpc_data_get_data (gp_rpc_data, GP_RPC_DATA_INPUT);
      guint32  data_size_to_hmac = gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_SIGNED_SIZE);

      // Если размер новой строки запроса сервера изменился - ошибка аутентификации.
      if( server_chalenge_size != priv->chalenge_size )
      {
        priv->status = GP_RPC_AUTH_CRAM_STATUS_FAIL;
        g_warning( "gp_rpc_auth_cram: bad server chalenge" );
        return GP_RPC_AUTH_FAILED;
      }

      // Если нет строки запроса сервера или
      // нет строки запроса клиента или
      // не задана хэш функция - ошибка аутентификации.
      if( priv->server_chalenge  == NULL|| priv->client_chalenge == NULL || priv->hmac == NULL )
      {
        priv->status = GP_RPC_AUTH_CRAM_STATUS_FAIL;
        g_warning( "gp_rpc_auth_cram: incomplete authentication state" );
        return GP_RPC_AUTH_FAILED;
      }

      // Вычисляем каким должен быть ответ сервера.
      hmac = g_hmac_copy( priv->hmac );
      g_hmac_update( hmac, data_to_hmac, data_size_to_hmac );
      g_hmac_update( hmac, priv->server_chalenge, priv->chalenge_size );
      g_hmac_update( hmac, priv->client_chalenge, priv->chalenge_size );
      g_hmac_get_digest( hmac, hmac_buffer, &hmac_size );
      g_hmac_unref( hmac );

      // Сравниваем с реальным ответом.
      server_response = gp_rpc_data_get (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_RESPONSE_S, &server_response_size);
      if( hmac_size != server_response_size || memcmp( hmac_buffer, server_response, hmac_size ) )
      { priv->status = GP_RPC_AUTH_CRAM_STATUS_FAIL; g_warning( "gp_rpc_auth_cram: digest error" ); return GP_RPC_AUTH_FAILED; }

      // Запоминаем новую строку запроса сервера.
      memcpy( priv->server_chalenge, server_chalenge, server_chalenge_size );

      priv->status = GP_RPC_AUTH_CRAM_STATUS_CHECKED;
      return GP_RPC_AUTH_AUTHENTICATED;

    }

  }

  priv->status = GP_RPC_AUTH_CRAM_STATUS_FAIL;
  g_warning( "gp_rpc_auth_cram: unknown error" );
  return GP_RPC_AUTH_FAILED;

}


gboolean gp_rpc_auth_cram_authenticate (GpRpcAuth *gp_rpc_auth_cram, GpRpcData *gp_rpc_data)
{

  GpRpcAuthCramPriv *priv = GP_RPC_AUTH_CRAM_GET_PRIVATE(gp_rpc_auth_cram);

  // Начальное состояние, аутентифиакция не проводилась.
  if( priv->status == GP_RPC_AUTH_CRAM_STATUS_ZERO )
  {

    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_AUTH_TYPE, GP_RPC_AUTH_CRAM_TYPE_ID);
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_STATUS, GP_RPC_AUTH_CRAM_STATUS_BEGIN);
    priv->status = GP_RPC_AUTH_CRAM_STATUS_BEGIN;
    return TRUE;

  }

  // Сервер прислал нам строку запроса, пришлем ему свою.
  if( priv->status == GP_RPC_AUTH_CRAM_STATUS_AUTH || priv->status == GP_RPC_AUTH_CRAM_STATUS_CHECKED )
  {

    guint32  i;
    GHmac   *hmac;
    guchar   hmac_buffer[GP_RPC_AUTH_CRAM_MAX_HASH_SIZE ];
    gsize    hmac_size = sizeof( hmac_buffer );
    gpointer data_to_hmac = gp_rpc_data_get_data (gp_rpc_data, GP_RPC_DATA_OUTPUT);
    guint32  data_size_to_hmac;

    // Генерируем нашу строку запроса.
    for( i = 0; i < ( priv->chalenge_size / sizeof( guint32 ) ); i++ )
      *( ( ( guint32* )priv->client_chalenge ) + i ) = g_random_int();

    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_SIGNED_SIZE, 0);
    data_size_to_hmac = gp_rpc_data_get_data_size (gp_rpc_data, GP_RPC_DATA_OUTPUT);
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_SIGNED_SIZE, data_size_to_hmac);

    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_AUTH_TYPE, GP_RPC_AUTH_CRAM_TYPE_ID);
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_STATUS, GP_RPC_AUTH_CRAM_STATUS_AUTH);

    // Вычисляем ответ серверу.
    hmac = g_hmac_copy( priv->hmac );
    g_hmac_update( hmac, data_to_hmac, data_size_to_hmac );
    g_hmac_update( hmac, priv->client_chalenge, priv->chalenge_size );
    g_hmac_update( hmac, priv->server_chalenge, priv->chalenge_size );
    g_hmac_get_digest( hmac, hmac_buffer, &hmac_size );
    g_hmac_unref( hmac );

    gp_rpc_data_set (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_CHALENGE_C, priv->client_chalenge, priv->chalenge_size);
    gp_rpc_data_set (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_RESPONSE_C, hmac_buffer, hmac_size);

    priv->status = GP_RPC_AUTH_CRAM_STATUS_AUTH;
    return TRUE;

  }

  g_warning( "gp_rpc_auth_cram: authentication not checked" );
  return FALSE;

}


gpointer gp_rpc_auth_cram_get_user_data (GpRpcAuth *gp_rpc_auth_cram, GpRpcData *gp_rpc_data)
{

  return NULL;

}


gboolean gp_rpc_auth_cram_clean_session (GpRpcAuth *gp_rpc_auth_cram, GpRpcData *gp_rpc_data)
{

  GpRpcAuthCramPriv *priv = GP_RPC_AUTH_CRAM_GET_PRIVATE(gp_rpc_auth_cram);

  if( priv->hmac != NULL )
    g_hmac_unref( priv->hmac );

  g_free( priv->key );
  g_free( priv->client_chalenge );
  g_free( priv->server_chalenge );

  priv->hash_type = GP_RPC_AUTH_CRAM_HASH_UNKNOWN;
  priv->hmac = NULL;

  priv->server_chalenge = NULL;
  priv->client_chalenge = NULL;

  priv->chalenge_size = 0;

  priv->status = GP_RPC_AUTH_CRAM_STATUS_ZERO;

  return TRUE;

}


static void gp_rpc_auth_cram_interface_init (GpRpcAuthInterface *iface)
{

  iface->check = gp_rpc_auth_cram_check;
  iface->authenticate = gp_rpc_auth_cram_authenticate;
  iface->get_user_data = gp_rpc_auth_cram_get_user_data;
  iface->clean_session = gp_rpc_auth_cram_clean_session;

}
