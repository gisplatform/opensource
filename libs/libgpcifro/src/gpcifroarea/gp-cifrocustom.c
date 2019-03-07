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
 * \file cifropoint.c
 *
 * \author Andrei Vodilov
 * \date 01.02.2016
 *
 * \brief Исходный файл GTK Widget для отображения пользовательской информации (точки, метки, линии).
 *
*/

#include "gp-cifrocustom.h"


enum { PROP_0, PROP_POINT_DATA };


typedef struct GpCifroCustomPriv {

  GpCifroCustomGravity   gravity;           // Направление осей осциллографа.
  gpointer               data;        // Пользовательские данные.
  GpIcaCustom            *icacustom;            // Объект рисования параметрической кривой.

} GpCifroCustomPriv;


#define GP_CIFRO_CUSTOM_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), GTK_TYPE_GP_CIFRO_CUSTOM, GpCifroCustomPriv ) )


static void gp_cifro_custom_set_property (GpCifroCustom *self, guint prop_id, const GValue *value, GParamSpec *pspec);
static GObject *gp_cifro_custom_constructor (GType type, guint n_construct_properties,
                                           GObjectConstructParam *construct_properties);


G_DEFINE_TYPE(GpCifroCustom, gp_cifro_custom, GTK_TYPE_GP_CIFRO_SCOPE)

static void gp_cifro_custom_init (GpCifroCustom *self){ ; }


// Настройка класса.
static void gp_cifro_custom_class_init (GpCifroCustomClass *klass)
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  g_type_class_add_private( klass, sizeof(GpCifroCustomPriv) );

  this_class->constructor = (void*) gp_cifro_custom_constructor;
  this_class->set_property = (void*) gp_cifro_custom_set_property;

  g_object_class_install_property( this_class, PROP_POINT_DATA,
    g_param_spec_pointer( "point-data", "Point data", "Point function data", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

}


// Функция установки параметров.
static void gp_cifro_custom_set_property (GpCifroCustom *self, guint prop_id, const GValue *value, GParamSpec *pspec)
{

  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  switch ( prop_id )
    {
    case PROP_POINT_DATA:
      priv->data = g_value_get_pointer( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( self, prop_id, pspec );
      break;

    }
}


static GObject*gp_cifro_custom_constructor (GType g_type, guint n_construct_properties,
                                           GObjectConstructParam *construct_properties)
{

  GObject *self = G_OBJECT_CLASS( gp_cifro_custom_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( self == NULL ) return NULL;

  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  GpCifroArea *carea = gp_cifro_scope_get_cifro_area (GP_CIFRO_SCOPE(self));
  GpIcaStateRenderer *state_renderer = gp_cifro_scope_get_state_renderer (GP_CIFRO_SCOPE(self));
  GpIcaCustom *icacustom = gp_ica_custom_new(state_renderer, priv->data);
  gp_cifro_area_add_layer (carea, GP_ICA_RENDERER(icacustom));

  gp_cifro_scope_set_show_info (GP_CIFRO_SCOPE(self), FALSE);

  priv->icacustom = icacustom;

  return self;

}


GtkWidget *gp_cifro_custom_new (GpCifroCustomGravity gravity, guint n_channels, gpointer point_data)
{

  return g_object_new(GTK_TYPE_GP_CIFRO_CUSTOM, "gravity", gravity, "point-data", point_data, "n_channels", n_channels, NULL );

}


void gp_cifro_custom_clear_points (GpCifroCustom *self)
{

  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  gp_ica_custom_clear_points(priv->icacustom);

}


void gp_cifro_custom_add_point (GpCifroCustom *self, gdouble x, gdouble y)
{

  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  gp_ica_custom_add_point(priv->icacustom, x, y);

}


void gp_cifro_custom_set_points (GpCifroCustom *self, GArray *points)
{

  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  gp_ica_custom_set_points(priv->icacustom, points);

}


GArray *gp_cifro_custom_get_points (GpCifroCustom *self)
{

  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  return gp_ica_custom_get_points(priv->icacustom);

}


void gp_cifro_custom_set_line_color (GpCifroCustom *self, gdouble red, gdouble green, gdouble blue)
{

  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  gp_ica_custom_set_line_color(priv->icacustom, red, green, blue);

}


void gp_cifro_custom_set_points_color (GpCifroCustom *self, gdouble red, gdouble green, gdouble blue)
{

  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  gp_ica_custom_set_points_color(priv->icacustom, red, green, blue);

}

void gp_cifro_custom_set_draw_type(GpCifroCustom *self, GpIcaCustomDrawType new_type)
{
  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  gp_ica_custom_set_draw_type(priv->icacustom, new_type);
}

GpIcaCustomDrawType gp_cifro_custom_get_draw_type(GpCifroCustom *self)
{
  GpCifroCustomPriv *priv = GP_CIFRO_CUSTOM_GET_PRIVATE( self );

  return gp_ica_custom_get_draw_type(priv->icacustom);
}
