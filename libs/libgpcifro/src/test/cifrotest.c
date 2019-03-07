#include <math.h>
#include <gtk/gtk.h>

#include "gp-cifroarea.h"
#include "gp-icastaterenderer.h"
#include "dummyrenderer.h"

#define CHANNELS_NUM  3
#define MAX_N_POINTS  (2*1000)

static gboolean quit = FALSE;

gdouble diff_time;
GTimeVal start_time, cur_time;

void destroy_callback( void )
{

  quit = TRUE;
  gtk_main_quit();

}


gboolean update_data( gpointer user_data )
{

  int i,k;
  static int j = 0;
  static gfloat *d[ CHANNELS_NUM ];

  if( quit ) return FALSE;

  g_get_current_time( &cur_time );
//  diff_time = (gfloat)(cur_time.tv_usec - start_time.tv_usec + ( 1000000 * ( cur_time.tv_sec - start_time.tv_sec ) ) ) / 1000000.0;
//  printf( "idle time: %.6lfs, freq: %.1fHz\n", diff_time, 1.0 / diff_time );
//  fflush( stdout );

  g_get_current_time( &start_time );


  for( i = 0; i < CHANNELS_NUM; i++ )
    if( !d[i] ) d[i] = g_new0( gfloat, MAX_N_POINTS );

  for( k = 0; k < CHANNELS_NUM; k++ )
    {

    for( i = 0; i < MAX_N_POINTS; i++ )
      {
      d[k][i] = 1.0 * sin( ( k + 1 ) * ( i + j ) * G_PI / 100.0 + ( ( G_PI / CHANNELS_NUM ) * k ) );
//      d[k][i] = (gdouble)MAX_N_POINTS / ( (gdouble)(i + MAX_N_POINTS) ) * ( (i%2) ? -1.0 : 1.0 );
      }

    if( k == 0 )
      {
      d[0][j] = 500.0;
      j++;
      if( j >= MAX_N_POINTS ) j = 0;
      }

//    gp_cifro_scope_set_data( GP_CIFRO_SCOPE( cscope ), k, MAX_N_POINTS, d[k] );

    }

//  gp_cifro_scope_update( GP_CIFRO_SCOPE( cscope ) );

  return TRUE;

}


gboolean update_angle( gpointer user_data )
{

  //GpCifroArea *carea = user_data;
  //static gfloat angle = 0.0;

//  gp_cifro_area_set_angle( carea, angle );
//  angle += 0.01;

  return TRUE;

}


void save_to_png(GpCifroArea *carea)
{
  cairo_surface_t *surface = gp_cifro_area_draw_to_surface(carea);

  cairo_status_t status = cairo_surface_write_to_png(surface, "cifrotest_export.png");

  if(status == CAIRO_STATUS_SUCCESS)
    g_message("File cifrotest_export.png has been successfully written.");
  else
    g_warning("Failed to save png: %s", cairo_status_to_string(status));

  cairo_surface_destroy(surface);
}


int main( int argc, char **argv )
{

  GtkWidget *window;
  GtkWidget *container;

  GtkWidget *cscope = NULL;

  GpCifroArea *carea;
  GpIcaStateRenderer *state_renderer;

  guint i;
  gfloat *in;

  //~ gdouble scales_x[3] = { 1.0, 0.5, 0.1 };
  //~ gdouble scales_y[3] = { 1.0, 0.5, 0.1 };

  gtk_init( &argc, &argv );
  g_random_set_seed( 0 );

  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

//  cscope = gp_cifro_scope_new( GP_CIFRO_SCOPE_GRAVITY_DOWN_LEFT, CHANNELS_NUM );
//  gp_cifro_scope_set_view( GP_CIFRO_SCOPE( cscope ), FALSE, "мс", -25.0, 25.0, "В", -1.1, 1.1 );

  carea = GP_CIFRO_AREA(gp_cifro_area_new (20) );

//  renderer = g_object_new( DUMMY_RENDERER_TYPE, NULL );
  state_renderer = gp_ica_state_renderer_new() ;
  gp_cifro_area_add_layer (carea, GP_ICA_RENDERER(state_renderer));
  gp_cifro_area_add_layer (carea, GP_ICA_RENDERER(dummy_renderer_new (state_renderer)));
  gp_cifro_area_set_swap (carea, FALSE, FALSE);
  gp_cifro_area_set_border (carea, 5, 5, 20, 20);
  gp_cifro_area_set_border (carea, 25, 25, 25, 25);
//  gp_cifro_area_set_shown_limits( carea, 0, 150, 0, 150 );
  gp_cifro_area_set_shown_limits (carea, -10000, 10000, -10000, 10000);
  gp_cifro_area_set_scale_limits (carea, 0.0001, 1000.0, 0.0001, 1000.0);
//  gp_cifro_area_set_scale_limits( carea, 0.2, 2.0, 1.0, 1.0 );
  gp_cifro_area_set_minimum_visible (carea, 64, 64);
  gp_cifro_area_set_scale_on_resize (carea, TRUE);
//  gp_cifro_area_set_angle( carea, G_PI / 31.4 );
  gp_cifro_area_set_rotation (carea, TRUE);
  gp_cifro_area_set_scale_aspect (carea, -1.0);
  gp_cifro_area_set_shown (carea, -100, 100, -100, 100);
//  gp_cifro_area_set_shown( carea, 210, 220, 230, 240 );
  gp_cifro_area_set_zoom_on_center (carea, FALSE);
  gp_cifro_area_set_zoom_scale (carea, 10.0);
  //~ gp_cifro_area_set_fixed_zoom_scales (carea, scales_x, scales_y, 3);
  gp_cifro_area_set_move_multiplier (carea, 25);
  gp_cifro_area_set_rotate_multiplier (carea, 0.1);

  gp_cifro_area_set_point_cursor (carea, NULL);
//  gp_cifro_area_set_move_cursor( carea, NULL );

  in = g_malloc( MAX_N_POINTS * sizeof( gfloat ) );

  for( i = 0; i < MAX_N_POINTS; i++ )
    in[i] = g_random_double_range( -1.0, 1.0 );

//  gp_cifro_scope_set_data( GP_CIFRO_SCOPE( cscope ), 0, MAX_N_POINTS, in );

//  gtk_box_pack_start( GTK_BOX( container ), cscope, TRUE, TRUE, 5 );
  gtk_box_pack_start( GTK_BOX( container ), GTK_WIDGET( carea ), TRUE, TRUE, 0 );

  gtk_container_add( GTK_CONTAINER( window ), container );

  { // Кнопки в нижней части окна -->
    GtkWidget *button;

    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(container), GTK_WIDGET(hbox), FALSE, FALSE, 0);

    button = gtk_button_new_from_icon_name("list-remove", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_end(GTK_BOX(hbox), GTK_WIDGET(button), FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy), carea);

    button = gtk_button_new_from_icon_name("document-save", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_end(GTK_BOX(hbox), GTK_WIDGET(button), FALSE, FALSE, 0);
    g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(save_to_png), carea);
  } // Кнопки в нижней части окна -->

  g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( destroy_callback ), NULL );

  gtk_window_set_default_size( GTK_WINDOW( window ), 300, 200 );
  gtk_widget_show_all( window );

  g_timeout_add( 100, update_data, cscope );
//  g_idle_add( update_data, cscope );

  g_timeout_add( 100, update_angle, carea );

  gtk_main();

  g_free( in );

  return 0;

}
