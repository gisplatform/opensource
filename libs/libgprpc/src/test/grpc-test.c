#include "urpc.h"
#include "urpc-server.h"
#include "gp-rpc-common.h"
#include "gp-rpc-auth-cram.h"
#include "gp-rpc-auth-cram-server.h"

#include <stdio.h>
#include <string.h>


#define GRPC_PROC_ID1        GP_RPC_PROC_USER + 1
#define GRPC_PROC_ID2        GP_RPC_PROC_USER + 2
#define GRPC_PROC_ID3        GP_RPC_PROC_USER + 3
#define GRPC_PROC_ID4        GP_RPC_PROC_USER + 4

#define GRPC_OBJ_ID1         GP_RPC_PROC_USER + 1
#define GRPC_OBJ_ID2         GP_RPC_PROC_USER + 2
#define GRPC_OBJ_ID3         GP_RPC_PROC_USER + 3
#define GRPC_OBJ_ID4         GP_RPC_PROC_USER + 4

#define RPC_STATUS_PARAM        GP_RPC_PARAM_USER + 1
#define RPC_ID_PARAM            GP_RPC_PARAM_USER + 2
#define RPC_DATA_PARAM          GP_RPC_PARAM_USER + 3
#define RPC_FLOAT1_PARAM        GP_RPC_PARAM_USER + 4
#define RPC_FLOAT2_PARAM        GP_RPC_PARAM_USER + 5
#define RPC_FLOAT3_PARAM        GP_RPC_PARAM_USER + 6
#define RPC_DOUBLE1_PARAM       GP_RPC_PARAM_USER + 7
#define RPC_DOUBLE2_PARAM       GP_RPC_PARAM_USER + 8
#define RPC_DOUBLE3_PARAM       GP_RPC_PARAM_USER + 9
#define RPC_MD5_PARAM           GP_RPC_PARAM_USER + 10
#define RPC_RTHREADS_PARAM      GP_RPC_PARAM_USER + 11
#define RPC_UNREAD_PARAM        GP_RPC_PARAM_USER + 12
#define RPC_NON_EXISTENT_PARAM  GP_RPC_PARAM_USER + 13


typedef struct GRpcTestStat {

  gchar *uri;

  gchar **key64;

  gboolean use_auth;
  gboolean non_existent;
  guint request_size;
  guint threads_num;
  guint requests_num;

  guint8 *client_data;
  guint8 client_data_md5[1024];
  gsize client_data_md5_size;

  volatile gint server_started;
  volatile gint server_failed;
  volatile gint shutdown;

  GMutex mutex;

} GRpcTestStat;


gboolean grpc_acl( GpRpcManager *grpc_manager, guint32 proc_id, guint32 obj_id, gpointer auth_data )
{

  guint client_id = GPOINTER_TO_UINT( auth_data );

  if( proc_id != obj_id || GP_RPC_PROC_USER + ( client_id % 4 ) + 1 != proc_id )
    {
    g_message( "Access denied for client %d calling proc_id = %d", client_id, proc_id );
    return FALSE;
    }

  return TRUE;

}


gboolean grpc_proc( GpRpcManager *grpc_manager, GpRpcData *grpc_data, gpointer object )
{

  static volatile guint running_threads = 0;

  gpointer id;
  gpointer data;
  guint32 data_size;

  gfloat fvalue1, fvalue2;
  gdouble dvalue1, dvalue2;

  GChecksum *data_sum;
  guint8 data_md5[1024];
  gsize data_md5_size = 1024;

  gboolean status = FALSE;

  g_atomic_int_add( &running_threads, 1 );

  id = gp_rpc_data_get (grpc_data, RPC_ID_PARAM, NULL);
  data = gp_rpc_data_get (grpc_data, RPC_DATA_PARAM, &data_size);
  if( id && data )
    {
    data_sum = g_checksum_new( G_CHECKSUM_MD5 );
    g_checksum_update( data_sum, data, data_size );
    g_checksum_get_digest( data_sum, data_md5, &data_md5_size );
    g_checksum_free( data_sum );
      gp_rpc_data_set (grpc_data, RPC_MD5_PARAM, data_md5, data_md5_size);
    status = TRUE;
    }

  fvalue1 = gp_rpc_data_get_float (grpc_data, RPC_FLOAT1_PARAM);
  fvalue2 = gp_rpc_data_get_float (grpc_data, RPC_FLOAT2_PARAM);
  dvalue1 = gp_rpc_data_get_double (grpc_data, RPC_DOUBLE1_PARAM);
  dvalue2 = gp_rpc_data_get_double (grpc_data, RPC_DOUBLE2_PARAM);

  gp_rpc_data_set_float (grpc_data, RPC_FLOAT3_PARAM, fvalue1 + fvalue2);
  gp_rpc_data_set_double (grpc_data, RPC_DOUBLE3_PARAM, dvalue1 - dvalue2);

  gp_rpc_data_set_uint32 (grpc_data, RPC_STATUS_PARAM, status);
  gp_rpc_data_set_uint32 (grpc_data, RPC_RTHREADS_PARAM, running_threads);

  // Устанавливаем параметр, который никто не читает.
  // Можно, например, проверить, не смешается ли он с RPC_NON_EXISTENT_PARAM или еще с чем.
  gp_rpc_data_set_uint32(grpc_data, RPC_UNREAD_PARAM, 12309);

  g_atomic_int_add( &running_threads, -1 );

  return TRUE;

}


gpointer server_thread( gpointer data )
{

  GRpcTestStat *stat = data;

  GError *err = NULL;
  GpRpcAuth *grpc_auth = NULL;
  GpRpcManager *grpc_manager = NULL;
  GpRpcServer *grpc_server = NULL;

  gchar *uri = NULL;

  if( stat->use_auth )
    {
    guint i;
    grpc_auth = GP_RPC_AUTH(gp_rpc_auth_cram_server_new (G_CHECKSUM_SHA256, 128, GP_RPC_AUTH_CRAM_MAX_SESSIONS,
                                                       GP_RPC_AUTH_CRAM_AUTH_TIMEOUT,
                                                       GP_RPC_AUTH_CRAM_TIMEOUT) );
    if( !grpc_auth )
      {
      g_atomic_int_set( &stat->server_failed, TRUE );
      g_message( "Failed to create authentication object for server" );
      goto server_exit;
      }
    for( i = 0; i < stat->threads_num; i++ )
      gp_rpc_auth_cram_server_add_key (GP_RPC_AUTH_CRAM_SERVER(grpc_auth), stat->key64[i], GUINT_TO_POINTER(i));
    }
  else
    grpc_auth = NULL;

  grpc_manager = gp_rpc_manager_new ();
  if( !grpc_manager )
    {
    g_atomic_int_set( &stat->server_failed, TRUE );
    g_message( "Failed to create gprpc manager object for server" );
    goto server_exit;
    }

  gp_rpc_manager_reg_proc (grpc_manager, GRPC_PROC_ID1, grpc_proc, TRUE);
  gp_rpc_manager_reg_proc (grpc_manager, GRPC_PROC_ID2, grpc_proc, TRUE);
  gp_rpc_manager_reg_proc (grpc_manager, GRPC_PROC_ID3, grpc_proc, TRUE);
  gp_rpc_manager_reg_proc (grpc_manager, GRPC_PROC_ID4, grpc_proc, TRUE);

  gp_rpc_manager_reg_obj (grpc_manager, GRPC_OBJ_ID1, stat, TRUE);
  gp_rpc_manager_reg_obj (grpc_manager, GRPC_OBJ_ID2, stat, TRUE);
  gp_rpc_manager_reg_obj (grpc_manager, GRPC_OBJ_ID3, stat, TRUE);
  gp_rpc_manager_reg_obj (grpc_manager, GRPC_OBJ_ID4, stat, TRUE);

  grpc_server = gp_rpc_server_create (stat->uri, stat->threads_num, GP_RPC_DEFAULT_DATA_SIZE,
                                      GP_RPC_DEFAULT_DATA_TIMEOUT,
                                      grpc_auth, grpc_acl, grpc_manager, &err);
  if(err)
  {
    g_atomic_int_set( &stat->server_failed, TRUE );
    g_message("Server failed: %s", err->message);
    g_error_free(err);
    goto server_exit;
  }

  uri = gp_rpc_server_get_self_uri (grpc_server);

  g_message( "GpRpc server is started at %s", uri );
  g_atomic_int_set( &stat->server_started, TRUE );

  while( !g_atomic_int_get( &stat->shutdown ) ) g_usleep( 100000 );

  server_exit:
    g_free( uri );
    if( grpc_server ) g_object_unref( grpc_server );
    if( grpc_manager ) g_object_unref( grpc_manager );
    if( grpc_auth ) g_object_unref( grpc_auth );

  g_message( "GpRpc server is stopped" );

  return NULL;

}


gpointer client_thread( gpointer data )
{

  GRpcTestStat *stat = data;

  GpRpc *grpc = NULL;
  GpRpcAuth *grpc_auth = NULL;
  GError *error = NULL;

  gchar *self_uri = NULL;
  gchar *server_uri = NULL;

  guint i;
  GpRpcData *grpc_data;

  gpointer data_md5;
  guint32 data_md5_size;
  gboolean status;

  gfloat fvalue1, fvalue2, fvalue3;
  gdouble dvalue1, dvalue2, dvalue3;

  static volatile gint total_clients = 0;
  static volatile gint clients = 0;
  gint client_id = g_atomic_int_add( &clients, 1 );
  gint proc_obj_id = GP_RPC_PROC_USER + ( client_id % 4 ) + 1 + (stat->non_existent ? 1 : 0);

  GTimer *timer1 = g_timer_new();
  GTimer *timer2 = g_timer_new();
  gdouble all_time = 0;
  gdouble work_time = 0;
  guint32 rpc_calls = 0;

  guint32 *running_threads = g_malloc( stat->threads_num * sizeof( guint32 ) );

  fvalue1 = g_random_double();
  fvalue2 = g_random_double();
  fvalue3 = fvalue1 + fvalue2;
  dvalue1 = g_random_double();
  dvalue2 = g_random_double();
  dvalue3 = dvalue1 - dvalue2;

  for( i = 0; i < stat->threads_num; i++ )
    running_threads[i] = 0;

  g_message( "Client thread with id = %d is waiting for server...", client_id );
  while( !g_atomic_int_get( &stat->server_started ) && !g_atomic_int_get( &stat->server_failed ) )
    g_usleep( 100 );

  if( g_atomic_int_get( &stat->server_failed ) )
    return NULL;

  if( stat->use_auth )
    {
    grpc_auth = GP_RPC_AUTH(gp_rpc_auth_cram_new (stat->key64[client_id % stat->threads_num]) );
    if( !grpc_auth )
      {
      g_message( "Failed to create authentication object for client with id = %d", client_id );
      goto client_exit;
      }
    }

  grpc = gp_rpc_create (stat->uri, GP_RPC_DEFAULT_DATA_SIZE, GP_RPC_DEFAULT_EXEC_TIMEOUT, GP_RPC_DEFAULT_RESTART, grpc_auth, NULL);
  if( !grpc )
    {
    g_message( "Failed to create gprpc client with id = %d", client_id );
    goto client_exit;
    }

  self_uri = gp_rpc_get_self_uri (grpc);
  server_uri = gp_rpc_get_server_uri (grpc);
  g_message( "GpRpc client with id = %d is started and connected %s -> %s", client_id, self_uri, server_uri );

  for( i = 0; i < stat->requests_num; i++ )
  {
    g_timer_start( timer1 );
    grpc_data = gp_rpc_lock (grpc);
    if( !grpc_data )
    {
      g_message( "Failed to lock gprpc client with id = %d", client_id );
      goto client_exit;
    }
    g_timer_start( timer2 );

    gp_rpc_data_set_uint32 (grpc_data, RPC_ID_PARAM, client_id);
    gp_rpc_data_set (grpc_data, RPC_DATA_PARAM, stat->client_data, stat->request_size);

    gp_rpc_data_set_float (grpc_data, RPC_FLOAT1_PARAM, fvalue1);
    gp_rpc_data_set_float (grpc_data, RPC_FLOAT2_PARAM, fvalue2);
    gp_rpc_data_set_double (grpc_data, RPC_DOUBLE1_PARAM, dvalue1);
    gp_rpc_data_set_double (grpc_data, RPC_DOUBLE2_PARAM, dvalue2);

    gp_rpc_exec(grpc, proc_obj_id, proc_obj_id, &error);

    if(G_UNLIKELY(error))
    {
      gboolean can_continue = (
        (G_OBJECT_TYPE(grpc) == G_TYPE_URPC) ||
        (g_error_matches(error, GP_RPC_ERROR, GP_RPC_ERROR_NOT_FOUND) && stat->non_existent));

      g_message("Failed to exec gprpc client with id = %d: %s", client_id, error->message);
      g_clear_error(&error);

      if(can_continue)
      {
        gp_rpc_unlock (grpc);
        continue;
      }

      goto client_exit;
    }

    data_md5 = gp_rpc_data_get (grpc_data, RPC_MD5_PARAM, &data_md5_size);
    status = gp_rpc_data_get_uint32 (grpc_data, RPC_STATUS_PARAM);

    running_threads[gp_rpc_data_get_uint32 (grpc_data, RPC_RTHREADS_PARAM) - 1 ] += 1;

    if( fvalue3 != gp_rpc_data_get_float (grpc_data, RPC_FLOAT3_PARAM) )
    {
      g_message( "float: local %.6f != remote %.6f", fvalue3, gp_rpc_data_get_float (grpc_data, RPC_FLOAT3_PARAM) );
      status = FALSE;
    }

    if(stat->non_existent)
    {
      gint32 param = gp_rpc_data_get_int32(grpc_data, RPC_NON_EXISTENT_PARAM);
      g_message("    reading unset param as int from non-free buffer: %4d, param %s 0",
                                                              param, (param == 0) ? "==" : "!=");
      g_assert_cmpint(param, ==, 0);
    }

    if( dvalue3 != gp_rpc_data_get_double (grpc_data, RPC_DOUBLE3_PARAM) )
    {
      g_message( "double: local %.6lf != remote %.6lf", dvalue3, gp_rpc_data_get_double (grpc_data, RPC_DOUBLE3_PARAM) );
      status = FALSE;
    }

    if(stat->non_existent)
    {
      guint32 uparam = gp_rpc_data_get_uint32(grpc_data, RPC_NON_EXISTENT_PARAM);
      g_message("    reading unset param as uint32 from  free buffer: %4u, param %s 0",
                                                              uparam, (uparam == 0) ? "==" : "!=");
      g_assert_cmpuint(uparam, ==, 0);
      gdouble param = gp_rpc_data_get_double(grpc_data, RPC_NON_EXISTENT_PARAM);
      g_message("    reading unset param as double from  free buffer: %.2lf, param %s 0.0",
                                                              param, (param == 0.0) ? "==" : "!=");
    }

    if( !data_md5 || ( memcmp( data_md5, stat->client_data_md5, data_md5_size ) != 0 ) || status != TRUE )
      g_message( "GpRpc call failed in client with id = %d, %d", client_id, proc_obj_id );
    else
      rpc_calls++;

    gp_rpc_unlock (grpc);
    work_time += g_timer_elapsed( timer2, NULL );
    all_time += g_timer_elapsed( timer1, NULL );
  }

  g_mutex_lock( &stat->mutex );
  g_message( "GpRpc client with id = %u stopped, total clients = %u", client_id, g_atomic_int_add( &total_clients, 1 ) + 1 );
  g_message( "  GpRpc client %d all time: %.4lf.", client_id, all_time );
  g_message( "  GpRpc client %d work time: %.4lf.", client_id, work_time );
  g_message( "  GpRpc client %d rpc calls: %d, %.0lf RPC/s, %.1lf Mb/s.", client_id, rpc_calls, stat->requests_num / all_time, ( stat->requests_num / all_time * stat->request_size ) / ( 1024 * 1024 ) );
  for( i = 0; i < stat->threads_num; i++ )g_message( "  GpRpc client %d running threads %d calls %d", client_id, i + 1, running_threads[i] );
  g_mutex_unlock( &stat->mutex );

  client_exit:
    g_free( running_threads );
    g_free( self_uri );
    g_free( server_uri );
    g_timer_destroy( timer1 );
    g_timer_destroy( timer2 );
    if( grpc ) g_object_unref( grpc );
    if( grpc_auth ) g_object_unref( grpc_auth );

    g_atomic_int_add( &clients, -1 );

  return NULL;

}


int main( int argc, char **argv )
{

  GRpcTestStat stat;
  GThread *server = NULL;
  GThread **clients = NULL;

  gboolean run_server = FALSE;
  gboolean run_clients = FALSE;

  guint    i, j;
  guchar  *key;
  guint    key_size = 32;

  gboolean use_auth = FALSE;
  gboolean non_existent = FALSE;
  guint    request_size = 1024;
  guint    threads_num = 4;
  guint    requests_num = 1000;
  guint    iterations_num = 1;

  GChecksum *data_sum;

  #if !GLIB_CHECK_VERSION( 2, 36, 0 )
    g_type_init();
  #endif

  { // Разбор командной строки.

  GError          *error = NULL;
  GOptionContext  *context;
  GOptionEntry     entries[] =
  {
    { "auth", 'a', 0, G_OPTION_ARG_NONE, &use_auth, "Use authentication", NULL },
    { "non-existent", 'n', 0, G_OPTION_ARG_NONE, &non_existent, "Call non-existent procedure of non-existent object and read non-existent param", NULL },
    { "size", 's', 0, G_OPTION_ARG_INT, &request_size, "Data size", NULL },
    { "threads", 't', 0, G_OPTION_ARG_INT, &threads_num, "Working threads (clients)", NULL },
    { "requests", 'r', 0, G_OPTION_ARG_INT, &requests_num, "Requests number", NULL },
    { "iterations", 'i', 0, G_OPTION_ARG_INT, &iterations_num, "Run clients threads multiple times", NULL },
    { "server-only", 0, 0, G_OPTION_ARG_NONE, &run_server, "Run only server ", NULL },
    { "clients-only", 0, 0, G_OPTION_ARG_NONE, &run_clients, "Run only clients", NULL },
    { NULL }
  };

  context = g_option_context_new("URI");
  g_option_context_set_help_enabled( context, TRUE );
  g_option_context_add_main_entries( context, entries, NULL );
  g_option_context_set_ignore_unknown_options( context, FALSE );
  if( !g_option_context_parse( context, &argc, &argv, &error ) )
    { g_message( error->message ); return -1; }

  if( argc < 2 )
    { g_print( "%s", g_option_context_get_help( context, FALSE, NULL ) ); return 0; }

  g_option_context_free( context );

  }

  if( !run_server && !run_clients )
    {
    run_server = TRUE;
    run_clients = TRUE;
    }

  // RPC URI
  stat.uri = argv[1];

  // Аутентификация.
  stat.use_auth = use_auth;
  if( use_auth )
    {
    g_random_set_seed( 0 );
    stat.key64 = g_malloc( threads_num * sizeof( gchar* ) );
    key = g_malloc( key_size );
    for( i = 0; i < threads_num; i++ )
      {
      for( j = 0; j < key_size / 4; j++ )
        *( ( ( guint32* )key ) + j ) = GUINT32_TO_BE( g_random_int() );
      stat.key64[i] = g_base64_encode( key, key_size );
      }
    g_free( key );
    }
  else
    stat.key64 = NULL;

  // Попробовать вызвать несуществующую процедуру несуществующего объекта.
  stat.non_existent = non_existent;

  if( request_size < 64 ) request_size = 64;
  if( request_size > GP_RPC_DEFAULT_DATA_SIZE - 1024 ) request_size = GP_RPC_DEFAULT_DATA_SIZE - 1024;
  if( threads_num < 1 ) threads_num = 1;
  if( threads_num > 128 ) threads_num = 128;
  if( requests_num < 1 ) requests_num = 1;

  // Параметры заросов.
  stat.request_size = request_size;
  stat.threads_num = threads_num;
  stat.requests_num = requests_num;

  stat.server_started = FALSE;
  stat.server_failed = FALSE;
  stat.shutdown = FALSE;

  // Данные для передачи.
  stat.client_data = g_malloc( request_size );
  for( i = 0; i < request_size; i++ )
    stat.client_data[i] = g_random_int();
  stat.client_data_md5_size = 1024;
  data_sum = g_checksum_new( G_CHECKSUM_MD5 );
  g_checksum_update( data_sum, stat.client_data, request_size );
  g_checksum_get_digest( data_sum, stat.client_data_md5, &stat.client_data_md5_size );
  g_checksum_free( data_sum );

  g_mutex_init( &stat.mutex );

  // Рабочие потоки.
  if( run_server )
    server = g_thread_new( "gprpc server", server_thread, &stat );
  else
    g_atomic_int_set( &stat.server_started, TRUE );

  if( run_clients )
    {

    clients = g_malloc( threads_num * sizeof( GThread* ) );

    for( j = 0; j < iterations_num; j++ )
      {

      for( i = 0; i < threads_num; i++ )
        clients[i] = g_thread_new( "gprpc client", client_thread, &stat );

      for( i = 0; i < threads_num; i++ )
        g_thread_join( clients[i] );

      }

    }

  if( run_server )
    {
    if( !run_clients ) { g_message( "Press [Enter] to terminate server..." ); getchar(); }
    g_atomic_int_set( &stat.shutdown, TRUE );
    g_thread_join( server );
    }

  return 0;

}
