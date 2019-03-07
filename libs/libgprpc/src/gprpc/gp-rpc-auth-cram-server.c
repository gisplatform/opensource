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
 * \file gp_rpc-auth-cram-server.c
 *
 * \author Andrei Fadeev
 * \date 24.02.2014
 * \brief Исходный файл класса взаимной аутентификации клиентов по механизму запрос-ответ.
 *
*/

#include "gp-rpc-auth-cram-common.h"
#include "gp-rpc-auth-cram-server.h"

#include "gp-rpc-common.h"
#include <string.h>


enum { PROP_O, PROP_HASH_TYPE, PROP_CHALENGE_SIZE, PROP_MAX_SESSIONS, PROP_TIMEOUT, PROP_AUTH_TIMEOUT };

typedef struct GpRpcAuthCramServerKey {

  GHmac       *hmac;                       // Хэш функция.
  guchar      *key;                        // Ключ аутентификации.
  guint32      key_size;                   // Размер ключа аутентификации.

  gpointer     data;                       // Пользовательские данные.

} GpRpcAuthCramServerKey;


typedef struct GpRpcAuthCramServerSession {

  GpRpcAuthCramServerKey *key;              // Используемый клиентом ключ.

  guchar      *server_chalenge;            // Строка запроса сервера.
  guchar      *prev_server_chalenge;       // Предыдущая строка запроса сервера.

  guint32      next_sequence;              // Идентификатор следующего запроса.

  guint32      status;                     // Текущий статус аутентификации.
  GTimeVal     last_used;                  // Время последнего использования.

  GMutex       lock;                       // Блокировка.

} GpRpcAuthCramServerSession;


typedef struct GpRpcAuthCramServerPriv {

  GRWLock         lock;                    // Блокировка.

  GChecksumType   hash_type;               // Тип хэш функции.
  guint32         chalenge_size;           // Размер строки запроса.

  GList          *keys;                    // Массив возможных ключей аутентификации.
  GHashTable     *sessions;                // Клиентские сессии.

  guint32         max_sessions;            // Максимально возможное число одновременных сессий.
  guint32         timeout;                 // Таймаут отключения клиента, с.
  guint32         auth_timeout;            // Таймаут отключения клиента при первичной аутентификации, с.

  GThread        *session_clean;           // Поток очистки сессий.
  volatile gint   close;                   // Завершение потока очистки.

} GpRpcAuthCramServerPriv;

#define GP_RPC_AUTH_CRAM_SERVER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_RPC_AUTH_CRAM_SERVER, GpRpcAuthCramServerPriv ) )


static void gp_rpc_auth_cram_server_interface_init (GpRpcAuthInterface *iface);
static void gp_rpc_auth_cram_server_class_init (GpRpcAuthCramServerClass *klass);
static void gp_rpc_auth_cram_server_set_property (GpRpcAuthCramServer *gp_rpc_auth_cram, guint prop_id,
                                                  const GValue *value, GParamSpec *pspec);
static GObject*gp_rpc_auth_cram_server_constructor (GType g_type, guint n_construct_properties,
                                                    GObjectConstructParam *construct_properties);
static void gp_rpc_auth_cram_server_finalize (GpRpcAuthCramServer *gp_rpc_auth_cram);

static gpointer gp_rpc_auth_cram_server_sessions_clean (gpointer data);
static void gp_rpc_auth_cram_server_key_free (gpointer data);
static void gp_rpc_auth_cram_server_session_free(GpRpcAuthCramServerSession *session_info);


G_DEFINE_TYPE_WITH_CODE(GpRpcAuthCramServer, gp_rpc_auth_cram_server, G_TYPE_INITIALLY_UNOWNED,
    G_IMPLEMENT_INTERFACE (G_TYPE_RPC_AUTH, gp_rpc_auth_cram_server_interface_init));

static void gp_rpc_auth_cram_server_init (GpRpcAuthCramServer *gp_rpc_auth_cram){ ; }


static void gp_rpc_auth_cram_server_class_init (GpRpcAuthCramServerClass *klass)
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  this_class->set_property = (void*) gp_rpc_auth_cram_server_set_property;
  this_class->constructor = (void*) gp_rpc_auth_cram_server_constructor;
  this_class->finalize = (void*) gp_rpc_auth_cram_server_finalize;

  g_object_class_install_property( this_class, PROP_HASH_TYPE,
                                   g_param_spec_int( "hash-type", "Hash type", "Authentication hash type", G_MININT, G_MAXINT, G_CHECKSUM_SHA256, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_CHALENGE_SIZE,
                                   g_param_spec_uint( "chalenge-size", "Chalenge size", "Authentication chalenge size",
                                                      GP_RPC_AUTH_CRAM_MIN_CHALENGE_SIZE, GP_RPC_AUTH_CRAM_MAX_CHALENGE_SIZE,
                                                      GP_RPC_AUTH_CRAM_MAX_CHALENGE_SIZE, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_MAX_SESSIONS,
                                   g_param_spec_uint( "max-sessions", "Max sessions", "Maximum sessions at once", 0, G_MAXUINT,
                                                      GP_RPC_AUTH_CRAM_MAX_SESSIONS, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_TIMEOUT,
                                   g_param_spec_uint( "timeout", "Timeout", "Session timeout", 0, G_MAXUINT, GP_RPC_AUTH_CRAM_TIMEOUT, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_AUTH_TIMEOUT,
                                   g_param_spec_uint( "auth-timeout", "Auth timeout", "Authentication timeout", 0, G_MAXUINT,
                                                      GP_RPC_AUTH_CRAM_AUTH_TIMEOUT, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_type_class_add_private( klass, sizeof(GpRpcAuthCramServerPriv) );

}


static void gp_rpc_auth_cram_server_set_property (GpRpcAuthCramServer *gp_rpc_auth_cram, guint prop_id,
                                                  const GValue *value, GParamSpec *pspec)
{

  GpRpcAuthCramServerPriv *priv = GP_RPC_AUTH_CRAM_SERVER_GET_PRIVATE(gp_rpc_auth_cram);

  switch ( prop_id )
  {

    case PROP_HASH_TYPE:
      priv->hash_type = g_value_get_int( value );
      break;

    case PROP_CHALENGE_SIZE:
      priv->chalenge_size = g_value_get_uint( value );
      break;

    case PROP_MAX_SESSIONS:
      priv->max_sessions = g_value_get_uint( value );
      break;

    case PROP_TIMEOUT:
      priv->timeout = g_value_get_uint( value );
      break;

    case PROP_AUTH_TIMEOUT:
      priv->auth_timeout = g_value_get_uint( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(gp_rpc_auth_cram, prop_id, pspec );
      break;

  }

}


static GObject*gp_rpc_auth_cram_server_constructor (GType g_type, guint n_construct_properties,
                                                    GObjectConstructParam *construct_properties)
{

  GObject *gp_rpc_auth_cram = G_OBJECT_CLASS( gp_rpc_auth_cram_server_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( gp_rpc_auth_cram == NULL ) return NULL;

  GpRpcAuthCramServerPriv *priv = GP_RPC_AUTH_CRAM_SERVER_GET_PRIVATE(gp_rpc_auth_cram);

  // Обнуляем внутренние переменные.
  priv->keys = NULL;
  priv->sessions = NULL;
  priv->session_clean = NULL;
  priv->close = FALSE;

  // Проверяем тип хэш функции.
  if( priv->hash_type != G_CHECKSUM_MD5 && priv->hash_type != G_CHECKSUM_SHA1 && priv->hash_type != G_CHECKSUM_SHA256 )
  { g_object_unref(gp_rpc_auth_cram); g_warning( "gp_rpc_auth_cram_server: unknown hash type" ); return NULL; }

  g_rw_lock_init( &priv->lock );
  priv->sessions = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, (GDestroyNotify)gp_rpc_auth_cram_server_session_free);
  priv->session_clean = g_thread_new ("auth cram server session clean", gp_rpc_auth_cram_server_sessions_clean, priv);

  return gp_rpc_auth_cram;

}


static void gp_rpc_auth_cram_server_finalize (GpRpcAuthCramServer *gp_rpc_auth_cram)
{

  GpRpcAuthCramServerPriv *priv = GP_RPC_AUTH_CRAM_SERVER_GET_PRIVATE(gp_rpc_auth_cram);

  g_atomic_int_set( &priv->close, TRUE );
  if( priv->session_clean != NULL )
    g_thread_join( priv->session_clean );

  if( priv->sessions != NULL )
    g_hash_table_unref( priv->sessions );

  if( priv->keys != NULL )
    g_list_free_full (priv->keys, gp_rpc_auth_cram_server_key_free);

  G_OBJECT_CLASS( gp_rpc_auth_cram_server_parent_class )->finalize(gp_rpc_auth_cram);

}


static gpointer gp_rpc_auth_cram_server_sessions_clean (gpointer data)
{

  GpRpcAuthCramServerPriv *priv = data;

  GHashTableIter session_iter;
  gpointer key, value;
  GpRpcAuthCramServerSession *session;

  guint32 timeout;
  GTimeVal cur_time;

  while( g_atomic_int_get( &priv->close ) != TRUE )
  {

    g_rw_lock_writer_lock( &priv->lock );

    g_get_current_time( &cur_time );
    g_hash_table_iter_init( &session_iter, priv->sessions );
    while( g_hash_table_iter_next( &session_iter, &key, &value ) )
    {

      session = value;

      if( ( session->status == GP_RPC_AUTH_CRAM_STATUS_CHECKED ) || ( session->status == GP_RPC_AUTH_CRAM_STATUS_AUTH ) )
        timeout = priv->timeout;
      else
        timeout = priv->auth_timeout;

      if( cur_time.tv_sec - session->last_used.tv_sec > timeout )
      {
        g_hash_table_iter_steal( &session_iter );
        gp_rpc_auth_cram_server_session_free(session);
      }

    }

    g_rw_lock_writer_unlock( &priv->lock );

    g_usleep( G_USEC_PER_SEC );

  }

  return NULL;

}


GpRpcAuthCramServer *gp_rpc_auth_cram_server_new (GChecksumType hash_type, guint32 chalenge_size, guint32 max_sessions,
                                                  guint32 auth_timeout, guint32 timeout)
{

  return g_object_new( G_TYPE_RPC_AUTH_CRAM_SERVER, "hash-type", hash_type, "chalenge-size", chalenge_size, "max-sessions", max_sessions, "auth-timeout", auth_timeout, "timeout", timeout, NULL );

}


gboolean gp_rpc_auth_cram_server_add_key (GpRpcAuthCramServer *gp_rpc_auth_cram, const gchar *key64, gpointer data)
{

  GpRpcAuthCramServerPriv *priv = GP_RPC_AUTH_CRAM_SERVER_GET_PRIVATE(gp_rpc_auth_cram);

  guchar *key;
  gsize key_size;
  GList *key_info_ptr = priv->keys;
  GpRpcAuthCramServerKey *key_info;

  key = g_base64_decode( key64, &key_size );
  if( key_size < GP_RPC_AUTH_CRAM_MIN_KEY_SIZE || key_size > GP_RPC_AUTH_CRAM_MAX_KEY_SIZE )
  { g_warning( "gp_rpc_auth_cram_server: wrong key size" ); return FALSE; }

  g_rw_lock_reader_lock( &priv->lock );

  while( key_info_ptr != NULL )
  {
    key_info = key_info_ptr->data;
    if( key_info->key_size == key_size && memcmp( key_info->key, key, key_size ) == 0 )
    { g_rw_lock_reader_unlock( &priv->lock ); g_warning( "gp_rpc_auth_cram_server: key already exists" ); return FALSE; }
    key_info_ptr = g_list_next( key_info_ptr );
    key_info = NULL;
  }
  g_rw_lock_reader_unlock( &priv->lock );

  key_info = g_malloc( sizeof(GpRpcAuthCramServerKey) );

  key_info->hmac = g_hmac_new( priv->hash_type, key, key_size );
  key_info->key = g_memdup( key, key_size );
  key_info->key_size = key_size;
  key_info->data = data;

  g_rw_lock_writer_lock( &priv->lock );
  priv->keys = g_list_prepend( priv->keys, key_info );
  g_rw_lock_writer_unlock( &priv->lock );

  return TRUE;

}


GpRpcAuthStatus gp_rpc_auth_cram_server_check (GpRpcAuth *gp_rpc_auth_cram, GpRpcData *gp_rpc_data)
{

  GpRpcAuthCramServerPriv *priv = GP_RPC_AUTH_CRAM_SERVER_GET_PRIVATE(gp_rpc_auth_cram);

  GpRpcHeader *iheader = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_INPUT);

  gint status;
  GpRpcAuthCramServerSession *session;

  guchar *server_chalenge;
  guchar *client_chalenge; guint32 client_chalenge_size;
  guchar *client_response; guint32 client_response_size;

  GHmac   *hmac;
  guchar   hmac_buffer[GP_RPC_AUTH_CRAM_MAX_HASH_SIZE ];
  gsize    hmac_size = sizeof( hmac_buffer );
  gpointer data_to_hmac = gp_rpc_data_get_data (gp_rpc_data, GP_RPC_DATA_INPUT);
  guint32  data_size_to_hmac = gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_SIGNED_SIZE);

  g_rw_lock_reader_lock( &priv->lock );
  session = g_hash_table_lookup( priv->sessions, GUINT_TO_POINTER( GUINT32_FROM_BE( iheader->session ) ) );
  g_rw_lock_reader_unlock( &priv->lock );

  // Отсутствует запрашиваемая сессия.
  if( session == NULL )
  {
    // Новое подключение?
    if(gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_STATUS) == GP_RPC_AUTH_CRAM_STATUS_BEGIN ) return GP_RPC_AUTH_IN_PROGRESS;
    g_warning( "gp_rpc_auth_cram_server: unknown session" );
    return GP_RPC_AUTH_FAILED;
  }

  // Кто-то уже использует эту сессию!?
  if( !g_mutex_trylock( &session->lock ) ) { g_warning( "gp_rpc_auth_cram_server: session already used" ); return GP_RPC_AUTH_FAILED; }

  // Аутентификация уже проверена, можно выполнять запрос.
  if( session->status == GP_RPC_AUTH_CRAM_STATUS_CHECKED ) return GP_RPC_AUTH_AUTHENTICATED;

  // Неверный статус аутентификации.
  if( session->status != GP_RPC_AUTH_CRAM_STATUS_BEGIN && session->status != GP_RPC_AUTH_CRAM_STATUS_AUTH )
  { status = GP_RPC_AUTH_FAILED;  g_warning( "gp_rpc_auth_cram_server: unknown session status" ); goto gp_rpc_auth_cram_server_check_exit; }

  // Не совпадает тип аутентификации.
  if(gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_PARAM_AUTH_TYPE) != GP_RPC_AUTH_CRAM_TYPE_ID )
  { status = GP_RPC_AUTH_NOT_SUPPORTED;  g_warning( "gp_rpc_auth_cram_server: unknown authentication type" ); goto gp_rpc_auth_cram_server_check_exit; }

  if(gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_STATUS) != GP_RPC_AUTH_CRAM_STATUS_AUTH )
  { status = GP_RPC_AUTH_FAILED;  g_warning( "gp_rpc_auth_cram_server: unknown authentication status" ); goto gp_rpc_auth_cram_server_check_exit; }

  client_chalenge = gp_rpc_data_get (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_CHALENGE_C, &client_chalenge_size);
  client_response = gp_rpc_data_get (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_RESPONSE_C, &client_response_size);

  if( client_response == NULL || client_response_size == 0 || client_chalenge == NULL || client_chalenge_size == 0 || client_chalenge_size != priv->chalenge_size )
  { status = GP_RPC_AUTH_FAILED;  g_warning( "gp_rpc_auth_cram_server: incomplete authentication request" ); goto gp_rpc_auth_cram_server_check_exit; }

  // Если пришел запрос с предыдущим идентификатором, значит наш ответ
  // клиенту не пришел. Воспользуемся предыдущей строкой запроса.
  if( GUINT32_FROM_BE( iheader->sequence ) == session->next_sequence )
    server_chalenge = session->server_chalenge;
  else
    server_chalenge = session->prev_server_chalenge;

  // Первый ответ клиента - ищем ключ с которым он пытается авторизоваться.
  if( session->status == GP_RPC_AUTH_CRAM_STATUS_BEGIN )
  {

    GList *key_info_ptr;
    GpRpcAuthCramServerKey *key_info = NULL;

    g_rw_lock_reader_lock( &priv->lock );

    key_info_ptr = priv->keys;
    while( key_info_ptr != NULL )
    {

      key_info = key_info_ptr->data;

      // Вычисляем каким должен быть ответ клиента.
      hmac = g_hmac_copy( key_info->hmac );
      g_hmac_update( hmac, data_to_hmac, data_size_to_hmac );
      g_hmac_update( hmac, client_chalenge, client_chalenge_size );
      g_hmac_update( hmac, server_chalenge, priv->chalenge_size );
      g_hmac_get_digest( hmac, hmac_buffer, &hmac_size );
      g_hmac_unref( hmac );

      // Сравниваем с реальным ответом. Завершаем поиск если ответы совпали.
      if( hmac_size == client_response_size && memcmp( hmac_buffer, client_response, hmac_size ) == 0 )
        break;

      key_info_ptr = g_list_next( key_info_ptr );
      key_info = NULL;

    }

    g_rw_lock_reader_unlock( &priv->lock );

    // Если совпадений не было найдено - аутентификация не прошла.
    if( !key_info )
    {
      session->status = GP_RPC_AUTH_CRAM_STATUS_FAIL;
      status = GP_RPC_AUTH_FAILED;
      g_warning( "gp_rpc_auth_cram_server: unknown user key" );
      goto gp_rpc_auth_cram_server_check_exit;
    }

    // Запоминаем рабочий ключ клиента.
    session->key = key_info;

    g_get_current_time( &session->last_used );

    session->status = GP_RPC_AUTH_CRAM_STATUS_CHECKED;
    return GP_RPC_AUTH_AUTHENTICATED;

  }

  // Нормальный запрос аутентификации.
  if( session->status == GP_RPC_AUTH_CRAM_STATUS_AUTH )
  {

    // Вычисляем каким должен быть ответ клиента.
    hmac = g_hmac_copy( session->key->hmac );
    g_hmac_update( hmac, data_to_hmac, data_size_to_hmac );
    g_hmac_update( hmac, client_chalenge, client_chalenge_size );
    g_hmac_update( hmac, server_chalenge, priv->chalenge_size );
    g_hmac_get_digest( hmac, hmac_buffer, &hmac_size );
    g_hmac_unref( hmac );

    // Сравниваем с реальным ответом.
    if( hmac_size != client_response_size || memcmp( hmac_buffer, client_response, hmac_size ) )
    { status = GP_RPC_AUTH_FAILED;  g_warning( "gp_rpc_auth_cram_server: digest error" ); goto gp_rpc_auth_cram_server_check_exit; }

    g_get_current_time( &session->last_used );

    session->status = GP_RPC_AUTH_CRAM_STATUS_CHECKED;
    return GP_RPC_AUTH_AUTHENTICATED;

  }

  status = GP_RPC_AUTH_FAILED;

  gp_rpc_auth_cram_server_check_exit:
  g_mutex_unlock( &session->lock );

  return status;

}


gboolean gp_rpc_auth_cram_server_authenticate (GpRpcAuth *gp_rpc_auth_cram, GpRpcData *gp_rpc_data)
{

  GpRpcAuthCramServerPriv *priv = GP_RPC_AUTH_CRAM_SERVER_GET_PRIVATE(gp_rpc_auth_cram);

  GpRpcHeader *iheader = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_INPUT);
  GpRpcHeader *oheader = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_OUTPUT);

  GpRpcAuthCramServerSession *session;
  guint32 new_session_id;
  guint32 i;

  g_rw_lock_reader_lock( &priv->lock );
  session = g_hash_table_lookup( priv->sessions, GUINT_TO_POINTER( GUINT32_FROM_BE( iheader->session ) ) );
  g_rw_lock_reader_unlock( &priv->lock );

  // Отсутствует запрашиваемая сессия и статус аутентификации не в начале.
  if( session == NULL &&
      gp_rpc_data_get_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_STATUS) != GP_RPC_AUTH_CRAM_STATUS_BEGIN )
  { g_warning( "gp_rpc_auth_cram_server: can't authenticate, unknown status" ); return FALSE; }

  // Новое подключение.
  if( session == NULL )
  {

    // Если слишком много клиентов.
    g_rw_lock_reader_lock( &priv->lock );
    i = g_hash_table_size( priv->sessions );
    g_rw_lock_reader_unlock( &priv->lock );
    if( i > priv->max_sessions )
    { g_warning( "gp_rpc_auth_cram_server: too many sessions" ); return FALSE; }

    // Структура для нового клиента.
    session = g_slice_new( GpRpcAuthCramServerSession );
    session->server_chalenge = g_malloc( priv->chalenge_size );
    session->prev_server_chalenge = g_malloc( priv->chalenge_size );

    // Состояние и время запроса.
    session->status = GP_RPC_AUTH_CRAM_STATUS_BEGIN;
    g_get_current_time( &session->last_used );
    session->key = NULL;
    g_mutex_init( &session->lock );

    // Генерируем новый идентификатор сессии.
    g_rw_lock_writer_lock( &priv->lock );
    do {
      new_session_id = g_random_int();
    } while( new_session_id == 0 || g_hash_table_lookup( priv->sessions, GUINT_TO_POINTER( new_session_id ) ) != NULL );
    g_hash_table_insert( priv->sessions, GUINT_TO_POINTER( new_session_id ), session );
    g_rw_lock_writer_unlock( &priv->lock );

    oheader->session = GUINT32_TO_BE( new_session_id );
    session->next_sequence = GUINT32_FROM_BE( iheader->sequence ) + 1;

    // Генерируем нашу строку запроса.
    for( i = 0; i < ( priv->chalenge_size / sizeof( guint32 ) ); i++ )
      *( ( ( guint32* )session->server_chalenge ) + i ) = g_random_int();

    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_AUTH_TYPE, GP_RPC_AUTH_CRAM_TYPE_ID);
    if( priv->hash_type == G_CHECKSUM_MD5 )
      gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_HASH,
                              GP_RPC_AUTH_CRAM_HASH_MD5);
    else if( priv->hash_type == G_CHECKSUM_SHA1 )
      gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_HASH,
                              GP_RPC_AUTH_CRAM_HASH_SHA1);
    else gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_HASH, GP_RPC_AUTH_CRAM_HASH_SHA256);

    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_STATUS, GP_RPC_AUTH_CRAM_STATUS_ONLY_ONE);
    gp_rpc_data_set (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_CHALENGE_S, session->server_chalenge, priv->chalenge_size);

    return TRUE;

  }

  // Ответ клиента проверен, можно отправлять наш ответ и новую строку запроса.
  if( session->status == GP_RPC_AUTH_CRAM_STATUS_CHECKED )
  {

    GHmac   *hmac;
    guchar   hmac_buffer[GP_RPC_AUTH_CRAM_MAX_HASH_SIZE ];
    gsize    hmac_size = sizeof( hmac_buffer );
    gpointer data_to_hmac = gp_rpc_data_get_data (gp_rpc_data, GP_RPC_DATA_OUTPUT);
    guint32  data_size_to_hmac;

    guint32 client_chalenge_size;
    guchar *client_chalenge;
    guchar *server_chalenge;

    // Если пришел запрос с предыдущим идентификатором, значит наш ответ
    // клиенту не пришел. Воспользуемся предыдущей строкой запроса.
    if( GUINT32_FROM_BE( iheader->sequence ) == session->next_sequence )
      server_chalenge = session->server_chalenge;
    else
      server_chalenge = session->prev_server_chalenge;

    client_chalenge = gp_rpc_data_get (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_CHALENGE_C, &client_chalenge_size);

    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_SIGNED_SIZE, 0);
    data_size_to_hmac = gp_rpc_data_get_data_size (gp_rpc_data, GP_RPC_DATA_OUTPUT);
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_SIGNED_SIZE, data_size_to_hmac);

    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_PARAM_AUTH_TYPE, GP_RPC_AUTH_CRAM_TYPE_ID);
    gp_rpc_data_set_uint32 (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_STATUS, GP_RPC_AUTH_CRAM_STATUS_RESPONSE);

    // Вычисляем ответ клиенту.
    hmac = g_hmac_copy( session->key->hmac );
    g_hmac_update( hmac, data_to_hmac, data_size_to_hmac );
    g_hmac_update( hmac, server_chalenge, priv->chalenge_size );
    g_hmac_update( hmac, client_chalenge, client_chalenge_size );
    g_hmac_get_digest( hmac, hmac_buffer, &hmac_size );
    g_hmac_unref( hmac );

    if( GUINT32_FROM_BE( iheader->sequence ) == session->next_sequence )
    {
      gpointer swap;
      swap = session->server_chalenge;
      session->server_chalenge = session->prev_server_chalenge;
      session->prev_server_chalenge = swap;
      session->next_sequence++;
    }

    // Генерируем новую строку запроса.
    for( i = 0; i < ( priv->chalenge_size / sizeof( guint32 ) ); i++ )
      *( ( ( guint32* )session->server_chalenge ) + i ) = g_random_int();

    oheader->session = iheader->session;

    gp_rpc_data_set (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_CHALENGE_S, session->server_chalenge, priv->chalenge_size);
    gp_rpc_data_set (gp_rpc_data, GP_RPC_AUTH_CRAM_PARAM_RESPONSE_S, hmac_buffer, hmac_size);

    g_get_current_time( &session->last_used );
    g_mutex_unlock( &session->lock );

    session->status = GP_RPC_AUTH_CRAM_STATUS_AUTH;

    return TRUE;

  }

  g_warning( "gp_rpc_auth_cram_server: unknown error" );
  return FALSE;

}


gpointer gp_rpc_auth_cram_server_get_user_data (GpRpcAuth *gp_rpc_auth_cram, GpRpcData *gp_rpc_data)
{

  GpRpcAuthCramServerPriv *priv = GP_RPC_AUTH_CRAM_SERVER_GET_PRIVATE(gp_rpc_auth_cram);

  GpRpcHeader *header = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_INPUT);

  GpRpcAuthCramServerSession *session;

  g_rw_lock_reader_lock( &priv->lock );
  session = g_hash_table_lookup( priv->sessions, GUINT_TO_POINTER( GUINT32_FROM_BE( header->session ) ) );
  g_rw_lock_reader_unlock( &priv->lock );

  if( session == NULL ) return NULL;
  if( session->status != GP_RPC_AUTH_CRAM_STATUS_CHECKED && session->status != GP_RPC_AUTH_CRAM_STATUS_AUTH )
  { g_warning( "gp_rpc_auth_cram_server: unknown user" ); return NULL; }

  return session->key->data;

}


gboolean gp_rpc_auth_cram_server_clean_session (GpRpcAuth *gp_rpc_auth_cram, GpRpcData *gp_rpc_data)
{

  GpRpcAuthCramServerPriv *priv = GP_RPC_AUTH_CRAM_SERVER_GET_PRIVATE(gp_rpc_auth_cram);

  GpRpcHeader *header = gp_rpc_data_get_header (gp_rpc_data, GP_RPC_DATA_INPUT);

  GpRpcAuthCramServerSession *session;

  g_rw_lock_writer_lock( &priv->lock );
  session = g_hash_table_lookup( priv->sessions, GUINT_TO_POINTER( GUINT32_FROM_BE( header->session ) ) );
  g_hash_table_steal( priv->sessions, GUINT_TO_POINTER( GUINT32_FROM_BE( header->session ) ) );
  g_rw_lock_writer_unlock( &priv->lock );

  if(session != NULL)
    gp_rpc_auth_cram_server_session_free(session);

  return session != NULL ? TRUE : FALSE;

}


static void gp_rpc_auth_cram_server_key_free (gpointer data)
{

  GpRpcAuthCramServerKey *key_info = data;

  g_hmac_unref( key_info->hmac );
  g_free( key_info->key );
  g_free( key_info );

}


static void gp_rpc_auth_cram_server_session_free(GpRpcAuthCramServerSession *session_info)
{
  g_mutex_clear(&session_info->lock);
  g_free(session_info->server_chalenge);
  g_free(session_info->prev_server_chalenge);
  g_slice_free(GpRpcAuthCramServerSession, session_info);
}


static void gp_rpc_auth_cram_server_interface_init (GpRpcAuthInterface *iface)
{

  iface->check = gp_rpc_auth_cram_server_check;
  iface->authenticate = gp_rpc_auth_cram_server_authenticate;
  iface->get_user_data = gp_rpc_auth_cram_server_get_user_data;
  iface->clean_session = gp_rpc_auth_cram_server_clean_session;

}
