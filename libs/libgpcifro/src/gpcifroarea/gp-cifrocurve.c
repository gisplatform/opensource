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
 * \file cifrocurve.c
 *
 * \author Andrei Fadeev
 * \date 22.07.2014
 *
 * \brief Исходный файл GTK Widget параметрической кривой.
 *
*/

#include "gp-cifrocurve.h"


enum { PROP_0, PROP_CURVE_FUNC, PROP_CURVE_DATA };


typedef struct GpCifroCurvePriv {

  GpCifroCurveGravity gravity;           // Направление осей осциллографа.

  GpIcaCurveFunc curve_func;        // Функция расчёта значений кривой.
  gpointer              curve_data;        // Пользовательские данные для функции расчёта значений кривой.

  GpIcaCurve *curve;             // Объект рисования параметрической кривой.

} GpCifroCurvePriv;


#define GP_CIFRO_CURVE_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), GTK_TYPE_GP_CIFRO_CURVE, GpCifroCurvePriv ) )


static void gp_cifro_curve_set_property (GpCifroCurve *ccurve, guint prop_id, const GValue *value, GParamSpec *pspec);
static GObject *gp_cifro_curve_constructor (GType type, guint n_construct_properties,
                                           GObjectConstructParam *construct_properties);


G_DEFINE_TYPE(GpCifroCurve, gp_cifro_curve, GTK_TYPE_GP_CIFRO_SCOPE)

static void gp_cifro_curve_init (GpCifroCurve *ccurve){ ; }


// Настройка класса.
static void gp_cifro_curve_class_init (GpCifroCurveClass *klass)
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  g_type_class_add_private( klass, sizeof(GpCifroCurvePriv) );

  this_class->constructor = (void*) gp_cifro_curve_constructor;
  this_class->set_property = (void*) gp_cifro_curve_set_property;

  g_object_class_install_property( this_class, PROP_CURVE_FUNC,
    g_param_spec_pointer( "curve-func", "Curve func", "Curve function", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_CURVE_DATA,
    g_param_spec_pointer( "curve-data", "Curve data", "Curve function data", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

}


// Функция установки параметров.
static void gp_cifro_curve_set_property (GpCifroCurve *ccurve, guint prop_id, const GValue *value, GParamSpec *pspec)
{

  GpCifroCurvePriv *priv = GP_CIFRO_CURVE_GET_PRIVATE( ccurve );

  switch ( prop_id )
    {

    case PROP_CURVE_FUNC:
      priv->curve_func = g_value_get_pointer( value );
      break;

    case PROP_CURVE_DATA:
      priv->curve_data = g_value_get_pointer( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( ccurve, prop_id, pspec );
      break;

    }

}


static GObject*gp_cifro_curve_constructor (GType g_type, guint n_construct_properties,
                                           GObjectConstructParam *construct_properties)
{

  GObject *ccurve = G_OBJECT_CLASS( gp_cifro_curve_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( ccurve == NULL ) return NULL;

  GpCifroCurvePriv *priv = GP_CIFRO_CURVE_GET_PRIVATE( ccurve );

  GpCifroArea *carea = gp_cifro_scope_get_cifro_area (GP_CIFRO_SCOPE(ccurve));
  GpIcaStateRenderer *state_renderer = gp_cifro_scope_get_state_renderer (GP_CIFRO_SCOPE(ccurve));
  GpIcaCurve *curve = gp_ica_curve_new(state_renderer, priv->curve_func, priv->curve_data);
  gp_cifro_area_add_layer (carea, GP_ICA_RENDERER(curve));

  gp_cifro_scope_set_show_info (GP_CIFRO_SCOPE(ccurve), FALSE);

  priv->curve = curve;

  return ccurve;

}


GtkWidget *gp_cifro_curve_new (GpCifroCurveGravity gravity, guint n_channels, GpIcaCurveFunc curve_func,
                               gpointer curve_data)
{

  return g_object_new(GTK_TYPE_GP_CIFRO_CURVE, "gravity", gravity, "curve-func", curve_func, "curve-data", curve_data, "n_channels", n_channels, NULL );

}


void gp_cifro_curve_clear_points (GpCifroCurve *ccurve)
{

  GpCifroCurvePriv *priv = GP_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gp_ica_curve_clear_points(priv->curve);

}


void gp_cifro_curve_add_point (GpCifroCurve *ccurve, gdouble x, gdouble y)
{

  GpCifroCurvePriv *priv = GP_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gp_ica_curve_add_point(priv->curve, x, y);

}


void gp_cifro_curve_set_points (GpCifroCurve *ccurve, GArray *points)
{

  GpCifroCurvePriv *priv = GP_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gp_ica_curve_set_points(priv->curve, points);

}


GArray *gp_cifro_curve_get_points (GpCifroCurve *ccurve)
{

  GpCifroCurvePriv *priv = GP_CIFRO_CURVE_GET_PRIVATE( ccurve );

  return gp_ica_curve_get_points(priv->curve);

}


void gp_cifro_curve_set_curve_color (GpCifroCurve *ccurve, gdouble red, gdouble green, gdouble blue)
{

  GpCifroCurvePriv *priv = GP_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gp_ica_curve_set_curve_color(priv->curve, red, green, blue);

}


void gp_cifro_curve_set_point_color (GpCifroCurve *ccurve, gdouble red, gdouble green, gdouble blue)
{

  GpCifroCurvePriv *priv = GP_CIFRO_CURVE_GET_PRIVATE( ccurve );

  gp_ica_curve_set_point_color(priv->curve, red, green, blue);

}
