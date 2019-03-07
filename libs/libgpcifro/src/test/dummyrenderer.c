#include "dummyrenderer.h"

#include <string.h>
#include <stdio.h>
#include <cairo.h>
#include <math.h>


typedef struct DummyRendererPriv {

  GpIcaStateRenderer *state_renderer;            // Псевдо отрисовщик для учёта состояния областей.
  const GpIcaState *state;                     // Состояние областей отрисовки.

  cairo_t *axis_cairo;
  cairo_t *axis_part_cairo;
  cairo_t *border_cairo;
  cairo_t *progress_cairo;

  gboolean axis_need_update;
  gboolean axis_part_need_update;
  gboolean border_need_update;

  gint     completion;
  gint64   start_rendering_time;
  gint64   last_completed_time;

} DummyRendererPriv;

#define DUMMY_RENDERER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_DUMMY_RENDERER, DummyRendererPriv ) )


static void dummy_renderer_interface_init( GpIcaRendererInterface *iface );
static void dummy_renderer_set_property( DummyRenderer *drenderer, guint prop_id, const GValue *value, GParamSpec *pspec );
static GObject* dummy_renderer_constructor( GType type, guint n_construct_properties, GObjectConstructParam *construct_properties );
static void dummy_renderer_finalize( DummyRenderer *drenderer );

#if !GLIB_CHECK_VERSION( 2, 28, 0 )
static gint64 g_get_monotonic_time( void );
#endif


enum { PROP_O, PROP_STATE_RENDERER };

G_DEFINE_TYPE_WITH_CODE( DummyRenderer, dummy_renderer, G_TYPE_INITIALLY_UNOWNED,
                         G_IMPLEMENT_INTERFACE(G_TYPE_GP_ICA_RENDERER, dummy_renderer_interface_init ) );

static void dummy_renderer_init( DummyRenderer *drenderer ){ ; }

static void dummy_renderer_class_init( DummyRendererClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  g_type_class_add_private( klass, sizeof( DummyRendererPriv ) );

  this_class->constructor = (void*)dummy_renderer_constructor;
  this_class->set_property = (void*)dummy_renderer_set_property;
  this_class->finalize = (void*)dummy_renderer_finalize;

  g_object_class_install_property( this_class, PROP_STATE_RENDERER,
    g_param_spec_object( "state-renderer", "State renderer", "State Renderer", G_TYPE_GP_ICA_STATE_RENDERER, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

}


// Функция установки параметров.
static void dummy_renderer_set_property( DummyRenderer *drenderer, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );

  switch ( prop_id )
    {

    case PROP_STATE_RENDERER:
      priv->state_renderer = g_value_get_object( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( drenderer, prop_id, pspec );
      break;

    }

}


// Конструктор объекта.
static GObject* dummy_renderer_constructor( GType g_type, guint n_construct_properties, GObjectConstructParam *construct_properties )
{

  GObject *drenderer = G_OBJECT_CLASS( dummy_renderer_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( drenderer == NULL ) return NULL;

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );

  priv->state = gp_ica_state_renderer_get_state( priv->state_renderer );

  priv->axis_cairo = NULL;
  priv->axis_part_cairo = NULL;
  priv->border_cairo = NULL;
  priv->progress_cairo = NULL;

  priv->axis_need_update = TRUE;
  priv->axis_part_need_update = TRUE;
  priv->border_need_update = TRUE;

  priv->completion = 0;
  priv->start_rendering_time = g_get_monotonic_time();
  priv->last_completed_time = g_get_monotonic_time();

  if( priv->state_renderer == NULL )
    {
    g_object_unref( drenderer );
    return NULL;
    }

  return drenderer;

}


// Освобождение памяти занятой объектом.
static void dummy_renderer_finalize( DummyRenderer *drenderer )
{

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );

  if( priv->axis_cairo ) cairo_destroy( priv->axis_cairo );
  if( priv->axis_part_cairo ) cairo_destroy( priv->axis_part_cairo );
  if( priv->border_cairo ) cairo_destroy( priv->border_cairo );
  if( priv->progress_cairo ) cairo_destroy( priv->progress_cairo );

  G_OBJECT_CLASS( dummy_renderer_parent_class )->finalize( G_OBJECT( drenderer ) );

}


static gint dummy_renderer_get_renderers_num( GpIcaRenderer *drenderer )
{

  return DUMMY_RENDERER_RENDERERS_NUM;

}


static const gchar *dummy_renderer_get_name( GpIcaRenderer *drenderer, gint renderer_id )
{

  switch( renderer_id )
    {
    case DUMMY_RENDERER_AXIS: return "Axis renderer.";
    case DUMMY_RENDERER_AXIS_PART: return "Axis part renderer.";
    case DUMMY_RENDERER_BORDER: return "Border renderer.";
    case DUMMY_RENDERER_PROGRESS: return "Progress renderer.";
    }

  return "Unknown renderer.";

}


static GpIcaRendererType dummy_renderer_get_renderer_type( GpIcaRenderer *drenderer, gint renderer_id )
{

  switch( renderer_id )
    {
    case DUMMY_RENDERER_AXIS: return GP_ICA_RENDERER_VISIBLE;
    case DUMMY_RENDERER_AXIS_PART: return GP_ICA_RENDERER_VISIBLE;
    case DUMMY_RENDERER_BORDER: return GP_ICA_RENDERER_AREA;
    case DUMMY_RENDERER_PROGRESS: return GP_ICA_RENDERER_AREA;
    }

  return GP_ICA_RENDERER_UNKNOWN;

}


static void dummy_renderer_set_surface( GpIcaRenderer *drenderer, gint renderer_id, guchar *data, gint width, gint height, gint stride )
{

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );
  cairo_surface_t *surface;

  switch( renderer_id )
    {

    case DUMMY_RENDERER_AXIS:
      if( priv->axis_cairo ) cairo_destroy( priv->axis_cairo );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->axis_cairo = cairo_create( surface );
      cairo_surface_destroy( surface );
      break;

    case DUMMY_RENDERER_AXIS_PART:
      if( priv->axis_part_cairo ) cairo_destroy( priv->axis_part_cairo );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->axis_part_cairo = cairo_create( surface );
      cairo_surface_destroy( surface );
      break;

    case DUMMY_RENDERER_BORDER:
      if( priv->border_cairo ) cairo_destroy( priv->border_cairo );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->border_cairo = cairo_create( surface );
      cairo_surface_destroy( surface );
      break;

    case DUMMY_RENDERER_PROGRESS:
      if( priv->progress_cairo ) cairo_destroy( priv->progress_cairo );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->progress_cairo = cairo_create( surface );
      cairo_surface_destroy( surface );
      break;

    }

}


static gint dummy_renderer_get_completion( GpIcaRenderer *drenderer, gint renderer_id )
{

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );

  gint completed;
  gint64 cur_rendering_time;

  if( renderer_id != DUMMY_RENDERER_PROGRESS ) return -1;

  cur_rendering_time = g_get_monotonic_time();

  if( cur_rendering_time - priv->start_rendering_time > 6000000 )
    {
    priv->start_rendering_time = cur_rendering_time;
    return 0;
    }

  completed = ( cur_rendering_time - priv->start_rendering_time ) / 5900;
  if( completed > 1000 ) completed = 1000;
  return completed;

}


static GpIcaRendererAvailability dummy_renderer_render( GpIcaRenderer *drenderer, gint renderer_id, gint *x, gint *y, gint *width, gint *height )
{

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );
  cairo_t *cairo = NULL;

  if( priv->state->area_width < 120 || priv->state->area_height < 100 ) return GP_ICA_RENDERER_AVAIL_NONE;
  if( ( renderer_id == DUMMY_RENDERER_AXIS ) && ( !priv->axis_need_update ) ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
  if( ( renderer_id == DUMMY_RENDERER_AXIS_PART ) && ( !priv->axis_part_need_update ) ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
  if( ( renderer_id == DUMMY_RENDERER_BORDER ) && ( !priv->border_need_update ) ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;

  if( renderer_id == DUMMY_RENDERER_PROGRESS )
    {
    if( priv->state->completion == 1000 )
      {
      priv->last_completed_time = g_get_monotonic_time();
      return GP_ICA_RENDERER_AVAIL_NONE;
      }
    if( priv->last_completed_time + 2000000 > g_get_monotonic_time() )
      return GP_ICA_RENDERER_AVAIL_NONE;
    }

  switch( renderer_id )
    {
    case DUMMY_RENDERER_AXIS:
      cairo = priv->axis_cairo;
      break;
    case DUMMY_RENDERER_BORDER:
      cairo = priv->border_cairo;
      break;
    case DUMMY_RENDERER_AXIS_PART:
      cairo = priv->axis_part_cairo;
      break;
    case DUMMY_RENDERER_PROGRESS:
      cairo = priv->progress_cairo;
      break;
    }

  if( cairo == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;

  cairo_set_operator( cairo, CAIRO_OPERATOR_SOURCE );
  cairo_set_source_rgba( cairo, 0.0, 0.0, 0.0, 0.0 );
  cairo_paint( cairo );

  cairo_set_operator( cairo, CAIRO_OPERATOR_OVER );
  cairo_set_line_width( cairo, 1.0 );

  switch( renderer_id )
    {

    case DUMMY_RENDERER_AXIS:
      {

      gint range;
      gint grid_width = 100;
      gdouble cur_val, from_val, step_val;
      gdouble xs, ys;

      cairo_set_source_rgba( cairo, 0.11, 0.11, 0.33, 0.9 );

      // Оцифровка по X.
      from_val = priv->state->from_x;
        gp_ica_state_get_axis_step( priv->state->cur_scale_x, grid_width, &from_val, &step_val, &range, NULL);
      for( cur_val = from_val; cur_val < priv->state->to_x; cur_val += step_val )
        {

          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, cur_val, priv->state->from_y );
        cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, cur_val, priv->state->to_y );
        cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        cairo_stroke( cairo );

        }

      // Оцифровка по Y.
      from_val = priv->state->from_y;
        gp_ica_state_get_axis_step( priv->state->cur_scale_y, grid_width, &from_val, &step_val, &range, NULL);
      for( cur_val = from_val; cur_val < priv->state->to_y; cur_val += step_val )
        {

          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, priv->state->from_x, cur_val );
        cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, priv->state->to_x, cur_val );
        cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        cairo_stroke( cairo );

        }

      cairo_set_source_rgba( cairo, 0.11, 0.11, 0.88, 0.9 );

      // Нулевая ось X.
        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, priv->state->from_x, 0 );
      cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, priv->state->to_x, 0 );
      cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
      cairo_stroke( cairo );

      // Нулевая ось Y.
        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, 0, priv->state->from_y );
      cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, 0, priv->state->to_y );
      cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
      cairo_stroke( cairo );

      // Засечки по X.
      from_val = priv->state->from_x;
        gp_ica_state_get_axis_step( priv->state->cur_scale_x, grid_width, &from_val, &step_val, &range, NULL);
      for( cur_val = from_val; cur_val < priv->state->to_x; cur_val += step_val )
        {

          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, cur_val, -step_val / 10 );
        cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, cur_val, step_val / 10 );
        cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        cairo_stroke( cairo );

        }

      // Засечки по Y.
      from_val = priv->state->from_y;
        gp_ica_state_get_axis_step( priv->state->cur_scale_y, grid_width, &from_val, &step_val, &range, NULL);
      for( cur_val = from_val; cur_val < priv->state->to_y; cur_val += step_val )
        {

          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, -step_val / 10, cur_val );
        cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, step_val / 10, cur_val );
        cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        cairo_stroke( cairo );

        }

      if( x ) *x = 0;
      if( y ) *y = 0;
      if( width ) *width = priv->state->visible_width;
      if( height ) *height = priv->state->visible_height;

      priv->axis_need_update = FALSE;

      break;

      }

    case DUMMY_RENDERER_AXIS_PART:
      {

      gint i;
      gdouble xs, ys;
      gdouble xp[16] = { 10, 20, 20, 10,  10,  20,  20,  10, -10, -20, -20, -10, -10, -20, -20, -10 };
      gdouble yp[16] = { 10, 10, 20, 20, -10, -10, -20, -20,  10,  10,  20,  20, -10, -10, -20, -20 };

      cairo_set_source_rgba( cairo, 0.11, 0.44, 0.11, 0.9 );

      for( i = 0; i < 16; i++ )
        {

          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, xp[i], yp[i] );
        cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

        if( ( ( i + 1 ) % 4 ) == 0 )
          {
          cairo_close_path( cairo );
          cairo_stroke( cairo );
          }

        }

      cairo_close_path( cairo );
      cairo_stroke( cairo );

      cairo_set_source_rgba( cairo, 0.11, 0.88, 0.11, 0.9 );

        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, -10, 0 );
      cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, 10, 0 );
      cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
      cairo_stroke( cairo );
        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, 0, -10 );
      cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, 0, 10 );
      cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
      cairo_stroke( cairo );

        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, -22, -22 );
      if( x ) *x = xs;
      if( y ) *y = ys;
        gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &xs, &ys, 22, 22 );
      if( width ) *width = xs - *x;
      if( height ) *height = ys - *y;


      priv->axis_part_need_update = FALSE;

      break;

      }

    case DUMMY_RENDERER_BORDER:
      {

      gdouble left, top;
      gdouble bwidth, bheight;

      left = priv->state->border_left - 0.5;
      top = priv->state->border_top - 0.5;
      bwidth = priv->state->area_width - priv->state->border_left - priv->state->border_right + 1;
      bheight = priv->state->area_height - priv->state->border_top - priv->state->border_bottom + 1;

      cairo_set_source_rgb( cairo, 0.11, 0.88, 0.11 );
      cairo_rectangle( cairo, left, top, bwidth, bheight );
      cairo_stroke( cairo );

      if( x ) *x = 0;
      if( y ) *y = 0;
      if( width ) *width = priv->state->area_width;
      if( height ) *height = priv->state->area_height;

      priv->border_need_update = FALSE;

      break;

      }

    case DUMMY_RENDERER_PROGRESS:
      {

      gdouble left, top;
      gdouble bwidth, bheight;

      left = priv->state->area_width - priv->state->border_left - priv->state->border_right - 110.0 + 0.5;
      top = priv->state->area_height - priv->state->border_top - priv->state->border_bottom - 20.0 + 0.5;
      bwidth = 100;
      bheight = 15;

      if( x ) *x = left - 0.5;
      if( y ) *y = top - 0.5;
      if( width ) *width = bwidth + 1;
      if( height ) *height = bheight + 1;

      cairo_set_operator( cairo, CAIRO_OPERATOR_OVER );
      cairo_set_source_rgba( cairo, 0.66, 0.66, 0.66, 0.5 );
      cairo_rectangle( cairo, left, top, bwidth, bheight );
      cairo_stroke( cairo );

      cairo_set_source_rgba( cairo, 0.5, 0.5, 0.5, 0.5 );
      cairo_rectangle( cairo, left, top, bwidth * ( priv->state->completion / 1000.0 ), bheight );
      cairo_fill( cairo );

      break;

      }

    }

  return GP_ICA_RENDERER_AVAIL_ALL;

}


static void dummy_renderer_set_visible_size( GpIcaRenderer *drenderer, gint width, gint height )
{

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );

  priv->axis_need_update = TRUE;
  priv->axis_part_need_update = TRUE;

}


static void dummy_renderer_set_area_size( GpIcaRenderer *drenderer, gint width, gint height )
{

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );

  priv->border_need_update = TRUE;

}


static void dummy_renderer_set_border( GpIcaRenderer *drenderer, gint left, gint right, gint top, gint bottom )
{

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );

  priv->border_need_update = TRUE;

}


static void dummy_renderer_set_shown( GpIcaRenderer *drenderer, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y )
{

  DummyRendererPriv *priv = DUMMY_RENDERER_GET_PRIVATE( drenderer );

  priv->axis_need_update = TRUE;
  priv->axis_part_need_update = TRUE;

}


static void dummy_renderer_button_print_event_type( GdkEvent *event )
{

  switch( event->type )
  {
    case GDK_BUTTON_PRESS:
      printf("Button Press");
      break;

    case GDK_2BUTTON_PRESS:
      printf("Button Double-click");
      break;

    case GDK_3BUTTON_PRESS:
      printf("Button Triple-click");
      break;

    case GDK_BUTTON_RELEASE:
      printf("Button Release");
      break;

    default:
      g_assert_not_reached();
      break;
  }

}


static gboolean dummy_renderer_button_press_event( GpIcaRenderer *self, GdkEvent *event, GtkWidget *widget )
{

  printf("Button Press Event: ");
  dummy_renderer_button_print_event_type( event );
  printf("\n");

  return FALSE;

}


static gboolean dummy_renderer_button_release_event( GpIcaRenderer *self, GdkEvent *event, GtkWidget *widget )
{
  printf( "Button Release Event: " );
  dummy_renderer_button_print_event_type( event );
  printf( "\n" );

  return FALSE;
}


static gboolean dummy_renderer_key_press_event( GpIcaRenderer *self, GdkEvent *event, GtkWidget *widget )
{

  printf( "Key Press Event: key '%s'\n", gdk_keyval_name( ( (GdkEventKey*)event )->keyval ) );

  return FALSE;

}


DummyRenderer *dummy_renderer_new( GpIcaStateRenderer *state_renderer )
{

  return g_object_new( G_TYPE_DUMMY_RENDERER, "state-renderer", state_renderer, NULL );

}


#if !GLIB_CHECK_VERSION( 2, 28, 0 )
static gint64 g_get_monotonic_time( void )
{

  GTimeVal cur_time;
  gint64 monotonic_time;

  g_get_current_time( &cur_time );

  monotonic_time = (gint64)cur_time.tv_sec * 1000000 + (gint64)cur_time.tv_usec;

  return monotonic_time;

}
#endif


static void dummy_renderer_interface_init( GpIcaRendererInterface *iface )
{

  iface->get_renderers_num = dummy_renderer_get_renderers_num;
  iface->get_renderer_type = dummy_renderer_get_renderer_type;
  iface->get_name = dummy_renderer_get_name;

  iface->set_surface = dummy_renderer_set_surface;
  iface->set_area_size = dummy_renderer_set_area_size;
  iface->set_visible_size = dummy_renderer_set_visible_size;

  iface->get_completion = dummy_renderer_get_completion;
  iface->set_total_completion = NULL;
  iface->render = dummy_renderer_render;
  iface->set_shown_limits = NULL;

  iface->set_border = dummy_renderer_set_border;
  iface->set_swap = NULL;
  iface->set_angle = NULL;
  iface->set_shown = dummy_renderer_set_shown;
  iface->set_pointer = NULL;

  iface->button_press_event = dummy_renderer_button_press_event;
  iface->button_release_event = dummy_renderer_button_release_event;
  iface->key_press_event = dummy_renderer_key_press_event;
  iface->key_release_event = NULL;
  iface->motion_notify_event = NULL;

}
