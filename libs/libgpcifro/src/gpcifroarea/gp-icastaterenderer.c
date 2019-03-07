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
 * \file icastaterenderer.c
 *
 * \author Andrei Fadeev
 * \date 5.04.2013
 * \brief Исходный файл класса отслеживания состояния CifroArea и CifroImage.
 *
*/

#include "gp-icastaterenderer.h"

#include <math.h>
#include <string.h>

#define GP_ICA_STATE_RENDERER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_GP_ICA_STATE_RENDERER, GpIcaState ) )


static void gp_ica_state_renderer_interface_init( GpIcaRendererInterface *iface );


G_DEFINE_TYPE_EXTENDED(GpIcaStateRenderer, gp_ica_state_renderer, G_TYPE_INITIALLY_UNOWNED, 0,
  G_IMPLEMENT_INTERFACE(G_TYPE_GP_ICA_RENDERER, gp_ica_state_renderer_interface_init ));


static void gp_ica_state_renderer_class_init( GpIcaStateRendererClass *class )
{

  g_type_class_add_private( class, sizeof( GpIcaState ) );

}


static void gp_ica_state_renderer_init( GpIcaStateRenderer *state_renderer )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  memset( state, 0, sizeof( GpIcaState ) );

  state->pointer_x = -1;
  state->pointer_y = -1;

}


static gint gp_ica_state_renderer_get_renderers_num( GpIcaRenderer *state_renderer )
{

  return 1;

}


static const gchar *gp_ica_state_renderer_get_name( GpIcaRenderer *state_renderer, gint renderer_id )
{

  return "State renderer.";

}


static GpIcaRendererType gp_ica_state_renderer_get_renderer_type( GpIcaRenderer *state_renderer, gint renderer_id )
{

  return GP_ICA_RENDERER_STATE;

}


static void gp_ica_state_renderer_set_visible_size( GpIcaRenderer *state_renderer, gint width, gint height )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  state->visible_width = width;
  state->visible_height = height;

  state->cur_scale_x = ( state->to_x - state->from_x ) / state->visible_width;
  state->cur_scale_y = ( state->to_y - state->from_y ) / state->visible_height;

}


static void gp_ica_state_renderer_set_total_completion( GpIcaRenderer *state_renderer, gint completion )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  state->completion = completion;

}


static void gp_ica_state_renderer_set_area_size( GpIcaRenderer *state_renderer, gint width, gint height )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  state->area_width = width;
  state->area_height = height;

}


static void gp_ica_state_renderer_set_shown_limits( GpIcaRenderer *state_renderer, gdouble min_x, gdouble max_x,
                                                    gdouble min_y, gdouble max_y )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  state->min_x = min_x;
  state->max_x = max_x;
  state->min_y = min_y;
  state->max_y = max_y;

}


static void gp_ica_state_renderer_set_border( GpIcaRenderer *state_renderer, gint left, gint right, gint top,
                                              gint bottom )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  state->border_left = left;
  state->border_right = right;
  state->border_top = top;
  state->border_bottom = bottom;

}


static void gp_ica_state_renderer_set_swap( GpIcaRenderer *state_renderer, gboolean swap_x, gboolean swap_y )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  state->swap_x = swap_x;
  state->swap_y = swap_y;

}


static void gp_ica_state_renderer_set_angle( GpIcaRenderer *state_renderer, gdouble angle )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  state->angle = angle;
  state->angle_cos = cos( angle );
  state->angle_sin = sin( angle );

}


static void gp_ica_state_renderer_set_shown( GpIcaRenderer *state_renderer, gdouble from_x, gdouble to_x,
                                             gdouble from_y, gdouble to_y )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  state->from_x = from_x;
  state->to_x = to_x;
  state->from_y = from_y;
  state->to_y = to_y;

  if( state->visible_width > 0 )
    state->cur_scale_x = ( state->to_x - state->from_x ) / state->visible_width;
  if( state->visible_height > 0 )
    state->cur_scale_y = ( state->to_y - state->from_y ) / state->visible_height;

}


static void gp_ica_state_renderer_set_pointer( GpIcaRenderer *state_renderer, gint pointer_x, gint pointer_y,
                                               gdouble value_x, gdouble value_y )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  state->pointer_x = pointer_x;
  state->pointer_y = pointer_y;
  state->value_x = value_x;
  state->value_y = value_y;

}


// Создание объекта GpIcaStateRenderer.
GpIcaStateRenderer *gp_ica_state_renderer_new( void )
{

  return g_object_new( G_TYPE_GP_ICA_STATE_RENDERER, NULL );

}


// Возвращает указатель на структуру состояния.
const GpIcaState *gp_ica_state_renderer_get_state( GpIcaStateRenderer *state_renderer )
{

  return GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

}


//  Преобразование координат из прямоугольной системы окна в логические координаты.
void gp_ica_state_renderer_area_point_to_value( GpIcaStateRenderer *state_renderer, gdouble x, gdouble y, gdouble *x_val,
                                       gdouble *y_val )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  gdouble x_val_tmp;
  gdouble y_val_tmp;

  x = x - state->area_width / 2.0;
  y = state->area_height / 2.0 - y;

  if( state->angle != 0.0 )
    {
    x_val_tmp = ( x * state->angle_cos - y * state->angle_sin ) * state->cur_scale_x;
    y_val_tmp = ( y * state->angle_cos + x * state->angle_sin ) * state->cur_scale_y;
    }
  else
    {
    x_val_tmp = x * state->cur_scale_x;
    y_val_tmp = y * state->cur_scale_y;
    }

  if( state->swap_x ) x_val_tmp = -x_val_tmp;
  if( state->swap_y ) y_val_tmp = -y_val_tmp;

  x_val_tmp = ( ( state->to_x - state->from_x ) / 2.0 ) + state->from_x + x_val_tmp;
  y_val_tmp = ( ( state->to_y - state->from_y ) / 2.0 ) + state->from_y + y_val_tmp;

  if( x_val ) *x_val = x_val_tmp;
  if( y_val ) *y_val = y_val_tmp;

}


// Преобразование координат из логических в прямоугольную систему координат окна.
void gp_ica_state_renderer_area_value_to_point( GpIcaStateRenderer *state_renderer, gdouble *x, gdouble *y, gdouble x_val,
                                       gdouble y_val )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  gdouble x_tmp;
  gdouble y_tmp;

  x_val = x_val - ( ( state->to_x - state->from_x ) / 2.0 ) - state->from_x;
  y_val = y_val - ( ( state->to_y - state->from_y ) / 2.0 ) - state->from_y;

  x_val = x_val / state->cur_scale_x;
  y_val = y_val / state->cur_scale_y;

  if( state->swap_x ) x_val = -x_val;
  if( state->swap_y ) y_val = -y_val;

  if( state->angle != 0.0 )
    {
    x_tmp = x_val * state->angle_cos + y_val * state->angle_sin;
    y_tmp = y_val * state->angle_cos - x_val * state->angle_sin;
    }
  else
    {
    x_tmp = x_val;
    y_tmp = y_val;
    }

  x_tmp = x_tmp + state->area_width / 2.0;
  y_tmp = state->area_height / 2.0 - y_tmp;

  if( x ) *x = x_tmp;
  if( y ) *y = y_tmp;

}


// Преобразование координат из прямоугольной системы видимой области в логические координаты.
void gp_ica_state_renderer_visible_point_to_value( GpIcaStateRenderer *state_renderer, gdouble x, gdouble y, gdouble *x_val,
                                          gdouble *y_val )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  if( x_val ) *x_val = state->from_x + x * state->cur_scale_x;
  if( y_val ) *y_val = state->to_y - y * state->cur_scale_y;

}


// Преобразование координат из логических в прямоугольную систему координат видимой области.
void gp_ica_state_renderer_visible_value_to_point( GpIcaStateRenderer *state_renderer, gdouble *x, gdouble *y, gdouble x_val,
                                          gdouble y_val )
{

  GpIcaState *state = GP_ICA_STATE_RENDERER_GET_PRIVATE( state_renderer );

  if( x ) *x = ( x_val - state->from_x ) / state->cur_scale_x;
  if( y ) *y = ( state->to_y - y_val ) / state->cur_scale_y;

}


// Расчет параметров прямоугольной координатной сетки.
void gp_ica_state_get_axis_step( gdouble scale, gdouble step_width, gdouble *from, gdouble *step, gint *range,
                                 gint *power )
{

  gdouble step_length;

  gdouble axis_1_width_delta;
  gdouble axis_2_width_delta;
  gdouble axis_5_width_delta;

  gdouble axis_1_score;
  gdouble axis_2_score;
  gdouble axis_5_score;

  gdouble from_ret;
  gdouble step_ret;
  gint range_ret;
  gint power_ret;

  step_length = scale * step_width;

  power_ret = 0;
  while( ( step_length > 10 ) || ( step_length < 1 ) )
    {
    if( step_length > 10 ) { step_length /= 10.0; power_ret = power_ret + 1; }
    if( step_length < 1 )  { step_length *= 10.0; power_ret = power_ret - 1; }
    }
  if( step_length > 5 ) { step_length /= 10.0; power_ret = power_ret + 1; }

  axis_1_width_delta = ( 1.0 * pow( 10.0, power_ret ) / scale ) - step_width;
  axis_2_width_delta = ( 2.0 * pow( 10.0, power_ret ) / scale ) - step_width;
  axis_5_width_delta = ( 5.0 * pow( 10.0, power_ret ) / scale ) - step_width;

  axis_1_score = ( axis_1_width_delta >= 0.0 ) ? 1.0 / axis_1_width_delta : -0.1 / axis_1_width_delta;
  axis_2_score = ( axis_2_width_delta >= 0.0 ) ? 1.0 / axis_2_width_delta : -0.1 / axis_2_width_delta;
  axis_5_score = ( axis_5_width_delta >= 0.0 ) ? 1.0 / axis_5_width_delta : -0.1 / axis_5_width_delta;

  if( axis_1_score > axis_2_score && axis_1_score > axis_5_score ) range_ret = 1;
  else if( axis_2_score > axis_5_score ) range_ret = 2;
  else range_ret = 5;

  step_ret = range_ret * pow( 10.0, power_ret );

  from_ret = step_ret * floor( *from / step_ret );
  if( from && from_ret < *from ) from_ret += step_ret;

  if( from != NULL ) *from = from_ret;
  if( step != NULL ) *step = step_ret;
  if( range != NULL ) *range = range_ret;
  if( power != NULL ) *power = power_ret;

}


// Выравнивание координат для использования в библиотеке cairo.
gdouble gp_ica_state_point_to_cairo( gdouble point )
{

  gdouble ipoint = (glong)point;

  return ( ( point - ipoint ) > 0.5 ) ? ipoint + 0.5 : ipoint - 0.5;

}


static void gp_ica_state_renderer_interface_init( GpIcaRendererInterface *iface )
{

  iface->get_renderers_num = gp_ica_state_renderer_get_renderers_num;
  iface->get_renderer_type = gp_ica_state_renderer_get_renderer_type;
  iface->get_name = gp_ica_state_renderer_get_name;

  iface->set_surface = NULL;
  iface->set_area_size = gp_ica_state_renderer_set_area_size;
  iface->set_visible_size = gp_ica_state_renderer_set_visible_size;

  iface->get_completion = NULL;
  iface->set_total_completion = gp_ica_state_renderer_set_total_completion;
  iface->render = NULL;
  iface->set_shown_limits = gp_ica_state_renderer_set_shown_limits;

  iface->set_border = gp_ica_state_renderer_set_border;
  iface->set_swap = gp_ica_state_renderer_set_swap;
  iface->set_angle = gp_ica_state_renderer_set_angle;
  iface->set_shown = gp_ica_state_renderer_set_shown;
  iface->set_pointer = gp_ica_state_renderer_set_pointer;

  iface->button_press_event = NULL;
  iface->button_release_event = NULL;
  iface->key_press_event = NULL;
  iface->key_release_event = NULL;
  iface->motion_notify_event = NULL;

}
