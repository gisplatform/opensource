#include "gp-rpc-common.h"
#include "gp-rpc-auth-cram.h"
#include "gp-rpc-auth-cram-server.h"

#include <string.h>

#define BUFFER_SIZE   16*1024*1024


void swap_buffers( GpRpcData *grpc_data )
{

  guint32 size1 = gp_rpc_data_get_data_size (grpc_data, GP_RPC_DATA_INPUT);
  guint32 size2 = gp_rpc_data_get_data_size (grpc_data, GP_RPC_DATA_OUTPUT);

  gpointer buffer1 = g_memdup(gp_rpc_data_get_data (grpc_data, GP_RPC_DATA_INPUT), size1 );
  gpointer buffer2 = g_memdup(gp_rpc_data_get_data (grpc_data, GP_RPC_DATA_OUTPUT), size2 );

  gp_rpc_data_set_data (grpc_data, GP_RPC_DATA_INPUT, buffer2, size2);
  gp_rpc_data_set_data (grpc_data, GP_RPC_DATA_OUTPUT, buffer1, size1);

  g_free( buffer1 );
  g_free( buffer2 );

}


int main( int argc, char **argv )
{

  guint   i;
  guchar *key;
  gchar  *key64;
  guint   key_size = 32;

  GpRpcHeader *iheader;
  GpRpcHeader *oheader;
  GpRpcData *grpc_data;
  gpointer ibuffer;
  gpointer obuffer;

  GpRpcAuth *client, *server;

  GpRpcAuthStatus auth_status;

  #if !GLIB_CHECK_VERSION( 2, 36, 0 )
    g_type_init();
  #endif

  ibuffer = g_malloc( BUFFER_SIZE );
  obuffer = g_malloc( BUFFER_SIZE );
  grpc_data = gp_rpc_data_new (BUFFER_SIZE, GP_RPC_HEADER_SIZE, ibuffer, obuffer);

  key = g_malloc( key_size );
  for( i = 0; i < key_size / 4; i++ )
    *( ( ( guint32* )key ) + i ) = GUINT32_TO_BE( g_random_int() );
  key64 = g_base64_encode( key, key_size );

  client = GP_RPC_AUTH(gp_rpc_auth_cram_new (key64) );
  server = GP_RPC_AUTH(gp_rpc_auth_cram_server_new (G_CHECKSUM_SHA256, 128, GP_RPC_AUTH_CRAM_MAX_SESSIONS, 5, 500) );
  gp_rpc_auth_cram_server_add_key (GP_RPC_AUTH_CRAM_SERVER(server), key64, NULL);

  iheader = gp_rpc_data_get_header (grpc_data, GP_RPC_DATA_INPUT);
  oheader = gp_rpc_data_get_header (grpc_data, GP_RPC_DATA_OUTPUT);
  iheader->session = 0;
  oheader->session = 0;
  oheader->sequence = 0;

  while( TRUE )
    {

      gp_rpc_data_set_data_size (grpc_data, GP_RPC_DATA_OUTPUT, 0);
    auth_status = gp_rpc_auth_check (client, grpc_data);
    if( auth_status != GP_RPC_AUTH_NOT_AUTHENTICATED && auth_status != GP_RPC_AUTH_IN_PROGRESS) break;
      gp_rpc_auth_authenticate (client, grpc_data);

    swap_buffers( grpc_data );
    iheader->session = oheader->session;
    oheader->sequence++;
    iheader->sequence = GUINT32_TO_BE( oheader->sequence );

      gp_rpc_data_set_data_size (grpc_data, GP_RPC_DATA_OUTPUT, 0);
      gp_rpc_auth_check (server, grpc_data);
      gp_rpc_auth_authenticate (server, grpc_data);
    swap_buffers( grpc_data );
    iheader->session = oheader->session;

    }

  if( auth_status == GP_RPC_AUTH_AUTHENTICATED)
    g_message( "Authenticated with session id = %u", GUINT32_FROM_BE( iheader->session ) );
  if( auth_status == GP_RPC_AUTH_FAILED)
    g_message( "Failed" );
  if( auth_status == GP_RPC_AUTH_NOT_SUPPORTED)
    g_message( "Not supported" );

  g_usleep( 7 * G_USEC_PER_SEC );

  g_object_unref( client );
  g_object_unref( server );
  g_object_unref( grpc_data );

  g_free( key64 );
  g_free( key );

  return 0;

}
