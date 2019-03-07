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
 * \file icarenderer.c
 *
 * \author Andrei Fadeev
 * \date 5.04.2013
 * \brief Исходный файл интерфейса формирования изображений.
 *
*/

#include "gp-icarenderer.h"


G_DEFINE_INTERFACE(GpIcaRenderer, gp_ica_renderer, G_TYPE_OBJECT);

static void gp_ica_renderer_default_init( GpIcaRendererInterface *iface ){ }


gint gp_ica_renderer_get_renderers_num(GpIcaRenderer *renderer)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->get_renderers_num != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->get_renderers_num( renderer );

  return 0;

}


const gchar *gp_ica_renderer_get_name(GpIcaRenderer *renderer, gint renderer_id)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->get_name != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->get_name( renderer, renderer_id );

  return "Unknown renderer name";

}


GpIcaRendererType gp_ica_renderer_get_renderer_type(GpIcaRenderer *renderer, gint renderer_id)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->get_renderer_type != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->get_renderer_type( renderer, renderer_id );

  return GP_ICA_RENDERER_UNKNOWN;

}


void gp_ica_renderer_set_surface(GpIcaRenderer *renderer, gint renderer_id, guchar *data, gint width, gint height,
                                 gint stride)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_surface != NULL )
    GP_ICA_RENDERER_GET_CLASS( renderer )->set_surface( renderer, renderer_id, data, width, height, stride );

}


void gp_ica_renderer_set_area_size(GpIcaRenderer *renderer, gint width, gint height)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_area_size != NULL )
    GP_ICA_RENDERER_GET_CLASS( renderer )->set_area_size( renderer, width, height );

}


void gp_ica_renderer_set_visible_size(GpIcaRenderer *renderer, gint width, gint height)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_visible_size != NULL )
    GP_ICA_RENDERER_GET_CLASS( renderer )->set_visible_size( renderer, width, height );

}


gint gp_ica_renderer_get_completion(GpIcaRenderer *renderer, gint renderer_id)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->get_completion != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->get_completion( renderer, renderer_id );

  return -1;

}


void gp_ica_renderer_set_total_completion(GpIcaRenderer *renderer, gint completion)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_total_completion != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->set_total_completion( renderer, completion );

}


GpIcaRendererAvailability gp_ica_renderer_render(GpIcaRenderer *renderer, gint renderer_id, gint *x, gint *y,
                                                 gint *width, gint *height)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->render != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->render( renderer, renderer_id, x, y, width, height );

  return GP_ICA_RENDERER_AVAIL_NONE;

}


void gp_ica_renderer_set_shown_limits(GpIcaRenderer *renderer, gdouble min_x, gdouble max_x, gdouble min_y,
                                      gdouble max_y)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_shown_limits != NULL )
    GP_ICA_RENDERER_GET_CLASS( renderer )->set_shown_limits( renderer, min_x, max_x, min_y, max_y );

}


void gp_ica_renderer_set_border(GpIcaRenderer *renderer, gint left, gint right, gint top, gint bottom)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_border != NULL )
    GP_ICA_RENDERER_GET_CLASS( renderer )->set_border( renderer, left, right, top, bottom );

}


void gp_ica_renderer_set_swap(GpIcaRenderer *renderer, gboolean swap_x, gboolean swap_y)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_swap != NULL )
    GP_ICA_RENDERER_GET_CLASS( renderer )->set_swap( renderer, swap_x, swap_y );

}


void gp_ica_renderer_set_angle(GpIcaRenderer *renderer, gdouble angle)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_angle != NULL )
    GP_ICA_RENDERER_GET_CLASS( renderer )->set_angle( renderer, angle );

}


void gp_ica_renderer_set_shown(GpIcaRenderer *renderer, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_shown != NULL )
    GP_ICA_RENDERER_GET_CLASS( renderer )->set_shown( renderer, from_x, to_x, from_y, to_y );

}


void gp_ica_renderer_set_pointer(GpIcaRenderer *renderer, gint pointer_x, gint pointer_y, gdouble value_x,
                                 gdouble value_y)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->set_pointer != NULL )
    GP_ICA_RENDERER_GET_CLASS( renderer )->set_pointer( renderer, pointer_x, pointer_y, value_x, value_y );

}


gboolean gp_ica_renderer_button_press_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->button_press_event != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->button_press_event( renderer, event, widget );
  else
    return FALSE;

}


gboolean gp_ica_renderer_button_release_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->button_release_event != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->button_release_event( renderer, event, widget );
  else
    return FALSE;

}


gboolean gp_ica_renderer_key_press_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->key_press_event != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->key_press_event( renderer, event, widget );
  else
    return FALSE;

}


gboolean gp_ica_renderer_key_release_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->key_release_event != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->key_release_event( renderer, event, widget );
  else
    return FALSE;

}


gboolean gp_ica_renderer_motion_notify_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget)
{

  if( GP_ICA_RENDERER_GET_CLASS( renderer )->motion_notify_event != NULL )
    return GP_ICA_RENDERER_GET_CLASS( renderer )->motion_notify_event( renderer, event, widget );
  else
    return FALSE;

}

