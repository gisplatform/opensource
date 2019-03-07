#include <math.h>
#include <gtk/gtk.h>

#include "gp-cifrocurve.h"
#include "gp-cifrocustom.h"


#define CHANNELS_NUM  4
#define MAX_N_POINTS  (2*1000)

static gboolean quit = FALSE;

GpCifroCurve *ccurve;
GpCifroCustom *cpoint;


typedef struct spline_tuple
{

  gdouble a, b, c, d, x;

} spline_tuple;


gdouble curve_func_akima( gdouble param, GArray *points, gpointer curve_data )
{

   guint i, j, k, n;

   GpIcaCurvePoint *point;
   GpIcaCurvePoint *point2;

   spline_tuple *spline;
   spline_tuple *splines;
   gfloat *alpha, *beta, dx, dy, y;

   if (points->len < 2)
      return param;

   if (points->len == 2)
   {
      point = &g_array_index( points, GpIcaCurvePoint, 0 );
      point2 = &g_array_index( points, GpIcaCurvePoint, 1 );
      dy = ( point2->y - point->y ) / ( point2->x - point->x );
      y = param * dy + ( point->y - point->x * dy );
      return y;
   }

   n = points->len;
   splines = g_malloc0( n * sizeof(spline_tuple) );
   alpha = g_malloc0( ( n - 1 ) * sizeof(gfloat) );
   beta = g_malloc0( ( n - 1 ) * sizeof(gfloat) );

   for ( i = 0; i < n; i++ )
   {
      point = &g_array_index( points, GpIcaCurvePoint, i );
      splines[i].x = point->x;
      splines[i].a = point->y;
   }
   splines[0].c = splines[n - 1].c = 0.0;

   alpha[0] = beta[0] = 0.;
   for ( i = 1; i < n - 1; i++ )
   {
      gfloat h_i = splines[i].x - splines[i - 1].x;
      gfloat h_i1 = splines[i + 1].x - splines[i].x;
      gfloat A = h_i;
      gfloat B = h_i1;
      gfloat C = 2.0 * ( h_i + h_i1 );
      gfloat F = 6.0 * ( ( splines[i + 1].a - splines[i].a ) / h_i1 - ( splines[i].a - splines[i - 1].a ) / h_i );
      gfloat z = A * alpha[i - 1] + C;
      alpha[i] = -B / z;
      beta[i] = ( F - A * beta[i - 1] ) / z;
   }

   for ( i = n - 2; i > 0; i-- )
      splines[i].c = alpha[i] * splines[i + 1].c + beta[i];

   for ( i = n - 1; i > 0; i-- )
   {
      gfloat h_i = splines[i].x - splines[i - 1].x;
      splines[i].d = ( splines[i].c - splines[i - 1].c ) / h_i;
      splines[i].b = h_i * ( 2.0 * splines[i].c + splines[i - 1].c ) / 6.0 + ( splines[i].a - splines[i - 1].a ) / h_i;
   }

   if (param <= splines[0].x)
      spline = splines + 1;
   else
      if (param >= splines[n - 1].x)
         spline = splines + n - 1;
      else
      {
         i = 0;
         j = n - 1;
         while ( i + 1 < j )
         {
            k = i + ( j - i ) / 2;
            if (param <= splines[k].x)
               j = k;
            else
               i = k;
         }
         spline = splines + j;
      }

   dx = ( param - spline->x );
   y = spline->a + ( spline->b + ( spline->c / 2.0 + spline->d * dx / 6.0 ) * dx ) * dx;

   g_free( splines );
   g_free( alpha );
   g_free( beta );

   return y;

}

gdouble curve_func( gdouble param, GArray *points, gpointer curve_data )
{

  return sin( param / 10 );

}


void destroy_callback( GtkWidget *widget )
{

  GArray *points = gp_cifro_curve_get_points (ccurve);

  GpIcaCurvePoint *point;
  guint i;

  g_message( "Control points: %d", points->len );
  for( i = 0; i < points->len; i++ )
    {
    point = &g_array_index( points, GpIcaCurvePoint, i );
    g_message( "%3d: %3.6lf %3.6lf", i, point->x, point->y );
    }

  g_array_unref( points );

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
    if( !d[i] ) d[i] = g_new0( gfloat, MAX_N_POINTS );

  for( k = 0; k < CHANNELS_NUM - 1; k++ )
    {

    for( i = 0; i < MAX_N_POINTS; i++ )
      d[k][i] = sin( ( k + 1 ) * ( i + j ) * G_PI / 100.0 + ( ( G_PI / CHANNELS_NUM ) * k ) );

    if( k == 0 )
      {
      d[0][j] = 10.0;
      j++;
      if( j >= MAX_N_POINTS ) j = 0;
      }

      gp_cifro_scope_set_data (GP_CIFRO_SCOPE(ccurve), k, MAX_N_POINTS, d[k]);

    }

  gp_cifro_scope_update(GP_CIFRO_SCOPE(ccurve));

  return TRUE;

}

void button_clicked(GtkButton *button, gpointer data)
{
  GpCifroCustom *custom = GP_CIFRO_CUSTOM(data);
  GpIcaCustomDrawType new_type = gp_cifro_custom_get_draw_type(custom);
  new_type++;

  if(new_type >= GP_ICA_CUSTOM_DRAW_TYPE_AMOUNT)
    new_type = GP_ICA_CUSTOM_DRAW_TYPE_POINT;

  printf("Set  type  = %d\n", new_type);
  gp_cifro_custom_set_draw_type(custom, new_type);
}

void button_clicked_clear(GtkButton *button, gpointer data)
{
  GpCifroCustom *custom = GP_CIFRO_CUSTOM(data);

  gp_cifro_custom_clear_points(custom);
}

int main( int argc, char **argv )
{

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
    { "dotted", 'd', 0, G_OPTION_ARG_NONE, &dotted, "Draw osciloscope data with dots", NULL },
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

  ccurve = GP_CIFRO_CURVE(gp_cifro_curve_new (GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP, 4, curve_func_akima, NULL) );
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

  gp_cifro_scope_set_shown (GP_CIFRO_SCOPE(ccurve), 0.0, 1000.0, -1.1, 1.1);

  if( dotted )
    gp_cifro_scope_set_channel_draw_type (GP_CIFRO_SCOPE(ccurve), -1, GP_CIFRO_SCOPE_DOTTED);

  gp_cifro_curve_set_curve_color (ccurve, 0.7, 0.7, 1.0);
  gp_cifro_curve_set_point_color (ccurve, 0.3, 0.3, 1.0);

  in = g_malloc( MAX_N_POINTS * sizeof( gfloat ) );

  for( i = 0; i < MAX_N_POINTS; i++ )
    in[i] = g_random_double_range( -1.0, 1.0 );

  gp_cifro_scope_set_data(GP_CIFRO_SCOPE(ccurve), CHANNELS_NUM - 1, MAX_N_POINTS, in);

  GtkWidget *vbox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add( GTK_CONTAINER( window ), GTK_WIDGET( vbox ) );
  gtk_box_pack_start( GTK_BOX( vbox ), GTK_WIDGET( ccurve ), TRUE, TRUE, 0 );

  cpoint = GP_CIFRO_CUSTOM(gp_cifro_custom_new (GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP, 1, NULL));
  gtk_box_pack_start( GTK_BOX( vbox ), GTK_WIDGET( cpoint ), TRUE, TRUE, 0 );

  gp_cifro_custom_set_line_color (cpoint, 0.7, 0.7, 1.0);
  gp_cifro_custom_set_points_color (cpoint, 0.3, 0.3, 1.0);

  GtkWidget *button = gtk_button_new_with_label("Custom type");
  gtk_box_pack_start( GTK_BOX( vbox ), GTK_WIDGET( button ), FALSE, TRUE, 0 );
  g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( button_clicked ), cpoint );

  button = gtk_button_new_with_label("Clear point list");
  gtk_box_pack_start( GTK_BOX( vbox ), GTK_WIDGET( button ), FALSE, TRUE, 0 );
  g_signal_connect( G_OBJECT( button ), "clicked", G_CALLBACK( button_clicked_clear ), cpoint );
  //~ gtk_container_add( GTK_CONTAINER( window ), GTK_WIDGET( ccurve ) );
  // TODO: Fix next line!
  //g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK( destroy_callback ), NULL );
  gtk_window_set_default_size( GTK_WINDOW( window ), 300, 200 );
  gtk_widget_show_all( window );

  g_timeout_add( 40, update_data, ccurve );

  gtk_main();
  g_free( in );

  return 0;

}
