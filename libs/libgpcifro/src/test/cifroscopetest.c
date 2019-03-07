#include <math.h>
#include <gtk/gtk.h>

#include "gp-cifroscope.h"


#define CHANNELS_NUM  4
#define MAX_N_POINTS  (2*1000)

static gboolean quit = FALSE;

GpCifroScope *ccurve;


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

  for( i = 0; i < CHANNELS_NUM; i++ )
    if( d[i] == NULL ) d[i] = g_new0( gfloat, MAX_N_POINTS );

  for( k = 0; k < CHANNELS_NUM - 1; k++ )
    {

    for( i = 0; i < MAX_N_POINTS; i++ )
      d[k][i] = sin( ( k + 1 ) * ( i + j ) * G_PI / 100.0 + ( ( G_PI / CHANNELS_NUM ) * k ) );

    if( k == 1 )
      for( i = MAX_N_POINTS / 3; i < 2 * MAX_N_POINTS / 3; i++ )
        if( i % 20 > 4 )
          d[1][i] = NAN;

    if( k == 0 )
      {
      d[0][j] = 10.0;
      j++;
      if( j >= MAX_N_POINTS ) j = 0;
      }

      gp_cifro_scope_set_data (GP_CIFRO_SCOPE(ccurve), k, MAX_N_POINTS, d[k]);

    }

  gp_cifro_scope_update (ccurve);

  return TRUE;

}


int main( int argc, char **argv )
{

  gboolean auto_color = FALSE;
  gboolean dark_theme = FALSE;
  gboolean dotted = FALSE;

  GtkWidget *window;

  guint i;
  gfloat *in;

  gtk_init( &argc, &argv );
  g_random_set_seed( 0 );

  { // Разбор командной строки.

  GError          *error = NULL;
  GOptionContext  *context;
  GOptionEntry     entries[] =
  {
    { "auto-color", 'a', 0, G_OPTION_ARG_NONE, &auto_color, "Use theme colors", NULL },
    { "dark-theme", 'b', 0, G_OPTION_ARG_NONE, &dark_theme, "Use dark theme", NULL },
    { "dotted",     'd', 0, G_OPTION_ARG_NONE, &dotted, "Draw osciloscope data with dots", NULL },
    { NULL }
  };

  context = g_option_context_new( "" );
  g_option_context_set_help_enabled( context, TRUE );
  g_option_context_add_main_entries( context, entries, NULL );
  g_option_context_set_ignore_unknown_options( context, FALSE );
  if( !g_option_context_parse( context, &argc, &argv, &error ) )
    { g_message( error->message ); return -1; }

  g_option_context_free( context );

  }

  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );

  ccurve = GP_CIFRO_SCOPE(gp_cifro_scope_new (GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP, CHANNELS_NUM) );
  gp_cifro_scope_set_view (GP_CIFRO_SCOPE(ccurve), "мс", -50.0, 1050.0, "В", -1.1, 1.1);
  gp_cifro_scope_set_zoom_scale (GP_CIFRO_SCOPE(ccurve), 0.1, 10.0, 0.001, 1000.0);

  gp_cifro_scope_set_channel_color (GP_CIFRO_SCOPE(ccurve), 0, 1.0, 0.0, 0.0);
  gp_cifro_scope_set_channel_color (GP_CIFRO_SCOPE(ccurve), 1, 0.0, 1.0, 0.0);
  gp_cifro_scope_set_channel_color (GP_CIFRO_SCOPE(ccurve), 2, 0.0, 0.0, 1.0);
  gp_cifro_scope_set_channel_color (GP_CIFRO_SCOPE(ccurve), 3, 1.0, 1.0, 1.0);

  gp_cifro_scope_set_channel_time_param (GP_CIFRO_SCOPE(ccurve), -1, 0.0, 0.5);

  gp_cifro_scope_set_channel_value_param (GP_CIFRO_SCOPE(ccurve), 0, 0.0, 0.99);
  gp_cifro_scope_set_channel_value_param (GP_CIFRO_SCOPE(ccurve), 1, 0.0, 0.66);
  gp_cifro_scope_set_channel_value_param (GP_CIFRO_SCOPE(ccurve), 2, 0.0, 0.33);

  gp_cifro_scope_set_channel_value_param (GP_CIFRO_SCOPE(ccurve), 3, 0.0, 0.1);
  gp_cifro_scope_set_channel_axis_name (GP_CIFRO_SCOPE(ccurve), 3, "NOISE");

  gp_cifro_scope_set_shown (GP_CIFRO_SCOPE(ccurve), 0.0, 1000.0, -1.1, 1.1);

  if(dark_theme)
    g_object_set( gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);

  if( dotted )
    gp_cifro_scope_set_channel_draw_type (GP_CIFRO_SCOPE(ccurve), -1, GP_CIFRO_SCOPE_DOTTED);

  if(!auto_color)
  {
    GValue param = G_VALUE_INIT;
    g_value_init( &param, G_TYPE_UINT );
    g_value_set_uint( &param, 0xFF00FF00 );
    g_object_set_property( G_OBJECT( ccurve ), "text-color", &param );
  }

  in = g_malloc( MAX_N_POINTS * sizeof( gfloat ) );

  for( i = 0; i < MAX_N_POINTS; i++ )
    in[i] = g_random_double_range( -1.0, 1.0 );

  gp_cifro_scope_set_data (ccurve, CHANNELS_NUM - 1, MAX_N_POINTS, in);

  gtk_container_add( GTK_CONTAINER( window ), GTK_WIDGET( ccurve ) );
  g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( destroy_callback ), NULL );
  gtk_window_set_default_size( GTK_WINDOW( window ), 300, 200 );
  gtk_widget_show_all( window );

  g_timeout_add( 40, update_data, ccurve );

  gtk_main();
  g_free( in );

  return 0;

}
