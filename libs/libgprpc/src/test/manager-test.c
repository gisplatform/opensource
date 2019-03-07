#include "gp-rpc-manager.h"

#define N_THREADS 32
#define N_ITERS 10000

volatile guint mt_safe_proc_counter = 0;
guint mt_unsafe_proc_counter = 0;
guint mt_unsafe_in_proc_counter = 0;

volatile guint start = 0;

typedef struct PayLoad {

  GpRpcManager *grpc_manager;
  guint proc_id;
  guint obj_id;

} PayLoad;


gboolean mt_safe_proc( GpRpcManager *grpc_manager, GpRpcData *grpc_data, gpointer object )
{

  guint *counter = object;

  *counter = *counter + 1;

  return TRUE;

}


gboolean mt_unsafe_proc( GpRpcManager *grpc_manager, GpRpcData *grpc_data, gpointer object )
{

  volatile guint *counter = object;

  g_atomic_int_inc( counter );
  mt_unsafe_in_proc_counter = mt_unsafe_in_proc_counter + 1;

  return TRUE;

}


gpointer worker(gpointer data)
{
  PayLoad *pay_load = data;

  guint i;
  GpRpcServerCallback proc;
  gpointer obj;

  while(g_atomic_int_get(&start) == 0);

  for(i = 0; i < N_ITERS; i++)
  {
    proc = gp_rpc_manager_get_proc (pay_load->grpc_manager, pay_load->proc_id);
    obj = gp_rpc_manager_get_obj (pay_load->grpc_manager, pay_load->obj_id);

    if(!proc) g_message("fail on proc");
    if(!obj) g_message("fail on obj");

    if(proc && obj)
      proc(NULL, NULL, obj);
    else
      break;

    gp_rpc_manager_release_obj(pay_load->grpc_manager, pay_load->obj_id);
    gp_rpc_manager_release_proc(pay_load->grpc_manager, pay_load->proc_id);

    g_usleep(10);
  }

  return NULL;
}


int main( int argc, char **argv )
{

  GpRpcManager *grpc_manager;

  PayLoad pay_load_mt_safe;
  PayLoad pay_load_mt_unsafe;

  GThread *mt_safe_threads[ N_THREADS ];
  GThread *mt_unsafe_threads[ N_THREADS ];

  guint i;

  #if !GLIB_CHECK_VERSION( 2, 36, 0 )
    g_type_init();
  #endif

  grpc_manager = gp_rpc_manager_new ();

  gp_rpc_manager_reg_proc (grpc_manager, 1, mt_safe_proc, TRUE);
  gp_rpc_manager_reg_proc (grpc_manager, 2, mt_unsafe_proc, FALSE);

  gp_rpc_manager_reg_obj (grpc_manager, 1, (gpointer) &mt_safe_proc_counter, TRUE);
  gp_rpc_manager_reg_obj (grpc_manager, 2, (gpointer) &mt_unsafe_proc_counter, FALSE);

  pay_load_mt_safe.grpc_manager = grpc_manager;
  pay_load_mt_safe.proc_id = 1;
  pay_load_mt_safe.obj_id = 2;

  pay_load_mt_unsafe.grpc_manager = grpc_manager;
  pay_load_mt_unsafe.proc_id = 2;
  pay_load_mt_unsafe.obj_id = 1;

  for( i = 0; i < N_THREADS; i++ )
    {
    mt_safe_threads[i] = g_thread_new( "mt-safe", worker, &pay_load_mt_safe );
    mt_unsafe_threads[i] = g_thread_new( "mt-unsafe", worker, &pay_load_mt_unsafe );
    }

  g_usleep( 1000000 );

  g_atomic_int_set( &start, 1 );

  for( i = 0; i < N_THREADS; i++ )
    {
    g_thread_join( mt_safe_threads[i] );
    g_thread_join( mt_unsafe_threads[i] );
    }

  g_object_unref( grpc_manager );

  g_message( "mt safe counter %d", mt_safe_proc_counter );
  g_message( "mt unsafe counter %d", mt_unsafe_proc_counter );
  g_message( "mt unsafe proc counter %d", mt_unsafe_in_proc_counter );

  return 0;

}
