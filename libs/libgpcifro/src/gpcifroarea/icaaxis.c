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
 * \file icaaxis.c
 *
 * \author Andrei Fadeev
 * \date 16.07.2014
 * \brief Исходный файл класса отображения осей.
 *
*/

#include "icaaxis.h"

#include <math.h>
#include <glib/gprintf.h>


void gp_ica_axis_draw_axis( cairo_sdline_surface *surface, GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info )
{

  const GpIcaState *state = gp_ica_state_renderer_get_state( state_renderer );

  gdouble step = 3 * state->border_top;

  gdouble axis;
  gdouble axis_pos;
  gdouble axis_step;

  axis = state->from_x;
  gp_ica_state_get_axis_step( state->cur_scale_x, step, &axis, &axis_step, NULL, NULL);
  while( axis <= state->to_x )
    {
      gp_ica_state_renderer_visible_value_to_point( state_renderer, &axis_pos, NULL, axis, 0.0 );
    cairo_sdline_v( surface, axis_pos, 0, surface->height, axis_info->axis_color );
    axis += axis_step;
    }

  gp_ica_state_renderer_visible_value_to_point( state_renderer, &axis_pos, NULL, 0.0, 0.0 );
  cairo_sdline_v( surface, axis_pos, 0, surface->height, axis_info->zero_axis_color );

  axis = state->from_y;
  gp_ica_state_get_axis_step( state->cur_scale_y, step, &axis, &axis_step, NULL, NULL);
  while( axis <= state->to_y )
    {
      gp_ica_state_renderer_visible_value_to_point( state_renderer, NULL, &axis_pos, 0.0, axis );
    cairo_sdline_h( surface, 0, surface->width, axis_pos, axis_info->axis_color );
    axis += axis_step;
    }

  gp_ica_state_renderer_visible_value_to_point( state_renderer, NULL, &axis_pos, 0.0, 0.0 );
  cairo_sdline_h( surface, 0, surface->width, axis_pos, axis_info->zero_axis_color );

  cairo_surface_mark_dirty( surface->cairo_surface );

}


void gp_ica_axis_draw_hruler( cairo_sdline_surface *surface, PangoLayout *font_layout,
                              GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info )
{

  const GpIcaState *state = gp_ica_state_renderer_get_state( state_renderer );

  gdouble   step = 3 * state->border_top;

  gdouble   axis;
  gdouble   axis_to;
  gdouble   axis_pos;
  gdouble   axis_from;
  gdouble   axis_step;
  gdouble   axis_scale;
  gdouble   axis_mini_step;
  gint      axis_range;
  gint      axis_power;
  gint      axis_height;
  gint      axis_count;

  gchar     text_format[128];
  gchar     text_str[ 128 ];

  gint      text_width;
  gint      text_height;

  gboolean  swap = FALSE;

  if( state->angle < -0.01 || state->angle > G_PI / 2.0 + 0.01 ) return;
  if( fabs( state->angle - G_PI / 2.0 ) < G_PI / 4.0 ) swap = TRUE;

  axis_to = swap ? state->to_y : state->to_x;
  axis_from = swap ? state->from_y : state->from_x;
  axis_scale = swap ? state->cur_scale_y : state->cur_scale_x;

  gp_ica_state_get_axis_step( axis_scale, step, &axis_from, &axis_step, &axis_range, &axis_power );

  if( axis_range == 1 ) axis_range = 10;
  if( axis_range == 2 ) axis_mini_step = axis_step / 2.0;
  else if( axis_range == 5 ) axis_mini_step = axis_step / 5.0;
  else axis_mini_step = axis_step / 10.0;

  axis_count = 0;
  axis = axis_from - axis_step;
  while( axis <= axis_to )
    {

    if( swap )
      gp_ica_state_renderer_area_value_to_point( state_renderer, &axis_pos, NULL, state->from_x, axis );
    else
      gp_ica_state_renderer_area_value_to_point( state_renderer, &axis_pos, NULL, axis, 0.0 );

    if( axis_count % axis_range == 0 ) axis_height = state->border_top / 3.5;
    else axis_height = state->border_top / 7;
    axis_count += 1;

    axis += axis_mini_step;
    if( axis_pos <= state->border_left + 1 ) continue;
    if( axis_pos >= state->area_width - state->border_right - 1 ) continue;

    cairo_sdline_v( surface, axis_pos, state->border_top - axis_height, state->border_top, axis_info->zero_axis_color );


    }

  cairo_surface_mark_dirty( surface->cairo_surface );

  if( axis_power > 0 ) axis_power = 0;
  g_sprintf( text_format, "%%.%df", (gint)fabs( axis_power ) );

  cairo_sdline_set_cairo_color( surface, axis_info->text_color );
  cairo_set_line_width( surface->cairo, 1.0 );

  axis = axis_from;
  while( axis <= axis_to )
    {

    g_sprintf( text_str, text_format, axis );
    pango_layout_set_text( font_layout, text_str, -1 );
    pango_layout_get_size( font_layout, &text_width, &text_height );
    text_width /= PANGO_SCALE;
    text_height /= PANGO_SCALE;

    if( swap )
      gp_ica_state_renderer_area_value_to_point( state_renderer, &axis_pos, NULL, state->from_x, axis );
    else
      gp_ica_state_renderer_area_value_to_point( state_renderer, &axis_pos, NULL, axis, 0.0 );
    axis_pos -= text_width / 2;
    axis += axis_step;

    if( axis_pos < state->border_left + 1 ) continue;
    if( axis_pos + text_width > state->area_width - state->border_right - 1 ) continue;

    cairo_move_to( surface->cairo, axis_pos, ( ( 0.75 * state->border_top ) - text_height ) / 2.0 );
    pango_cairo_show_layout( surface->cairo, font_layout );

    }

  if( swap )
    pango_layout_set_text( font_layout, axis_info->y_axis_name, -1 );
  else
    pango_layout_set_text( font_layout, axis_info->x_axis_name, -1 );

  pango_layout_get_size( font_layout, &text_width, &text_height );
  text_width /= PANGO_SCALE;
  text_height /= PANGO_SCALE;

  cairo_move_to( surface->cairo, state->area_width - state->border_right / 2  - text_width / 2, state->border_top / 2 - text_height / 2 );
  pango_cairo_show_layout( surface->cairo, font_layout );
  cairo_surface_flush( surface->cairo_surface );

}


void gp_ica_axis_draw_vruler( cairo_sdline_surface *surface, PangoLayout *font_layout,
                              GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info )
{

  const GpIcaState *state = gp_ica_state_renderer_get_state( state_renderer );

  gdouble   step = 3 * state->border_left;

  gdouble   axis;
  gdouble   axis_to;
  gdouble   axis_pos;
  gdouble   axis_from;
  gdouble   axis_step;
  gdouble   axis_scale;
  gdouble   axis_mini_step;
  gint      axis_range;
  gint      axis_power;
  gint      axis_height;
  gint      axis_count;

  gboolean  axis_swap = FALSE;

  gchar     text_format[ 128 ];
  gchar     text_str[ 128 ];

  gint      text_width;
  gint      text_height;

  if( state->angle < -0.01 || state->angle > G_PI / 2.0 + 0.01 ) return;
  if( fabs( state->angle - G_PI / 2.0 ) < G_PI / 4.0 ) axis_swap = TRUE;

  axis_to = axis_swap ? state->to_x : state->to_y;
  axis_from = axis_swap ? state->from_x : state->from_y;
  axis_scale = axis_swap ? state->cur_scale_x : state->cur_scale_y;

  gp_ica_state_get_axis_step( axis_scale, step, &axis_from, &axis_step, &axis_range, &axis_power );

  if( axis_range == 1 ) axis_range = 10;
  if( axis_range == 2 ) axis_mini_step = axis_step / 2.0;
  else if( axis_range == 5 ) axis_mini_step = axis_step / 5.0;
  else axis_mini_step = axis_step / 10.0;

  axis_count = 0;
  axis = axis_from - axis_step;
  while( axis <= axis_to )
    {

    if( axis_swap )
      gp_ica_state_renderer_area_value_to_point( state_renderer, NULL, &axis_pos, axis, state->to_y );
    else
      gp_ica_state_renderer_area_value_to_point( state_renderer, NULL, &axis_pos, 0.0, axis );

    if( axis_count % axis_range == 0 ) axis_height = state->border_top / 3.5;
    else axis_height = state->border_top / 7;
    axis_count += 1;

    axis += axis_mini_step;
    if( axis_pos <= state->border_top + 1) continue;
    if( axis_pos >= state->area_height - state->border_bottom - 1 ) continue;

    cairo_sdline_h( surface, state->border_left - axis_height, state->border_left, axis_pos, axis_info->zero_axis_color );

    }

  cairo_surface_mark_dirty( surface->cairo_surface );

  if( axis_power > 0 ) axis_power = 0;
  g_sprintf( text_format, "%%.%df", (gint)fabs( axis_power ) );

  cairo_sdline_set_cairo_color( surface, axis_info->text_color );
  cairo_set_line_width( surface->cairo, 1.0 );

  axis = axis_from;
  while( axis <= axis_to )
    {

    g_sprintf( text_str, text_format, axis );
    pango_layout_set_text( font_layout, text_str, -1 );
    pango_layout_get_size( font_layout, &text_width, &text_height );
    text_width /= PANGO_SCALE;
    text_height /= PANGO_SCALE;

    if( axis_swap )
      gp_ica_state_renderer_area_value_to_point( state_renderer, NULL, &axis_pos, axis, state->to_y );
    else
      gp_ica_state_renderer_area_value_to_point( state_renderer, NULL, &axis_pos, 0.0, axis );
    axis_pos += text_width / 2;
    axis += axis_step;

    if( axis_pos - text_width <= state->border_top + 1) continue;
    if( axis_pos >= state->area_height - state->border_bottom - 1 ) continue;

    cairo_save( surface->cairo );
    cairo_move_to( surface->cairo, 0, axis_pos );
    cairo_rotate( surface->cairo, - G_PI / 2.0 );
    pango_cairo_show_layout( surface->cairo, font_layout );
    cairo_restore( surface->cairo );

    }

  if( axis_swap )
    pango_layout_set_text( font_layout, axis_info->x_axis_name, -1 );
  else
    pango_layout_set_text( font_layout, axis_info->y_axis_name, -1 );
  pango_layout_get_size( font_layout, &text_width, &text_height );
  text_width /= PANGO_SCALE;
  text_height /= PANGO_SCALE;

  cairo_move_to( surface->cairo, state->border_left / 2  - text_width / 2, state->area_height - state->border_top / 2 - text_height / 2 );
  pango_cairo_show_layout( surface->cairo, font_layout );
  cairo_surface_flush( surface->cairo_surface );

}


void gp_ica_axis_draw_x_pos( cairo_sdline_surface *surface, GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info )
{

  const GpIcaState *state = gp_ica_state_renderer_get_state( state_renderer );

  gdouble   from;
  gdouble   to;

  gint      x;
  gint      y;
  gint      width;
  gint      height;

  gboolean  axis_swap = FALSE;

  if( fabs( state->angle - G_PI / 2.0 ) < G_PI / 4.0 ) axis_swap = TRUE;

  if( axis_swap )
    {
    from = ( state->from_y - state->min_y ) / ( state->max_y - state->min_y );
    to = ( state->to_y - state->min_y ) / ( state->max_y - state->min_y );
    if( state->swap_y ) { from = 1.0 - from; to = 1.0 - to; }
    }
  else
    {
    from = ( state->from_x - state->min_x ) / ( state->max_x - state->min_x );
    to = ( state->to_x - state->min_x ) / ( state->max_x - state->min_x );
    if( state->swap_x ) { from = 1.0 - from; to = 1.0 - to; }
    }

  x = state->border_left;
  y = state->area_height - 0.75 * state->border_bottom;
  width = state->area_width - state->border_left - state->border_right;
  height = state->border_bottom / 2;

  cairo_sdline_bar( surface, x, y, x + width, y + height, axis_info->axis_color );
  cairo_sdline_bar( surface, x + from * width, y, x + to * width, y + height, axis_info->zero_axis_color );

}


void gp_ica_axis_draw_y_pos( cairo_sdline_surface *surface, GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info )
{

  const GpIcaState *state = gp_ica_state_renderer_get_state( state_renderer );

  gdouble   from;
  gdouble   to;

  gint      x;
  gint      y;
  gint      width;
  gint      height;

  gboolean  axis_swap = FALSE;

  if( fabs( state->angle - G_PI / 2.0 ) < G_PI / 4.0 ) axis_swap = TRUE;

  if( axis_swap )
    {
    from = 1.0 - ( state->from_x - state->min_x ) / ( state->max_x - state->min_x );
    to = 1.0 - ( state->to_x - state->min_x ) / ( state->max_x - state->min_x );
    if( !state->swap_x ) { from = 1.0 - from; to = 1.0 - to; }
    }
  else
    {
    from = 1.0 - ( state->from_y - state->min_y ) / ( state->max_y - state->min_y );
    to = 1.0 - ( state->to_y - state->min_y ) / ( state->max_y - state->min_y );
    if( state->swap_y ) { from = 1.0 - from; to = 1.0 - to; }
    }

  x = state->area_width - 0.75 * state->border_left;
  y = state->border_top;
  width = state->border_left / 2;
  height = state->area_height - state->border_top - state->border_bottom;

  cairo_sdline_bar( surface, x, y, x + width, y + height, axis_info->axis_color );
  cairo_sdline_bar( surface, x, y + from * height, x + width, y + to * height, axis_info->zero_axis_color );

}
