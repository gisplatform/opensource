/*
 * CifroArea is a raster 2D graphical library.
 *
 * Copyright 2013 Andrei Fadeev
 *
 * This file is part of CifroArea.
 *
 * CifroArea is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * CifroArea is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with CifroArea. If not, see <http://www.gnu.org/licenses/>.
 *
*/

/*!
 * \file icacurve.c
 *
 * \author Andrei Fadeev
 * \date 2.06.2014
 * \brief Заголовочный файл класса отображения параметрической кривой.
 *
 * \defgroup GpIcaCurve GpIcaCurve - класс отображения параметрической кривой.
 *
 *
 *
*/

#include <math.h>

#include "gp-icacurve.h"
#include "cairosdline.h"


enum { PROP_O, PROP_STATE_RENDERER, PROP_CURVE_FUNC, PROP_CURVE_DATA };

enum { CURVE_SURFACE = 0, SURFACES_NUM };


typedef struct GpIcaCurvePriv {


  GpIcaStateRenderer *state_renderer;   // Псевдо отрисовщик для учёта состояния областей.

  GArray                *curve_points;     // Точки кривой.
  GpIcaCurveFunc curve_func;       // Функция расчёта значений кривой.
  gpointer               curve_data;       // Пользовательские данные для функции расчёта значений кривой.

  guint32                curve_color;      // Цвет кривой.
  guint32                point_color;      // Цвет точек.

  gint                   selected_point;   // Индекс текущей выбранной точки;
  gboolean               move_point;       // Перемещать или нет выбранную точку;
  gboolean               remove_point;     // Удалить выбранную точку.

  cairo_sdline_surface  *curve_surface;    // Поверхность для рисования кривой.
  gboolean               curve_update;     // Признак необходимости перерисовки кривой.

} GpIcaCurvePriv;

#define GP_ICA_CURVE_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_GP_ICA_CURVE, GpIcaCurvePriv ) )


static void gp_ica_curve_interface_init( GpIcaRendererInterface *iface );
static void gp_ica_curve_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec );
static GObject *gp_ica_curve_constructor( GType g_type, guint n_construct_properties,
                                         GObjectConstructParam *construct_properties );
static void gp_ica_curve_finalize(GObject *object);

G_DEFINE_TYPE_WITH_CODE(GpIcaCurve, gp_ica_curve, G_TYPE_INITIALLY_UNOWNED,
  G_IMPLEMENT_INTERFACE(G_TYPE_GP_ICA_RENDERER, gp_ica_curve_interface_init));

static void gp_ica_curve_init( GpIcaCurve *curve ) { ; }


static void gp_ica_curve_class_init( GpIcaCurveClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  g_type_class_add_private( klass, sizeof( GpIcaCurvePriv ) );

  this_class->set_property = gp_ica_curve_set_property;
  this_class->constructor = gp_ica_curve_constructor;
  this_class->finalize = gp_ica_curve_finalize;

  g_object_class_install_property( this_class, PROP_STATE_RENDERER,
    g_param_spec_object( "state-renderer", "State renderer", "State Renderer", G_TYPE_GP_ICA_STATE_RENDERER, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_CURVE_FUNC,
    g_param_spec_pointer( "curve-func", "Curve func", "Curve function", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_CURVE_DATA,
    g_param_spec_pointer( "curve-data", "Curve data", "Curve function data", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

}


static void gp_ica_curve_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec )
{
  GpIcaCurve *curve = GP_ICA_CURVE(object);
  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  switch ( prop_id )
    {

    case PROP_STATE_RENDERER:
      priv->state_renderer = g_value_get_object( value );
      break;

    case PROP_CURVE_FUNC:
      priv->curve_func = g_value_get_pointer( value );
      break;

    case PROP_CURVE_DATA:
      priv->curve_data = g_value_get_pointer( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( curve, prop_id, pspec );
      break;

    }

}


static GObject*gp_ica_curve_constructor( GType g_type, guint n_construct_properties,
                                         GObjectConstructParam *construct_properties )
{

  GObject *curve = G_OBJECT_CLASS( gp_ica_curve_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( curve == NULL ) return NULL;

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  priv->curve_points = g_array_new( FALSE, FALSE, sizeof(GpIcaCurvePoint) );

  priv->curve_color = cairo_sdline_color( g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), 1.0 );
  priv->point_color = cairo_sdline_color( g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), 1.0 );

  priv->selected_point = -1;
  priv->move_point = FALSE;

  priv->curve_surface = NULL;
  priv->curve_update = TRUE;

  return curve;

}


static void gp_ica_curve_finalize(GObject *object)
{
  GpIcaCurve *curve = GP_ICA_CURVE(object);
  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  g_array_unref( priv->curve_points );

  G_OBJECT_CLASS( gp_ica_curve_parent_class )->finalize( G_OBJECT( curve ) );
}


static gint gp_ica_curve_get_renderers_num( GpIcaRenderer *curve )
{

  return SURFACES_NUM;

}


static GpIcaRendererType gp_ica_curve_get_renderer_type( GpIcaRenderer *curve, gint renderer_id )
{

  switch( renderer_id )
    {
    case CURVE_SURFACE: return GP_ICA_RENDERER_VISIBLE;
    }

  return GP_ICA_RENDERER_UNKNOWN;

}


static const gchar *gp_ica_curve_get_name( GpIcaRenderer *curve, gint renderer_id )
{

  switch( renderer_id )
    {
    case CURVE_SURFACE:  return "Cifro curve main renderer";
    }

  return "Unknown renderer.";

}


static void gp_ica_curve_set_surface( GpIcaRenderer *curve, gint renderer_id, guchar *data, gint width, gint height,
                                      gint stride )
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  cairo_surface_t *surface;

  switch( renderer_id )
    {

    case CURVE_SURFACE:

      // Поверхность для рисования осциллограмм.
      cairo_sdline_surface_destroy( priv->curve_surface );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->curve_surface = cairo_sdline_surface_create_for( surface );
      priv->curve_surface->self_create = 1;

      break;

    }

  priv->curve_update = TRUE;

}



static GpIcaRendererAvailability gp_ica_curve_render( GpIcaRenderer *curve, gint renderer_id, gint *x, gint *y,
                                                      gint *width, gint *height )
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );
  const GpIcaState *state = gp_ica_state_renderer_get_state( priv->state_renderer );

  gint i;

  switch( renderer_id )
    {

    case CURVE_SURFACE:

      if( priv->curve_points->len == 0 ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( priv->curve_surface == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( !priv->curve_update ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
      priv->curve_update = FALSE;

      *x = 0;
      *y = 0;
      *width = state->visible_width;
      *height = state->visible_height;
      cairo_sdline_clear_color( priv->curve_surface, 0 );

      // Рисуем кривую.
      for( i = 0; i < state->visible_width; i++ )
        {

        gdouble x_value;
        gdouble y_value;
        gdouble y1, y2;

          gp_ica_state_renderer_visible_point_to_value( priv->state_renderer, i, 0, &x_value, NULL);
        y_value = priv->curve_func( x_value, priv->curve_points, priv->curve_data );
          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, NULL, &y2, x_value, y_value );

        if( i == 0 ) { y1 = y2; continue; }
        cairo_sdline( priv->curve_surface, i - 1, y1, i, y2, priv->curve_color );
        y1 = y2;

        }

      cairo_surface_mark_dirty( priv->curve_surface->cairo_surface );
      cairo_sdline_set_cairo_color( priv->curve_surface, priv->point_color );
      cairo_set_line_width( priv->curve_surface->cairo, 1.0 );

      // Рисуем точки.
      for( i = 0; i < priv->curve_points->len; i++ )
        {

        gdouble x, y;
        gdouble point_radius = 0.4 * state->border_top;

        GpIcaCurvePoint *point = &g_array_index( priv->curve_points, GpIcaCurvePoint, i );
          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, point->x, point->y );
        if( x < 0 || x >= state->visible_width || y < 0 || y >= state->visible_height ) continue;

        cairo_arc( priv->curve_surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius / 4.0, 0.0, 2*G_PI );
        cairo_fill( priv->curve_surface->cairo );

        if( priv->selected_point != i ) continue;

        cairo_arc( priv->curve_surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius, 0.0, 2*G_PI );
        cairo_stroke( priv->curve_surface->cairo );

        }

      cairo_surface_flush( priv->curve_surface->cairo_surface );

      return GP_ICA_RENDERER_AVAIL_ALL;

    }

  return GP_ICA_RENDERER_AVAIL_NONE;

}



static void gp_ica_curve_set_shown( GpIcaRenderer *curve, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y )
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  priv->curve_update = TRUE;

}


static void gp_ica_curve_set_pointer( GpIcaRenderer *curve, gint pointer_x, gint pointer_y, gdouble value_x,
                                      gdouble value_y )
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );
  const GpIcaState *state = gp_ica_state_renderer_get_state( priv->state_renderer );

  guint i;

  gint selected_point;

  gdouble x, y;
  gdouble point_distance;
  gdouble prev_point_distance;
  gdouble point_radius = 0.4 * state->border_top;

  if( priv->move_point ) return;

  // Ищем ближающую рядом с курсором точку, которая находится в радиусе "размера" точки.
  selected_point = -1;
  prev_point_distance = G_MAXDOUBLE;
  for( i = 0; i < priv->curve_points->len; i++ )
    {

    GpIcaCurvePoint *point = &g_array_index( priv->curve_points, GpIcaCurvePoint, i );
      gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x, &y, point->x, point->y );
    if( x < 0 || x >= state->area_width || y < 0 || y >= state->area_height ) continue;

    // Расстояние от курсора до текущей проверяемой точки.
    point_distance = sqrt( ( x - state->pointer_x ) * ( x - state->pointer_x ) +
                           ( y - state->pointer_y ) * ( y - state->pointer_y ) );

    // Если расстояние слишком большое пропускаем эту точку.
    if( point_distance > 2 * point_radius ) continue;

    // Сравниваем с расстоянием до предыдущей ближайшей точки.
    if( point_distance < prev_point_distance )
      {
      selected_point = i;
      prev_point_distance = point_distance;
      }

    }

  if( selected_point != priv->selected_point )
    {
    priv->selected_point = selected_point;
    priv->curve_update = TRUE;
    }

}


static gboolean gp_ica_curve_button_press_event( GpIcaRenderer *curve, GdkEvent *event, GtkWidget *widget )
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  GdkEventButton *button = (GdkEventButton*)event;

  // Обрабатываем только нажатия левой кнопки манипулятора.
  if( button->button != 1 ) return FALSE;

  priv->remove_point = FALSE;
  priv->move_point = FALSE;

  if( priv->selected_point >= 0 )
    {
    // Выбрана точка для перемещения и нажата кнопка Ctrl - нужно удалить эту точку.
    if( button->state & GDK_CONTROL_MASK )
      {
      g_array_remove_index( priv->curve_points, priv->selected_point );
      priv->selected_point = -1;
      priv->curve_update = TRUE;
      }
    // Выбрана точка для перемещения.
    else
      priv->move_point = TRUE;
    }
  else
    {
    // Точка для перемещения не выбрана, но нажата кнопка Ctrl - нужно добавить новую точку.
    if( button->state & GDK_CONTROL_MASK )
      {
      gdouble value_x, value_y;
        gp_ica_state_renderer_area_point_to_value( priv->state_renderer, button->x, button->y, &value_x, &value_y );
        gp_ica_curve_add_point(GP_ICA_CURVE(curve), value_x, value_y);
      }
    }

  return FALSE;

}


static gboolean gp_ica_curve_button_release_event( GpIcaRenderer *curve, GdkEvent *event, GtkWidget *widget )
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  GdkEventButton *button = (GdkEventButton*)event;

  // Обрабатываем только нажатия левой кнопки манипулятора.
  if( button->button != 1 ) return FALSE;

  // Обрабатываем удаление точки при её совмещении с другой точкой.
  if( priv->move_point && priv->remove_point && priv->selected_point >= 0 )
    g_array_remove_index( priv->curve_points, priv->selected_point );

  priv->move_point = FALSE;

  return FALSE;

}


static gboolean gp_ica_curve_motion_notify_event( GpIcaRenderer *curve, GdkEvent *event, GtkWidget *widget )
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );
  const GpIcaState *state = gp_ica_state_renderer_get_state( priv->state_renderer );

  GdkEventMotion *motion = (GdkEventMotion*)event;

  GpIcaCurvePoint *point;
  GpIcaCurvePoint *near_point;

  gdouble value_x, value_y;
  gdouble min_x, max_x;

  gdouble point_radius = 0.4 * state->border_top;

  // Если мы не находимся в режиме перемещения точки.
  if( !priv->move_point ) return FALSE;

  priv->remove_point = FALSE;

  // Вышли за границу окна.
  if( priv->selected_point < 0 ) return TRUE;

  // Расчитываем новое местоположение точки.
  gp_ica_state_renderer_area_point_to_value( priv->state_renderer, motion->x, motion->y, &value_x, &value_y );
  gp_ica_renderer_set_pointer(GP_ICA_RENDERER(priv->state_renderer), motion->x, motion->y, value_x, value_y);

  point = &g_array_index( priv->curve_points, GpIcaCurvePoint, priv->selected_point );

  // Определяем границы перемещения точки и расстояние до соседних точек.
  // Если расстояние до одной из соседних точек меньше чем "радиус" точки - помечаем точку на удаление.
  // Само удаление будет произведено при отпускании кнопки манипулятора.
  if( priv->selected_point > 0 )
    {
    near_point = &g_array_index( priv->curve_points, GpIcaCurvePoint, priv->selected_point - 1 );
    min_x = MAX( state->from_x, near_point->x );
    if( near_point->x > state->from_x )
      {
      gdouble x1, y1, x2, y2;
        gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x1, &y1, value_x, value_y );
        gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x2, &y2, near_point->x, near_point->y );
      if( sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) ) < point_radius ) priv->remove_point = TRUE;
      }
    }
  else
    min_x = state->from_x;

  if( priv->selected_point < priv->curve_points->len - 1 )
    {
    near_point = &g_array_index( priv->curve_points, GpIcaCurvePoint, priv->selected_point + 1 );
    max_x = MIN( state->to_x, near_point->x );
    if( near_point->x < state->to_x )
      {
      gdouble x1, y1, x2, y2;
        gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x1, &y1, value_x, value_y );
        gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x2, &y2, near_point->x, near_point->y );
      if( sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) ) < point_radius ) priv->remove_point = TRUE;
      }
    }
  else
    max_x = state->to_x;

  if( value_x < min_x ) value_x = min_x;
  if( value_x > max_x ) value_x = max_x;
  if( value_y < state->from_y ) value_y = state->from_y;
  if( value_y > state->to_y ) value_y = state->to_y;

  // Задаём новое положение точки.
  point->x = value_x;
  point->y = value_y;

  priv->curve_update = TRUE;

  return TRUE;

}


GpIcaCurve *gp_ica_curve_new(GpIcaStateRenderer *state_renderer, GpIcaCurveFunc curve_func, gpointer curve_data)
{

  return g_object_new(G_TYPE_GP_ICA_CURVE, "state-renderer", state_renderer, "curve-func", curve_func, NULL );

}


void gp_ica_curve_clear_points(GpIcaCurve *curve)
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  g_array_set_size( priv->curve_points, 0 );

  priv->curve_update = TRUE;

}


void gp_ica_curve_add_point(GpIcaCurve *curve, gdouble x, gdouble y)
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  GpIcaCurvePoint new_point;
  GpIcaCurvePoint *point;
  gint i;

  // Ищем место для добавления точки.
  for( i = 0; i < priv->curve_points->len; i++ )
    {
    point = &g_array_index( priv->curve_points, GpIcaCurvePoint, i );
    if( point->x > x ) break;
    }

  new_point.x = x;
  new_point.y = y;
  g_array_insert_val( priv->curve_points, i, new_point );

  priv->curve_update = TRUE;

}


void gp_ica_curve_set_points(GpIcaCurve *curve, GArray *points)
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  GpIcaCurvePoint *point;
  guint i;

  g_array_set_size( priv->curve_points, 0 );
  for( i = 0; i < points->len; i++ )
    {
    point = &g_array_index( points, GpIcaCurvePoint, i );
      gp_ica_curve_add_point(curve, point->x, point->y);
    }

}


GArray *gp_ica_curve_get_points(GpIcaCurve *curve)
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  GArray *points = g_array_new( FALSE, FALSE, sizeof(GpIcaCurvePoint) );

  g_array_insert_vals( points, 0, priv->curve_points->data, priv->curve_points->len );

  return points;

}


void gp_ica_curve_set_curve_color(GpIcaCurve *curve, gdouble red, gdouble green, gdouble blue)
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  priv->curve_color = cairo_sdline_color( red, green, blue, 1.0 );

}


void gp_ica_curve_set_point_color(GpIcaCurve *curve, gdouble red, gdouble green, gdouble blue)
{

  GpIcaCurvePriv *priv = GP_ICA_CURVE_GET_PRIVATE( curve );

  priv->point_color = cairo_sdline_color( red, green, blue, 1.0 );

}


static void gp_ica_curve_interface_init( GpIcaRendererInterface *iface )
{

  iface->get_renderers_num = gp_ica_curve_get_renderers_num;
  iface->get_renderer_type = gp_ica_curve_get_renderer_type;
  iface->get_name = gp_ica_curve_get_name;

  iface->set_surface = gp_ica_curve_set_surface;
  iface->set_area_size = NULL;
  iface->set_visible_size = NULL;

  iface->get_completion = NULL;
  iface->set_total_completion = NULL;
  iface->render = gp_ica_curve_render;
  iface->set_shown_limits = NULL;

  iface->set_border = NULL;
  iface->set_swap = NULL;
  iface->set_angle = NULL;
  iface->set_shown = gp_ica_curve_set_shown;
  iface->set_pointer = gp_ica_curve_set_pointer;

  iface->button_press_event = gp_ica_curve_button_press_event;
  iface->button_release_event = gp_ica_curve_button_release_event;
  iface->key_press_event = NULL;
  iface->key_release_event = NULL;
  iface->motion_notify_event = gp_ica_curve_motion_notify_event;

}
