#include "gp-rpc-common.h"
#include "gp-rpc-data.h"

#include <string.h>

#define BUFFER_SIZE   16*1024*1024
#define PARAMS_NUM    128

int main( int argc, char **argv )
{

  gint i;

  gsize file_size;
  gchar *file_data;

  GpRpcData *grpc_data;
  gpointer  buffer;

  gpointer  data;
  guint32   data_size;

  guint32   str_len_param;
  guint32   int_params[PARAMS_NUM],    int_param;
  gfloat    float_params[PARAMS_NUM],  float_param;
  gdouble   double_params[PARAMS_NUM], double_param;
  gchar    *str_params[PARAMS_NUM],   *str_param;

  #if !GLIB_CHECK_VERSION( 2, 36, 0 )
    g_type_init();
  #endif

  buffer = g_malloc( BUFFER_SIZE );
  grpc_data = gp_rpc_data_new (BUFFER_SIZE, GP_RPC_HEADER_SIZE, buffer, buffer);

  g_random_set_seed( 123 );

  for( i = 0; i < PARAMS_NUM; i++ )
    {
    int_params[i] = g_random_int();
    float_params[i] = 2.0 * ( g_random_double() - 0.5 );
    double_params[i] = 2.0 * ( g_random_double() - 0.5 );
    str_params[i] = g_strdup_printf( "Test string %u, %8.6f, %8.6lf", int_params[i], float_params[i], double_params[i] );
    }


  for( i = 0; i < PARAMS_NUM; i++ )
    {
    str_len_param = strlen( str_params[i] ) + 1;
      gp_rpc_data_set (grpc_data, 5 * i, str_params[i], str_len_param);
      gp_rpc_data_set_uint32 (grpc_data, 5 * i + 1, str_len_param);
      gp_rpc_data_set_uint32 (grpc_data, 5 * i + 2, int_params[i]);
      gp_rpc_data_set_float (grpc_data, 5 * i + 3, float_params[i]);
      gp_rpc_data_set_double (grpc_data, 5 * i + 4, double_params[i]);
    }

  data = gp_rpc_data_get_data (grpc_data, GP_RPC_DATA_OUTPUT);
  data_size = gp_rpc_data_get_data_size (grpc_data, GP_RPC_DATA_OUTPUT);
  g_file_set_contents( "datatest.dat", data, data_size, NULL );

  memset( buffer, 0, BUFFER_SIZE );

  g_file_get_contents( "datatest.dat", &file_data, &file_size, NULL );
  gp_rpc_data_set_data (grpc_data, GP_RPC_DATA_INPUT, file_data, file_size);

  for( i = 0; i < PARAMS_NUM; i++ )
    {

    str_param = gp_rpc_data_get_string (grpc_data, 5 * i);
    int_param = gp_rpc_data_get_uint32 (grpc_data, 5 * i + 1);
    str_len_param = strlen( str_param ) + 1;
    if( str_len_param != int_param )
      g_warning( "Parameter %d size mismatch %u != %u", i, str_len_param, int_param );
    if( g_strcmp0( str_param, str_params[i] ) )
      g_warning( "Parameter %d string mismatch", i );

    int_param = gp_rpc_data_get_uint32 (grpc_data, 5 * i + 2);
    if( int_params[i] != int_param )
      g_warning( "Parameter %d int mismatch", i );

    float_param = gp_rpc_data_get_float (grpc_data, 5 * i + 3);
    if( float_params[i] != float_param )
      g_warning( "Parameter %d float mismatch", i );

    double_param = gp_rpc_data_get_double (grpc_data, 5 * i + 4);
    if( double_params[i] != double_param )
      g_warning( "Parameter %d double mismatch", i );

    g_message( "%4u. Random int: %12u, float: %8.6f, double: %8.6lf, string: '%s'", i, int_param, float_param, double_param, str_param );

    }

  g_object_unref( grpc_data );
  g_free( buffer );

  return 0;

}
