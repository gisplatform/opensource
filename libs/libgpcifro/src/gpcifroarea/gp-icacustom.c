/*
 * GpIcaCustom is a common graphical library with GpIcaRendererInterface.
 *
 * Copyright (C) 2016 Andrey Vodilov.
 *
 * GpIcaCustom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GpIcaCustom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GpIcaCustom. If not, see <http://www.gnu.org/licenses/>.
 *
*/

/*!
 * \file gp-icacustom.c
 *
 * \author Andrei Vodilov
 * \date 1.02.2016
 * \brief Заголовочный файл класса для отображения пользовательской информации (точки, метки, линии).
 *
 * \defgroup GpIcaCustom GpIcaCustom - класс для отображения пользовательской информации (точки, метки, линии).
 *
 *
 *
*/

#include <math.h>

#include "gp-icacustom.h"
#include "cairosdline.h"


enum { PROP_O, PROP_STATE_RENDERER, PROP_POINT_DATA };

enum { POINT_SURFACE = 0, SURFACES_NUM };

typedef struct GpIcaCustomPriv {


  GpIcaStateRenderer    *state_renderer;   // Псевдо отрисовщик для учёта состояния областей.
  GpIcaCustomDrawType   draw_type;             // Тип данных
  GpIcaCustomLineType   line_type;             // Тип линий

  GArray                *points;     // Точки.

  gpointer               data;       // Пользовательские данные.

  guint32                line_color;      // Цвет кривой.
  guint32                points_color;      // Цвет точек.

  gint                   selected_point;   // Индекс текущей выбранной точки;
  gboolean               move_point;       // Перемещать или нет выбранную точку;
  gboolean               remove_point;     // Удалить выбранную точку.
  gboolean               inside;     // Признак нахождения указателя внутри области.

  cairo_sdline_surface  *surface;    // Поверхность для рисования.
  gboolean               update;     // Признак необходимости перерисовки кривой.

} GpIcaCustomPriv;

#define GP_ICA_CUSTOM_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_GP_ICA_CUSTOM, GpIcaCustomPriv ) )


static void gp_ica_custom_interface_init( GpIcaRendererInterface *iface );
static void gp_ica_custom_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec );
static GObject *gp_ica_custom_constructor( GType g_type, guint n_construct_properties,
                                         GObjectConstructParam *construct_properties );
static void gp_ica_custom_finalize(GObject *object);

G_DEFINE_TYPE_WITH_CODE(GpIcaCustom, gp_ica_custom, G_TYPE_INITIALLY_UNOWNED,
  G_IMPLEMENT_INTERFACE(G_TYPE_GP_ICA_RENDERER, gp_ica_custom_interface_init));

static void gp_ica_custom_init( GpIcaCustom *self ) { ; }


static void gp_ica_custom_class_init( GpIcaCustomClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  g_type_class_add_private( klass, sizeof( GpIcaCustomPriv ) );

  this_class->set_property = gp_ica_custom_set_property;
  this_class->constructor = gp_ica_custom_constructor;
  this_class->finalize = gp_ica_custom_finalize;

  g_object_class_install_property( this_class, PROP_STATE_RENDERER,
    g_param_spec_object( "state-renderer", "State renderer", "State Renderer", G_TYPE_GP_ICA_STATE_RENDERER, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_POINT_DATA,
    g_param_spec_pointer( "point-data", "Point data", "Point function data", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

}


static void gp_ica_custom_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec )
{
  GpIcaCustom *self = GP_ICA_CUSTOM(object);
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  switch ( prop_id )
    {

    case PROP_STATE_RENDERER:
      priv->state_renderer = g_value_get_object( value );
      break;

    case PROP_POINT_DATA:
      priv->data = g_value_get_pointer( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( self, prop_id, pspec );
      break;

    }

}


static GObject *gp_ica_custom_constructor( GType g_type, guint n_construct_properties,
                                         GObjectConstructParam *construct_properties )
{

  GObject *self = G_OBJECT_CLASS( gp_ica_custom_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( self == NULL ) return NULL;

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  priv->points = g_array_new( FALSE, FALSE, sizeof(GpIcaCustomPoints) );

  priv->line_color = cairo_sdline_color( g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), 1.0 );
  priv->points_color = cairo_sdline_color( g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), 1.0 );

  priv->selected_point = -1;
  priv->move_point = FALSE;
  priv->inside = FALSE;

  priv->surface = NULL;
  priv->update = TRUE;

  priv->draw_type = GP_ICA_CUSTOM_DRAW_TYPE_NA;
  priv->line_type = GP_ICA_CUSTOM_LINE_TYPE_NA;

  return self;

}


static void gp_ica_custom_finalize(GObject *object)
{
  GpIcaCustom *self = GP_ICA_CUSTOM(object);
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  g_array_unref( priv->points );

  G_OBJECT_CLASS( gp_ica_custom_parent_class )->finalize( G_OBJECT( self ) );
}


static gint gp_ica_custom_get_renderers_num( GpIcaRenderer *self )
{

  return SURFACES_NUM;

}


static GpIcaRendererType gp_ica_custom_get_renderer_type( GpIcaRenderer *self, gint renderer_id )
{

  switch( renderer_id )
    {
    case POINT_SURFACE: return GP_ICA_RENDERER_AREA;
    }

  return GP_ICA_RENDERER_UNKNOWN;

}


static const gchar *gp_ica_custom_get_name( GpIcaRenderer *self, gint renderer_id )
{

  switch( renderer_id )
    {
    case POINT_SURFACE:  return "Cifro custom main renderer";
    }

  return "Unknown renderer.";

}


static void gp_ica_custom_set_surface( GpIcaRenderer *self, gint renderer_id, guchar *data, gint width, gint height,
                                      gint stride )
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  cairo_surface_t *surface;

  switch( renderer_id )
    {

    case POINT_SURFACE:

      // Поверхность для рисования осциллограмм.
      cairo_sdline_surface_destroy( priv->surface );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->surface = cairo_sdline_surface_create_for( surface );
      priv->surface->self_create = 1;

      break;

    }

  priv->update = TRUE;

}



static GpIcaRendererAvailability gp_ica_custom_render( GpIcaRenderer *self, gint renderer_id, gint *x, gint *y,
                                                      gint *width, gint *height )
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );
  const GpIcaState *state = gp_ica_state_renderer_get_state( priv->state_renderer );

  if(priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_NA)
    return GP_ICA_RENDERER_AVAIL_NONE;


  gint i;

  switch( renderer_id )
  {

    case POINT_SURFACE:
    {

      if( priv->points->len == 0 ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( priv->surface == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( !priv->update ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
      priv->update = FALSE;

      *x = 0;
      *y = 0;
      *width = state->visible_width;
      *height = state->visible_height;
      cairo_sdline_clear_color( priv->surface, 0 );

      cairo_surface_mark_dirty( priv->surface->cairo_surface );
      cairo_sdline_set_cairo_color( priv->surface, priv->line_color );
      cairo_set_line_width( priv->surface->cairo, 3.0 );

      cairo_set_line_cap(priv->surface->cairo, CAIRO_LINE_CAP_ROUND);
      // Рисуем точки.

      cairo_rectangle(priv->surface->cairo, 0, 0, state->visible_width, state->visible_height);
      cairo_clip(priv->surface->cairo);

      gdouble point_radius = 30;//0.4 * state->border_top;

      switch(priv->draw_type)
      {
        case GP_ICA_CUSTOM_DRAW_TYPE_CUR_INDEX:
        {
          GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, 0);
          gdouble x, y;
          gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

          cairo_set_source_rgba(priv->surface->cairo, 1, 1, 1, 0.6);
          cairo_set_line_width( priv->surface->cairo, 5.0 );
          cairo_move_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y - point_radius));
          cairo_line_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y + point_radius));
          cairo_stroke( priv->surface->cairo );
          cairo_move_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x - point_radius), gp_ica_state_point_to_cairo( y));
          cairo_line_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x + point_radius), gp_ica_state_point_to_cairo( y));
          cairo_stroke( priv->surface->cairo );

          cairo_sdline_set_cairo_color( priv->surface, priv->line_color );
          cairo_set_line_width( priv->surface->cairo, 3.0 );
          cairo_move_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y - point_radius));
          cairo_line_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y + point_radius));
          cairo_stroke( priv->surface->cairo );
          cairo_move_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x - point_radius), gp_ica_state_point_to_cairo( y));
          cairo_line_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x + point_radius), gp_ica_state_point_to_cairo( y));
          cairo_stroke( priv->surface->cairo );

        }
        break;
        case GP_ICA_CUSTOM_DRAW_TYPE_POINT:
        {
          for( i = 0; i < priv->points->len; i++ )
          {
            gdouble x, y;

            GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
            gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

            cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius / 4.0, 0.0, 2*G_PI );
            cairo_fill( priv->surface->cairo );

            if( priv->selected_point != i ) continue;

            cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius, 0.0, 2*G_PI );
            cairo_stroke( priv->surface->cairo );
          }
        }
        break;

        case GP_ICA_CUSTOM_DRAW_TYPE_LINE:
        {
          for( i = 0; i < priv->points->len; i++ )
          {

            gdouble x, y;

            GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
            gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

            if(i == 0)
              cairo_move_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ));
            else
              cairo_line_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ));
          }

          cairo_stroke(priv->surface->cairo);

          for( i = 0; i < priv->points->len; i++ )
          {
            gdouble x, y;
            gdouble point_radius = 30;//0.4 * state->border_top;

            GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
            gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

            cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius / 4.0, 0.0, 2*G_PI );
            cairo_fill( priv->surface->cairo );

            if( priv->selected_point != i ) continue;

            cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius, 0.0, 2*G_PI );
            cairo_stroke( priv->surface->cairo );
          }
        }
        break;
        case GP_ICA_CUSTOM_DRAW_TYPE_LINE_FREE:
        {
          for( i = 0; i < priv->points->len; i++ )
          {

            gdouble x, y;

            GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
            gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

            if(i == 0)
              cairo_move_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ));
            else
              cairo_line_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ));
          }

          cairo_stroke(priv->surface->cairo);

          for( i = 0; i < priv->points->len; i++ )
          {
            gdouble x, y;
            gdouble point_radius = 30;//0.4 * state->border_top;

            GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
            gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

            cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius / 4.0, 0.0, 2*G_PI );
            cairo_fill( priv->surface->cairo );

            if( priv->selected_point != i ) continue;

            cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius, 0.0, 2*G_PI );
            cairo_stroke( priv->surface->cairo );
          }
        }
        break;
        case GP_ICA_CUSTOM_DRAW_TYPE_AREA:
        {
          gdouble start_x, start_y;

          cairo_save(priv->surface->cairo);
          cairo_set_source_rgba(priv->surface->cairo, 1.0, 1.0, 1.0, 0.5);
          for( i = 0; i < priv->points->len; i++ )
          {
            gdouble x, y;

            GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
            gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

            if(i == 0)
            {
              start_x = gp_ica_state_point_to_cairo( x );
              start_y = gp_ica_state_point_to_cairo( y );

              cairo_move_to(priv->surface->cairo, start_x, start_y);
            }
            else
              cairo_line_to(priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ));
          }

          cairo_close_path(priv->surface->cairo);
          cairo_fill_preserve(priv->surface->cairo);

          cairo_restore(priv->surface->cairo);

          cairo_close_path(priv->surface->cairo);
          cairo_stroke(priv->surface->cairo);

          for( i = 0; i < priv->points->len; i++ )
          {
            gdouble x, y;
            gdouble point_radius = 30;//0.4 * state->border_top;

            GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
            gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

            cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius / 4.0, 0.0, 2*G_PI );
            cairo_fill( priv->surface->cairo );

            if( priv->selected_point != i ) continue;

            cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius, 0.0, 2*G_PI );
            cairo_stroke( priv->surface->cairo );
          }
        }
        break;

        case GP_ICA_CUSTOM_DRAW_TYPE_SQUARE:
        {
          if (priv->points->len < 2) break;

          gdouble start_x, start_y, stop_x, stop_y;
          cairo_save(priv->surface->cairo);
          cairo_set_source_rgba(priv->surface->cairo, 1.0, 1.0, 1.0, 0.5);

          //~ for( i = 0; i < 4; i++ )
          {
            gdouble x, y, x_r_start, y_r_start;

            GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, 0 );
            gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

            start_x = gp_ica_state_point_to_cairo( x );
            start_y = gp_ica_state_point_to_cairo( y );

            points = &g_array_index( priv->points, GpIcaCustomPoints, 1 );
            gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

            stop_x = gp_ica_state_point_to_cairo( x );
            stop_y = gp_ica_state_point_to_cairo( y );

            x_r_start = MIN(start_x, stop_x);
            y_r_start = MIN(start_y, stop_y);

            gdouble sub_width = fabs(stop_x - start_x);
            gdouble sub_height = fabs(stop_y - start_y);

            cairo_rectangle(priv->surface->cairo, x_r_start, y_r_start, sub_width, sub_height);
          }

          cairo_close_path(priv->surface->cairo);

          cairo_move_to(priv->surface->cairo, 0, 0);
          cairo_line_to(priv->surface->cairo, *width, 0);
          cairo_line_to(priv->surface->cairo, *width, *height);
          cairo_line_to(priv->surface->cairo, 0, *height);
          cairo_close_path(priv->surface->cairo);

          cairo_set_fill_rule(priv->surface->cairo, CAIRO_FILL_RULE_EVEN_ODD);
          cairo_fill_preserve(priv->surface->cairo);

          cairo_restore(priv->surface->cairo);
          cairo_close_path(priv->surface->cairo);
          cairo_stroke(priv->surface->cairo);
          cairo_fill( priv->surface->cairo );

          cairo_sdline_set_cairo_color( priv->surface, priv->line_color );
          cairo_arc( priv->surface->cairo, start_x, start_y, point_radius / 4.0, 0.0, 2*G_PI );
          cairo_fill( priv->surface->cairo );

          cairo_sdline_set_cairo_color( priv->surface, priv->line_color );
          cairo_arc( priv->surface->cairo, stop_x, stop_y, point_radius / 4.0, 0.0, 2*G_PI );
          cairo_fill( priv->surface->cairo );

        }
        break;
        case GP_ICA_CUSTOM_DRAW_TYPE_PUNCHER:
        {
          if(priv->points->len > 1)
          {
            if(priv->inside || priv->selected_point > -1)
            {
              cairo_sdline_set_cairo_color( priv->surface, priv->points_color );
              for( i = 0; i < priv->points->len; i++ )
              {
                gdouble x, y;

                GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
                gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

                cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius / 4.0, 0.0, 2*G_PI );
                cairo_fill( priv->surface->cairo );

                if( priv->selected_point != i ) continue;

                cairo_arc( priv->surface->cairo, gp_ica_state_point_to_cairo( x ), gp_ica_state_point_to_cairo( y ), point_radius, 0.0, 2*G_PI );
                cairo_stroke( priv->surface->cairo );
              }
            }

            gdouble start_x, start_y, stop_x, stop_y;
            cairo_set_source_rgba(priv->surface->cairo, 1.0, 1.0, 1.0, 0.5);

            for(i = 0; i < priv->points->len - 1; i += 2)
            {
              gdouble x, y;

              GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
              gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

              start_x = gp_ica_state_point_to_cairo( x );
              start_y = gp_ica_state_point_to_cairo( y );

              points = &g_array_index( priv->points, GpIcaCustomPoints, i + 1);
              gp_ica_state_renderer_visible_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );

              stop_x = gp_ica_state_point_to_cairo( x );
              stop_y = gp_ica_state_point_to_cairo( y );

              cairo_rectangle(priv->surface->cairo, MIN(start_x, stop_x), MIN(start_y, stop_y), fabs(start_x - stop_x), fabs(start_y - stop_y));
              cairo_fill( priv->surface->cairo );
            }
          }
        }
        break;
        default:
          return GP_ICA_RENDERER_AVAIL_NONE;

      }
      cairo_surface_flush( priv->surface->cairo_surface );
    }
      return GP_ICA_RENDERER_AVAIL_ALL;

  }

  return GP_ICA_RENDERER_AVAIL_NONE;

}



static void gp_ica_custom_set_shown( GpIcaRenderer *self, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y )
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  priv->update = TRUE;

}


static void gp_ica_custom_set_pointer( GpIcaRenderer *self, gint pointer_x, gint pointer_y, gdouble value_x,
                                      gdouble value_y )
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );
  const GpIcaState *state = gp_ica_state_renderer_get_state( priv->state_renderer );

  guint i;

  gint selected_point;

  gdouble x, y;
  gdouble point_distance;
  gdouble prev_point_distance;
  gdouble point_radius = 30;//0.4 * state->border_top;

  if( priv->move_point ) return;

  // Ищем ближающую рядом с курсором точку, которая находится в радиусе "размера" точки.
  selected_point = -1;
  prev_point_distance = G_MAXDOUBLE;
  for( i = 0; i < priv->points->len; i++ )
    {

    GpIcaCustomPoints *points = &g_array_index( priv->points, GpIcaCustomPoints, i );
      gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x, &y, points->x, points->y );
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
    priv->update = TRUE;
    }

}


static gboolean gp_ica_custom_button_press_event( GpIcaRenderer *self, GdkEvent *event, GtkWidget *widget )
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  if(priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_CUR_INDEX)
    return FALSE;

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
        g_array_remove_index( priv->points, priv->selected_point );
        if (priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_PUNCHER)
        {
          if(priv->points->len % 2 != 0)
          {
            if(priv->selected_point % 2 == 0)
              g_array_remove_index( priv->points, priv->selected_point);
            else
              g_array_remove_index( priv->points, priv->selected_point - 1);
          }
        }

      priv->selected_point = -1;

      if (priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_SQUARE)
        g_array_set_size( priv->points, 0 );

      priv->update = TRUE;
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
        //~  if( button->state & GDK_SHIFT_MASK)
        if(priv->points->len > 0)
        {
          if(priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_LINE && priv->line_type == GP_ICA_CUSTOM_LINE_TYPE_HORIZONTAL)
           //~ if( button->state & GDK_SHIFT_MASK)
          {
            gdouble value_x, value_y;
            GpIcaCustomPoints *perv_point = &g_array_index( priv->points, GpIcaCustomPoints, priv->points->len - 1);
            //~ gp_ica_state_renderer_area_point_to_value( priv->state_renderer, button->x, button->y, &value_x, &value_y );
            gp_ica_state_renderer_visible_point_to_value( priv->state_renderer, button->x, button->y, &value_x, &value_y );
            value_y = perv_point->y;
            gp_ica_custom_add_point(GP_ICA_CUSTOM(self), value_x, value_y);
          }
          //~ else if( priv->line_type == GP_ICA_CUSTOM_LINE_TYPE_VERTICAL)
                //~ {
                  //~ gdouble value_x, value_y;
                  //~ GpIcaCustomPoints *perv_point = &g_array_index( priv->points, GpIcaCustomPoints, priv->points->len - 1);
                  //~ gp_ica_state_renderer_area_point_to_value( priv->state_renderer, button->x, button->y, &value_x, &value_y );
                  //~ value_y = perv_point->y;
                  //~ gp_ica_custom_add_point(GP_ICA_CUSTOM(self), value_x, value_y);
                //~ }
                else
                {
                  gdouble value_x, value_y;
                  //~ gp_ica_state_renderer_area_point_to_value( priv->state_renderer, button->x, button->y, &value_x, &value_y );
                  gp_ica_state_renderer_visible_point_to_value( priv->state_renderer, button->x, button->y, &value_x, &value_y );
                  gp_ica_custom_add_point(GP_ICA_CUSTOM(self), value_x, value_y);
                }
        }
        else
        {
          gdouble value_x, value_y;
          //~ gp_ica_state_renderer_area_point_to_value( priv->state_renderer, button->x, button->y, &value_x, &value_y );
          gp_ica_state_renderer_visible_point_to_value( priv->state_renderer, button->x, button->y, &value_x, &value_y );
          gp_ica_custom_add_point(GP_ICA_CUSTOM(self), value_x, value_y);
        }
      }
    }

  return FALSE;

}


static gboolean gp_ica_custom_button_release_event( GpIcaRenderer *self, GdkEvent *event, GtkWidget *widget )
{
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  if(priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_CUR_INDEX)
    return FALSE;
  GdkEventButton *button = (GdkEventButton*)event;

  // Обрабатываем только нажатия левой кнопки манипулятора.
  if( button->button != 1 ) return FALSE;

  // Обрабатываем удаление точки при её совмещении с другой точкой.
  if( priv->move_point && priv->remove_point && priv->selected_point >= 0 )
  {
    g_array_remove_index( priv->points, priv->selected_point );

    if (priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_SQUARE)
    {
      g_array_set_size( priv->points, 0 );
      priv->update = TRUE;
    }
  }

  priv->move_point = FALSE;

  return FALSE;

}


static gboolean gp_ica_custom_motion_notify_event( GpIcaRenderer *self, GdkEvent *event, GtkWidget *widget )
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );
  const GpIcaState *state = gp_ica_state_renderer_get_state( priv->state_renderer );

  if(priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_CUR_INDEX)
    return FALSE;

  GdkEventMotion *motion = (GdkEventMotion*)event;

  GpIcaCustomPoints *points;
  GpIcaCustomPoints *near_point;

  gdouble value_x, value_y;

  gdouble point_radius = 30;// * state->border_top;

  if (priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_PUNCHER && !priv->move_point)
  {
    if(priv->points->len > 1)
    {
      gdouble start_x, start_y, stop_x, stop_y;
      int i = 0;
      for(i = 0; i < priv->points->len - 1; i += 2)
      {
        gdouble x, y;

        GpIcaCustomPoints *punch_points = &g_array_index( priv->points, GpIcaCustomPoints, i );
        gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x, &y, punch_points->x, punch_points->y );

        start_x = gp_ica_state_point_to_cairo( x );
        start_y = gp_ica_state_point_to_cairo( y );

        punch_points = &g_array_index( priv->points, GpIcaCustomPoints, i + 1);
        gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x, &y, punch_points->x, punch_points->y );

        stop_x = gp_ica_state_point_to_cairo( x );
        stop_y = gp_ica_state_point_to_cairo( y );

        if((motion->x > MIN(start_x, stop_x) && motion->x < MAX(start_x, stop_x))
            &&(motion->y > MIN(start_y, stop_y) && motion->y < MAX(start_y, stop_y)))
        {
          priv->inside = TRUE;
          break;
        }

        else
          priv->inside = FALSE;
      }
    }
    priv->update = TRUE;
    return FALSE;
  }


  // Если мы не находимся в режиме перемещения точки.
  if(!priv->move_point ) return FALSE;
  priv->remove_point = FALSE;

  // Расчитываем новое местоположение точки.
  //~ gp_ica_state_renderer_area_point_to_value( priv->state_renderer, motion->x, motion->y, &value_x, &value_y );
  gp_ica_state_renderer_visible_point_to_value( priv->state_renderer, motion->x, motion->y, &value_x, &value_y );
  gp_ica_renderer_set_pointer(GP_ICA_RENDERER(priv->state_renderer), motion->x, motion->y, value_x, value_y);

  points = &g_array_index( priv->points, GpIcaCustomPoints, priv->selected_point );

  // Определяем границы перемещения точки и расстояние до соседних точек.
  // Если расстояние до одной из соседних точек меньше чем "радиус" точки - помечаем точку на удаление.
  // Само удаление будет произведено при отпускании кнопки манипулятора.
  if (priv->draw_type != GP_ICA_CUSTOM_DRAW_TYPE_SQUARE)
  {
    if( priv->selected_point > 0 )
    {
      near_point = &g_array_index( priv->points, GpIcaCustomPoints, priv->selected_point - 1 );
      if( near_point->x > state->from_x )
        {
          gdouble x1, y1, x2, y2;
          gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x1, &y1, value_x, value_y );
          gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x2, &y2, near_point->x, near_point->y );
        if( sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) ) < point_radius ) priv->remove_point = TRUE;
        }
    }

    if( priv->selected_point < priv->points->len - 1 )
    {
      near_point = &g_array_index( priv->points, GpIcaCustomPoints, priv->selected_point + 1 );

      if( near_point->x < state->to_x )
        {
        gdouble x1, y1, x2, y2;
          gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x1, &y1, value_x, value_y );
          gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x2, &y2, near_point->x, near_point->y );
        if( sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) ) < point_radius ) priv->remove_point = TRUE;
        }
    }
  }

  if(priv->points->len > 1 && priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_LINE)
  {
    //~ if(motion->state & GDK_SHIFT_MASK)
    if(priv->line_type == GP_ICA_CUSTOM_LINE_TYPE_HORIZONTAL)
    {
      GpIcaCustomPoints *perv_point = &g_array_index( priv->points, GpIcaCustomPoints, 1);
      value_y = perv_point->y;
    }
    //~ else if(motion->state & GDK_MOD1_MASK)
    else if(priv->line_type == GP_ICA_CUSTOM_LINE_TYPE_VERTICAL)
          {
            GpIcaCustomPoints *perv_point = &g_array_index( priv->points, GpIcaCustomPoints, 1);
            value_x = perv_point->x;
          }
  }

  if (priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_SQUARE)
  {
    gdouble x1, y1, x2, y2;
    gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x1, &y1, points->x, points->y );

    // Новое положение предыдущей точки
    gint near_index = (priv->selected_point > 0) ? priv->selected_point - 1 : 3;
    near_point = &g_array_index( priv->points, GpIcaCustomPoints, near_index);
    gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x2, &y2, near_point->x, near_point->y );

    //~ gp_ica_state_renderer_area_point_to_value( priv->state_renderer, x2, y2, &near_point->x, &near_point->y );
    gp_ica_state_renderer_visible_point_to_value( priv->state_renderer, x2, y2, &near_point->x, &near_point->y );

    // Новое положение следующей точки
    near_index = (priv->selected_point < 3) ? priv->selected_point + 1 : 0;
    near_point = &g_array_index( priv->points, GpIcaCustomPoints, near_index);
    gp_ica_state_renderer_area_value_to_point( priv->state_renderer, &x2, &y2, near_point->x, near_point->y );

    //~ gp_ica_state_renderer_area_point_to_value( priv->state_renderer, x2, y2, &near_point->x, &near_point->y );
    gp_ica_state_renderer_visible_point_to_value( priv->state_renderer, x2, y2, &near_point->x, &near_point->y );
  }

  if(motion->state & GDK_SHIFT_MASK)
  {
    gdouble delta_x = value_x - points->x;
    gdouble delta_y = value_y - points->y;
    if(priv->selected_point % 2 == 0)
      near_point = &g_array_index( priv->points, GpIcaCustomPoints, priv->selected_point + 1 );
    else
      near_point = &g_array_index( priv->points, GpIcaCustomPoints, priv->selected_point - 1 );

    near_point->x += delta_x;
    near_point->y += delta_y;
  }

  // Задаём новое положение точки.
  points->x = value_x;
  points->y = value_y;

  priv->update = TRUE;

  return TRUE;

}


GpIcaCustom *gp_ica_custom_new(GpIcaStateRenderer *state_renderer, gpointer point_data)
{

  return g_object_new(G_TYPE_GP_ICA_CUSTOM, "state-renderer", state_renderer, NULL );

}


void gp_ica_custom_clear_points(GpIcaCustom *self)
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  g_array_set_size( priv->points, 0 );

  priv->update = TRUE;

}


void gp_ica_custom_add_point(GpIcaCustom *self, gdouble x, gdouble y)
{
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  GpIcaCustomPoints new_point;
  GpIcaCustomPoints *point_perv;
  GpIcaCustomPoints *point_next;
  gint i = 0, index_to_insert = 0;
  gdouble perv_x, perv_y, next_x, next_y, mid_x, mid_y;

  if (priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_SQUARE && priv->points->len >= 2) return;

  mid_x = gp_ica_state_point_to_cairo(x);
  mid_y = gp_ica_state_point_to_cairo(y);

  // Ищем место для добавления точки.
  if(priv->points->len > 1)
    for( i = 1; i < priv->points->len - 1; i++ )
      {
      point_perv = &g_array_index( priv->points, GpIcaCustomPoints, i );
      point_next = &g_array_index( priv->points, GpIcaCustomPoints, i + 1);

      perv_x = point_perv->x;
      perv_y = point_perv->y;
      next_x = point_next->x;
      next_y = gp_ica_state_point_to_cairo(point_next->y);
      if(((mid_x > MIN(perv_x, next_x) && mid_x < MAX(perv_x, next_x)) && (mid_y > MIN(perv_y, next_y) && mid_y < MAX(perv_y, next_y)))
       || (fabs(point_perv->x - point_next->x) < 2 && (y > MIN(point_perv->y, point_next->y)) && (y < MAX(point_perv->y, point_next->y)))
       || (x > MIN(point_perv->x, point_next->x) && x < MAX(point_perv->x, point_next->x) && fabs(point_perv->y - point_next->y) < 2)
        )
        {
          double a = next_y - perv_y;
          double b = perv_x - next_x;
          double c = next_y * perv_x - next_x * perv_y;

          double delta = (a * mid_x + b * mid_y - c) / sqrt(a*a + b*b);

          if(fabs(delta) <= 5)
          {
            index_to_insert = i + 1;
            break;
          }
        }
      }

  new_point.x = x;
  new_point.y = y;

  if(index_to_insert != 0)
    g_array_insert_val( priv->points, index_to_insert, new_point );
  else
    g_array_append_val( priv->points, new_point );

  priv->update = TRUE;

}

gint gp_ica_custom_get_points_count(GpIcaCustom *self)
{
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );
  return priv->points->len;
}

void gp_ica_custom_get_points_data(GpIcaCustom *self, GpIcaCustomPoints *data, guint *len)
{
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );
  data = g_memdup(priv->points->data, priv->points->len * sizeof(GpIcaCustomPoints));
  *len = priv->points->len;
}

gboolean gp_ica_custom_update_num_point_coords(GpIcaCustom *self, guint n, gdouble x, gdouble y)
{
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );
  if(n >= priv->points->len)
    return FALSE;

  GpIcaCustomPoints *point = &g_array_index( priv->points, GpIcaCustomPoints, n );
  point->x = x;
  point->y = y;
  priv->update = TRUE;
  //(priv->points->data + priv->points->len * sizeof(GpIcaCustomPoints) * n)

  return TRUE;
}

gboolean gp_ica_custom_get_num_point_coords(GpIcaCustom *self, guint n, gdouble *x, gdouble *y)
{
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );
  if(n >= priv->points->len)
    return FALSE;

  GpIcaCustomPoints *point = &g_array_index( priv->points, GpIcaCustomPoints, n );

  *x = point->x;
  *y = point->y;

  return TRUE;
}

void gp_ica_custom_set_points(GpIcaCustom *self, GArray *points)
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  GpIcaCustomPoints *points_to_set;
  guint i;

  g_array_set_size( priv->points, 0 );
  for( i = 0; i < points->len; i++ )
    {
    points_to_set = &g_array_index( points, GpIcaCustomPoints, i );
      gp_ica_custom_add_point(self, points_to_set->x, points_to_set->y);
    }

}

GArray *gp_ica_custom_get_points(GpIcaCustom *self)
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  GArray *points = g_array_new( FALSE, FALSE, sizeof(GpIcaCustomPoints) );

  g_array_insert_vals( points, 0, priv->points->data, priv->points->len );

  return points;

}


void gp_ica_custom_set_line_color(GpIcaCustom *self, gdouble red, gdouble green, gdouble blue)
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  if (priv->draw_type == GP_ICA_CUSTOM_DRAW_TYPE_SQUARE)
    priv->line_color = cairo_sdline_color( red, green, blue, 0.0 );
  else
    priv->line_color = cairo_sdline_color( red, green, blue, 1.0 );

}


void gp_ica_custom_set_points_color(GpIcaCustom *self, gdouble red, gdouble green, gdouble blue)
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  priv->points_color = cairo_sdline_color( red, green, blue, 1.0 );

}

void gp_ica_custom_set_draw_type(GpIcaCustom *self, GpIcaCustomDrawType new_type)
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  if(new_type < GP_ICA_CUSTOM_DRAW_TYPE_NA || new_type >= GP_ICA_CUSTOM_DRAW_TYPE_AMOUNT)
  {
    g_warning("Custom set draw type.........................Error");
    return;
  }
  priv->draw_type = new_type;
  priv->update = TRUE;
}

GpIcaCustomDrawType gp_ica_custom_get_draw_type(GpIcaCustom *self)
{
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  return priv->draw_type;
}

void gp_ica_custom_set_line_type(GpIcaCustom *self, GpIcaCustomLineType new_type)
{

  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  if(new_type < GP_ICA_CUSTOM_LINE_TYPE_NA || new_type >= GP_ICA_CUSTOM_LINE_TYPE_AMOUNT)
  {
    g_warning("Custom set type.........................Error");
    return;
  }
  priv->line_type = new_type;
  priv->update = TRUE;
}

GpIcaCustomLineType gp_ica_custom_get_line_type(GpIcaCustom *self)
{
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  return priv->line_type;
}

gdouble gp_ica_custom_get_calc_value(GpIcaCustom *self)
{
  GpIcaCustomPriv *priv = GP_ICA_CUSTOM_GET_PRIVATE( self );

  GpIcaCustomPoints *point_perv;
  GpIcaCustomPoints *point_next;

  gdouble value_to_return = 0.0;
  gint i;
  if(priv->points->len > 1)
    for( i = 0; i < priv->points->len - 1; i++ )
      {
      point_perv = &g_array_index( priv->points, GpIcaCustomPoints, i );
      point_next = &g_array_index( priv->points, GpIcaCustomPoints, i + 1);

      switch(priv->draw_type)
      {
        case GP_ICA_CUSTOM_DRAW_TYPE_LINE:
        {
          value_to_return += sqrt((point_perv->x - point_next->x) * (point_perv->x - point_next->x) + (point_perv->y - point_next->y) * (point_perv->y - point_next->y));
        }
        break;
        default:
        {
          value_to_return += (gdouble)i;//
        }
        break;
      }
    }

  return value_to_return;
}

static void gp_ica_custom_interface_init( GpIcaRendererInterface *iface )
{

  iface->get_renderers_num = gp_ica_custom_get_renderers_num;
  iface->get_renderer_type = gp_ica_custom_get_renderer_type;
  iface->get_name = gp_ica_custom_get_name;

  iface->set_surface = gp_ica_custom_set_surface;
  iface->set_area_size = NULL;
  iface->set_visible_size = NULL;

  iface->get_completion = NULL;
  iface->set_total_completion = NULL;
  iface->render = gp_ica_custom_render;
  iface->set_shown_limits = NULL;

  iface->set_border = NULL;
  iface->set_swap = NULL;
  iface->set_angle = NULL;
  iface->set_shown = gp_ica_custom_set_shown;
  iface->set_pointer = gp_ica_custom_set_pointer;

  iface->button_press_event = gp_ica_custom_button_press_event;
  iface->button_release_event = gp_ica_custom_button_release_event;
  iface->key_press_event = NULL;
  iface->key_release_event = NULL;
  iface->motion_notify_event = gp_ica_custom_motion_notify_event;

}
