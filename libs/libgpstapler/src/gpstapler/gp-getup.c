/*
 * GpGetup is a grid library with GpIcaRendererInterface.
 *
 * Copyright (C) 2013 Andrey Vodilov.
 *
 * GpGetup is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GpGetup is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GpGetup. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include "gp-getup.h"

#include <string.h>
#include <stdio.h>
#include <cairo.h>
#include <math.h>


typedef struct GpGetupPriv {

  GpIcaStateRenderer *state_renderer;            // Псевдо отрисовщик для учёта состояния областей.
  const GpIcaState *state;                     // Состояние областей отрисовки.

  cairo_t *axis_cairo;
  cairo_t *border_cairo;

  gboolean axis_need_update;
  gboolean border_need_update;

} GpGetupPriv;

#define GP_GETUP_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_GP_GETUP, GpGetupPriv ) )


static void gp_getup_interface_init( GpIcaRendererInterface *iface );
static void gp_getup_init( GpGetup *grenderer );
static void gp_getup_set_property( GpGetup *grenderer, guint prop_id, const GValue *value, GParamSpec *pspec );
static GObject *gp_getup_constructor( GType type, guint n_construct_properties,
                                     GObjectConstructParam *construct_properties );
static void gp_getup_finalize( GpGetup *grenderer );


enum { PROP_O, PROP_STATE_RENDERER };

G_DEFINE_TYPE_WITH_CODE( GpGetup, gp_getup, G_TYPE_INITIALLY_UNOWNED,
                         G_IMPLEMENT_INTERFACE( G_TYPE_GP_ICA_RENDERER, gp_getup_interface_init ));


static void gp_getup_class_init( GpGetupClass *class )
{

  GObjectClass *parent_class = G_OBJECT_CLASS( class );

  g_type_class_add_private( class, sizeof( GpGetupPriv ) );

  parent_class->constructor = (void*) gp_getup_constructor;
  parent_class->set_property = (void*) gp_getup_set_property;
  parent_class->finalize = (void*) gp_getup_finalize;

  g_object_class_install_property( parent_class, PROP_STATE_RENDERER,
    g_param_spec_object( "state_renderer", "state_renderer", "State Renderer", G_TYPE_GP_ICA_STATE_RENDERER, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

}


static void gp_getup_init( GpGetup *grenderer )
{

  GpGetupPriv *grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );

  memset( grenderer_priv, 0, sizeof( GpGetupPriv ) );

  grenderer_priv->axis_cairo = NULL;
  grenderer_priv->border_cairo = NULL;

  grenderer_priv->axis_need_update = TRUE;
  grenderer_priv->border_need_update = TRUE;
}


// Функция установки параметров.
static void gp_getup_set_property( GpGetup *grenderer, guint prop_id, const GValue *value, GParamSpec *pspec )
{

  GpGetupPriv *grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );

  switch ( prop_id )
    {

    case PROP_STATE_RENDERER:
      grenderer_priv->state_renderer = g_value_get_object( value );
      grenderer_priv->state = gp_ica_state_renderer_get_state( grenderer_priv->state_renderer );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( grenderer, prop_id, pspec );
      break;

    }

}


// Конструктор объекта.
static GObject*gp_getup_constructor( GType type, guint n_construct_properties,
                                     GObjectConstructParam *construct_properties )
{

  GpGetup *grenderer;
  GpGetupPriv *grenderer_priv;
  GObjectClass *parent_class;

  parent_class = g_type_class_peek_parent( g_type_class_peek( G_TYPE_GP_GETUP ) );
  grenderer = GP_GETUP( parent_class->constructor( type, n_construct_properties, construct_properties ) );
  grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );

  if( grenderer_priv->state_renderer == NULL )
    {
    g_object_unref( grenderer );
    return NULL;
    }
	
  return G_OBJECT( grenderer );

}


// Освобождение памяти занятой объектом.
static void gp_getup_finalize( GpGetup *grenderer )
{

  GpGetupPriv *grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );

  if( grenderer_priv->axis_cairo ) cairo_destroy( grenderer_priv->axis_cairo );
  if( grenderer_priv->border_cairo ) cairo_destroy( grenderer_priv->border_cairo );

}


static gint gp_getup_get_renderers_num( GpIcaRenderer *grenderer )
{

  return GP_GETUP_RENDERERS_NUM;

}


static const gchar *gp_getup_get_name( GpIcaRenderer *grenderer, gint renderer_id )
{

  switch( renderer_id )
    {
    case GP_GETUP_AXIS: return "Axis renderer.";
    case GP_GETUP_BORDER: return "Border renderer.";
    }

  return "Unknown renderer.";

}


static GpIcaRendererType gp_getup_get_renderer_type( GpIcaRenderer *grenderer, gint renderer_id )
{

  switch( renderer_id )
    {
    case GP_GETUP_AXIS: return GP_ICA_RENDERER_VISIBLE;
    case GP_GETUP_BORDER: return GP_ICA_RENDERER_AREA;
    }

  return GP_ICA_RENDERER_UNKNOWN;

}


static void gp_getup_set_surface( GpIcaRenderer *grenderer, gint renderer_id, guchar *data, gint width, gint height,
                                  gint stride )
{

  GpGetupPriv *grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );
  cairo_surface_t *surface;

  switch( renderer_id )
    {

    case GP_GETUP_AXIS:
      if( grenderer_priv->axis_cairo ) cairo_destroy( grenderer_priv->axis_cairo );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      grenderer_priv->axis_cairo = cairo_create( surface );
      cairo_surface_destroy( surface );
      break;

    case GP_GETUP_BORDER:
      if( grenderer_priv->border_cairo ) cairo_destroy( grenderer_priv->border_cairo );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      grenderer_priv->border_cairo = cairo_create( surface );
      cairo_surface_destroy( surface );
      break;
      

    }

}


static GpIcaRendererAvailability gp_getup_render( GpIcaRenderer *grenderer, gint renderer_id, gint *x, gint *y,
                                                  gint *width, gint *height )
{

  GpGetupPriv *grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );
  cairo_t *cairo = NULL;

  if( grenderer_priv->state->area_width < 64 || grenderer_priv->state->area_height < 64 ) return GP_ICA_RENDERER_AVAIL_NONE;
  if( ( renderer_id == GP_GETUP_AXIS) && ( !grenderer_priv->axis_need_update ) ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
  if( ( renderer_id == GP_GETUP_BORDER) && ( !grenderer_priv->border_need_update ) ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;

  switch( renderer_id )
    {
    case GP_GETUP_AXIS:
      cairo = grenderer_priv->axis_cairo;
      break;
    case GP_GETUP_BORDER:
      cairo = grenderer_priv->border_cairo;
      break;
    }

  if( cairo == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;

  cairo_set_operator( cairo, CAIRO_OPERATOR_SOURCE );
  cairo_set_source_rgba( cairo, 1.0, 0.0, 0.0, 0.0 );
  cairo_paint( cairo );

  cairo_set_operator( cairo, CAIRO_OPERATOR_OVER );
  cairo_set_line_width( cairo, 1.0 );

  switch( renderer_id )
    {

    case GP_GETUP_AXIS:
      {

      gint range;
      gint grid_width = 100;
      gdouble cur_val, from_val, step_val;
      gdouble xs, ys;

      cairo_set_source_rgba( cairo, 0.11, 0.11, 0.33, 0.9 );

      // Оцифровка по X.
      from_val = grenderer_priv->state->from_x;
        gp_ica_state_get_axis_step( grenderer_priv->state->cur_scale_x, grid_width, &from_val, &step_val, &range, NULL);
      for( cur_val = from_val; cur_val < grenderer_priv->state->to_x; cur_val += step_val )
        {

          gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, cur_val,
                                               grenderer_priv->state->from_y );
        cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

          gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, cur_val,
                                               grenderer_priv->state->to_y );
        cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        cairo_stroke( cairo );

        }

      // Оцифровка по Y.
      from_val = grenderer_priv->state->from_y;
        gp_ica_state_get_axis_step( grenderer_priv->state->cur_scale_y, grid_width, &from_val, &step_val, &range, NULL);
      for( cur_val = from_val; cur_val < grenderer_priv->state->to_y; cur_val += step_val )
        {

          gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, grenderer_priv->state->from_x,
                                               cur_val );
        cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

          gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, grenderer_priv->state->to_x,
                                               cur_val );
        cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        cairo_stroke( cairo );

        }

      cairo_set_source_rgba( cairo, 0.11, 0.11, 0.88, 0.9 );

      // Нулевая ось X.
        gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, grenderer_priv->state->from_x,
                                             0 );
      cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

        gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, grenderer_priv->state->to_x, 0 );
      cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
      cairo_stroke( cairo );

      // Нулевая ось Y.
        gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, 0,
                                             grenderer_priv->state->from_y );
      cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

        gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, 0, grenderer_priv->state->to_y );
      cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
      cairo_stroke( cairo );
	
		cairo_text_extents_t te;
		cairo_set_font_size(cairo, 15);
		gchar buf[50];
		
      // Засечки по X.
      from_val = grenderer_priv->state->from_x;
        gp_ica_state_get_axis_step( grenderer_priv->state->cur_scale_x, grid_width, &from_val, &step_val, &range, NULL);
      for( cur_val = from_val; cur_val < grenderer_priv->state->to_x; cur_val += step_val )
        {

          gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, cur_val, -step_val / 10 );
        cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

          gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, cur_val, step_val / 10 );
        cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        cairo_stroke( cairo );
		
		g_snprintf(buf, sizeof(buf), "%1.2f", cur_val);
		cairo_text_extents(cairo, buf, &te);
		
		cairo_move_to (cairo, gp_ica_state_point_to_cairo( xs ) - te.width - te.x_bearing,
				10 + te.height / 2 - te.y_bearing);
				
		cairo_save(cairo);
		cairo_set_source_rgb(cairo, 1, 0, 0);
		cairo_text_path(cairo, buf);
		cairo_fill(cairo);
		cairo_restore(cairo);
        }

      // Засечки по Y.
      from_val = grenderer_priv->state->from_y;
        gp_ica_state_get_axis_step( grenderer_priv->state->cur_scale_y, grid_width, &from_val, &step_val, &range, NULL);
      for( cur_val = from_val; cur_val < grenderer_priv->state->to_y; cur_val += step_val )
        {

          gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, -step_val / 10, cur_val );
        cairo_move_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );

          gp_ica_state_renderer_visible_value_to_point( grenderer_priv->state_renderer, &xs, &ys, step_val / 10, cur_val );
        cairo_line_to( cairo, gp_ica_state_point_to_cairo( xs ), gp_ica_state_point_to_cairo( ys ) );
        cairo_stroke( cairo );
        
        g_snprintf(buf, sizeof(buf), "%1.2f", cur_val);
		cairo_text_extents(cairo, buf, &te);
		
		cairo_move_to (cairo, te.height / 2 - te.y_bearing,
                   gp_ica_state_point_to_cairo( ys ));
				
		cairo_save(cairo);
		cairo_set_source_rgb(cairo, 1, 0, 0);
		cairo_text_path(cairo, buf);
		cairo_fill(cairo);
		cairo_restore(cairo);

        }

      if( x ) *x = 0;
      if( y ) *y = 0;
      if( width ) *width = grenderer_priv->state->visible_width;
      if( height ) *height = grenderer_priv->state->visible_height;

      grenderer_priv->axis_need_update = FALSE;

      break;

      }

    case GP_GETUP_BORDER:
      {
      gdouble left, top;
      gdouble bwidth, bheight;

      left = grenderer_priv->state->border_left - 0.5;
      top = grenderer_priv->state->border_top - 0.5;
      bwidth = grenderer_priv->state->area_width - grenderer_priv->state->border_left - grenderer_priv->state->border_right + 1;
      bheight = grenderer_priv->state->area_height - grenderer_priv->state->border_top - grenderer_priv->state->border_bottom + 1;

      cairo_set_source_rgb( cairo, 0.11, 0.88, 0.11 );
      cairo_rectangle( cairo, left, top, bwidth, bheight );
      cairo_stroke( cairo );

      if( x ) *x = 0;
      if( y ) *y = 0;
      if( width ) *width = grenderer_priv->state->area_width;
      if( height ) *height = grenderer_priv->state->area_height;

      grenderer_priv->border_need_update = FALSE;

      break;

      }
  }
  return GP_ICA_RENDERER_AVAIL_ALL;

}


static void gp_getup_set_visible_size( GpIcaRenderer *grenderer, gint width, gint height )
{

  GpGetupPriv *grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );

  grenderer_priv->axis_need_update = TRUE;

}


static void gp_getup_set_area_size( GpIcaRenderer *grenderer, gint width, gint height )
{

  GpGetupPriv *grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );

  grenderer_priv->border_need_update = TRUE;

}


static void gp_getup_set_border( GpIcaRenderer *grenderer, gint left, gint right, gint top, gint bottom )
{

  GpGetupPriv *grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );

  grenderer_priv->border_need_update = TRUE;

}


static void gp_getup_set_shown( GpIcaRenderer *grenderer, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y )
{

  GpGetupPriv *grenderer_priv = GP_GETUP_GET_PRIVATE( grenderer );

  grenderer_priv->axis_need_update = TRUE;

}


GpGetup *gp_getup_new( GpIcaStateRenderer *state_renderer )
{

  return g_object_new( G_TYPE_GP_GETUP, "state_renderer", state_renderer, NULL );

}


static void gp_getup_interface_init( GpIcaRendererInterface *iface )
{

  iface->get_renderers_num = gp_getup_get_renderers_num;
  iface->get_renderer_type = gp_getup_get_renderer_type;
  iface->get_name = gp_getup_get_name;

  iface->set_surface = gp_getup_set_surface;
  iface->set_area_size = gp_getup_set_area_size;
  iface->set_visible_size = gp_getup_set_visible_size;

  iface->get_completion = NULL;
  iface->set_total_completion = NULL;
  iface->render = gp_getup_render;

  iface->set_border = gp_getup_set_border;
  iface->set_swap = NULL;
  iface->set_angle = NULL;
  iface->set_shown = gp_getup_set_shown;
  iface->set_pointer = NULL;

  iface->button_press_event = NULL;
  iface->button_release_event = NULL;
  iface->key_press_event = NULL;
  iface->key_release_event = NULL;
  iface->motion_notify_event = NULL;

}
