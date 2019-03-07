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
 * \file cifroscope.c
 *
 * \author Andrei Fadeev
 * \date 16.07.2014
 *
 * \brief Исходный файл GTK+ виджета осциллограф.
 *
*/

#include "gp-cifroscope.h"

#include "gp-cifroarea.h"
#include "gp-icastaterenderer.h"
#include "gp-icascope.h"
#include "cairosdline.h"


enum {
  PROP_0,
  PROP_GRAVITY,
  PROP_N_CHANNELS,
  PROP_GROUND_COLOR,
  PROP_BORDER_COLOR,
  PROP_AXIS_COLOR,
  PROP_ZERO_AXIS_COLOR,
  PROP_TEXT_COLOR
 };


typedef struct GpCifroScopePriv {

  GpCifroScopeGravity gravity;           // Направление осей осциллографа.

  guint                 n_channels;        // Число каналов осциллографа.

  GpIcaStateRenderer *state_renderer;    // Состояние GpCifroArea.
  GpCifroArea *carea;             // Объект GpCifroArea.
  GpIcaScope *scope;             // Объект рисования осциллограмм.

} GpCifroScopePriv;


#define GP_CIFRO_SCOPE_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), GTK_TYPE_GP_CIFRO_SCOPE, GpCifroScopePriv ) )


static void gp_cifro_scope_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static GObject *gp_cifro_scope_constructor (GType type, guint n_construct_properties,
                                           GObjectConstructParam *construct_properties);

static void gp_cifro_scope_size_request (GtkWidget *widget, GpCifroScopePriv *priv);


G_DEFINE_TYPE(GpCifroScope, gp_cifro_scope, GTK_TYPE_BOX)

static void gp_cifro_scope_init (GpCifroScope *cscope){ ; }


// Настройка класса.
static void gp_cifro_scope_class_init (GpCifroScopeClass *klass)
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  g_type_class_add_private( klass, sizeof(GpCifroScopePriv) );

  this_class->constructor = (void*) gp_cifro_scope_constructor;
  this_class->set_property = gp_cifro_scope_set_property;

  g_object_class_install_property( this_class, PROP_GRAVITY,
    g_param_spec_uint( "gravity", "Gravity", "GpCifroScope axis direction", GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP,
                       GP_CIFRO_SCOPE_GRAVITY_DOWN_LEFT,
                       GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_N_CHANNELS,
    g_param_spec_uint( "n_channels", "Number of channels", "Number of cifroscope channels", 1, 64, 1, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_GROUND_COLOR,
    g_param_spec_uint( "ground-color", "Ground color", "Ground color", 0, G_MAXUINT32, 0xFF000000, G_PARAM_WRITABLE ) );

  g_object_class_install_property( this_class, PROP_BORDER_COLOR,
    g_param_spec_uint( "border-color", "Border color", "Border color", 0, G_MAXUINT32, 0xFFA8A8A8, G_PARAM_WRITABLE ) );

  g_object_class_install_property( this_class, PROP_AXIS_COLOR,
    g_param_spec_uint( "axis-color", "Axis color", "Axis color", 0, G_MAXUINT32, 0xFF333333, G_PARAM_WRITABLE ) );

  g_object_class_install_property( this_class, PROP_ZERO_AXIS_COLOR,
    g_param_spec_uint( "zero-axis-color", "Zero axis color", "Zero axis color", 0, G_MAXUINT32, 0xFF808080, G_PARAM_WRITABLE ) );

  g_object_class_install_property( this_class, PROP_TEXT_COLOR,
    g_param_spec_uint( "text-color", "Text color", "Text color", 0, G_MAXUINT32, 0xFFA8A8A8, G_PARAM_WRITABLE ) );

}


// Функция установки параметров.
static void gp_cifro_scope_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE(GP_CIFRO_SCOPE(object));

  switch ( prop_id )
  {
    case PROP_GRAVITY:
      priv->gravity = g_value_get_uint( value );
    break;

    case PROP_N_CHANNELS:
      priv->n_channels = g_value_get_uint( value );
    break;

    case PROP_GROUND_COLOR:
      g_object_set_property(G_OBJECT(priv->scope), "ground-color", value);
    break;

    case PROP_BORDER_COLOR:
      g_object_set_property(G_OBJECT(priv->scope), "border-color", value);
    break;

    case PROP_AXIS_COLOR:
      g_object_set_property(G_OBJECT(priv->scope), "axis-color", value);
    break;

    case PROP_ZERO_AXIS_COLOR:
      g_object_set_property(G_OBJECT(priv->scope), "zero-axis-color", value);
    break;

    case PROP_TEXT_COLOR:
      g_object_set_property(G_OBJECT(priv->scope), "text-color", value);
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    break;
  }
}


static GObject*gp_cifro_scope_constructor (GType g_type, guint n_construct_properties,
                                           GObjectConstructParam *construct_properties)
{

  GObject *cscope = G_OBJECT_CLASS( gp_cifro_scope_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( cscope == NULL ) return NULL;

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  GpCifroArea *carea = GP_CIFRO_AREA(gp_cifro_area_new (40) );
  GpIcaStateRenderer *state_renderer = gp_ica_state_renderer_new();
  GpIcaScope *scope = gp_ica_scope_new(state_renderer, priv->n_channels);

  gp_cifro_area_add_layer (carea, GP_ICA_RENDERER(state_renderer));
  gp_cifro_area_add_layer (carea, GP_ICA_RENDERER(scope));

  // Устанавливаем направления осей.
  switch( priv->gravity )
    {

    case GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP:
      gp_cifro_area_set_angle (carea, 0.0);
        gp_cifro_area_set_swap (carea, FALSE, FALSE);
      break;

    case GP_CIFRO_SCOPE_GRAVITY_LEFT_UP:
      gp_cifro_area_set_angle (carea, 0.0);
        gp_cifro_area_set_swap (carea, TRUE, FALSE);
      break;

    case GP_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN:
      gp_cifro_area_set_angle (carea, 0.0);
        gp_cifro_area_set_swap (carea, FALSE, TRUE);
      break;

    case GP_CIFRO_SCOPE_GRAVITY_LEFT_DOWN:
      gp_cifro_area_set_angle (carea, 0.0);
        gp_cifro_area_set_swap (carea, TRUE, TRUE);
      break;

    case GP_CIFRO_SCOPE_GRAVITY_UP_RIGHT:
      gp_cifro_area_set_angle (carea, G_PI / 2.0);
        gp_cifro_area_set_swap (carea, TRUE, FALSE);
      break;

    case GP_CIFRO_SCOPE_GRAVITY_UP_LEFT:
      gp_cifro_area_set_angle (carea, G_PI / 2.0);
        gp_cifro_area_set_swap (carea, TRUE, TRUE);
      break;

    case GP_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT:
      gp_cifro_area_set_angle (carea, G_PI / 2.0);
        gp_cifro_area_set_swap (carea, FALSE, FALSE);
      break;

    case GP_CIFRO_SCOPE_GRAVITY_DOWN_LEFT:
      gp_cifro_area_set_angle (carea, G_PI / 2.0);
        gp_cifro_area_set_swap (carea, FALSE, TRUE);
      break;

    default: break;

    }

  gp_cifro_area_set_shown_limits (carea, -100, 100, -100, 100);
  gp_cifro_area_set_scale_limits (carea, 0.01, 100.0, 0.01, 100.0);
  gp_cifro_area_set_scale_on_resize (carea, TRUE);
  gp_cifro_area_set_rotation (carea, FALSE);
  gp_cifro_area_set_scale_aspect (carea, -1.0);
  gp_cifro_area_set_shown (carea, -100, 100, -100, 100);
  gp_cifro_area_set_zoom_on_center (carea, FALSE);
  gp_cifro_area_set_zoom_scale (carea, 10);

  g_signal_connect( cscope, "realize", G_CALLBACK (gp_cifro_scope_size_request), priv );

  gtk_box_pack_start( GTK_BOX( cscope ), GTK_WIDGET( carea ), TRUE, TRUE, 0 );

  priv->state_renderer = state_renderer;
  priv->carea = carea;
  priv->scope = scope;

  return cscope;

}


static void gp_cifro_scope_size_request (GtkWidget *widget, GpCifroScopePriv *priv)
{

  gchar *font_name;
  PangoFontDescription *font_desc;
  cairo_sdline_surface *font_surface;
  PangoLayout *font_layout;

  gint border;

  // Расчитываем размер обрамления в зависимости от размера шрифта по умолчанию.
  g_object_get( gtk_settings_get_default(), "gtk-font-name", &font_name, NULL );
  font_desc = pango_font_description_from_string( font_name );
  font_surface = cairo_sdline_surface_create( 1024, 1024 );
  font_layout = pango_cairo_create_layout( font_surface->cairo );
  pango_layout_set_font_description( font_layout, font_desc );
  pango_layout_set_text( font_layout, "0123456789ABCDEFGHIJKLMNOPQRSTUWXYZ.,", -1 );
  pango_layout_get_size( font_layout, NULL, &border );
  g_object_unref( font_layout );
  cairo_sdline_surface_destroy( font_surface );
  pango_font_description_free( font_desc );
  g_free( font_name );

  border *= 1.4;
  border /= PANGO_SCALE;

  gp_ica_scope_configure( priv->scope );

  gp_cifro_area_set_border (priv->carea, border, border, border, border);
  gp_cifro_area_set_minimum_visible (priv->carea, 3 * border, 3 * border);

}


GtkWidget *gp_cifro_scope_new (GpCifroScopeGravity gravity, guint n_channels)
{

  return g_object_new(GTK_TYPE_GP_CIFRO_SCOPE, "gravity", gravity, "n_channels", n_channels, NULL );

}


void gp_cifro_scope_set_view (GpCifroScope *cscope,
                              const gchar *time_axis_name, gdouble time_min, gdouble time_max,
                              const gchar *value_axis_name, gdouble value_min, gdouble value_max)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gp_cifro_area_set_shown_limits (priv->carea, time_min, time_max, value_min, value_max);
  gp_ica_scope_set_axis_name( priv->scope, time_axis_name, value_axis_name );

}


void gp_cifro_scope_set_show_info (GpCifroScope *cscope, gboolean show)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gp_ica_scope_set_show_info( priv->scope, show );

}


void gp_cifro_scope_set_channel_time_param (GpCifroScope *cscope, gint channel, gfloat time_shift, gfloat time_step)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gint i;

  if( channel < 0 )
    for( i = 0; i < priv->n_channels; i++ )
      gp_ica_scope_set_channel_time_param( priv->scope, i, time_shift, time_step );
  else
    gp_ica_scope_set_channel_time_param( priv->scope, channel, time_shift, time_step );

}


void gp_cifro_scope_set_channel_value_param (GpCifroScope *cscope, gint channel, gfloat value_shift, gfloat value_scale)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gint i;

  if( channel < 0 )
    for( i = 0; i < priv->n_channels; i++ )
      gp_ica_scope_set_channel_value_param( priv->scope, i, value_shift, value_scale );
  else
    gp_ica_scope_set_channel_value_param( priv->scope, channel, value_shift, value_scale );

}


void gp_cifro_scope_set_channel_draw_type (GpCifroScope *cscope, gint channel, GpCifroScopeDrawType draw_type)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gint i;

  if( channel < 0 )
    for( i = 0; i < priv->n_channels; i++ )
      gp_ica_scope_set_channel_draw_type( priv->scope, i, draw_type );
  else
    gp_ica_scope_set_channel_draw_type( priv->scope, channel, draw_type );

}


void gp_cifro_scope_set_channel_color (GpCifroScope *cscope, gint channel, gdouble red, gdouble green, gdouble blue)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gint i;

  if( channel < 0 )
    for( i = 0; i < priv->n_channels; i++ )
      gp_ica_scope_set_channel_color( priv->scope, i, red, green, blue );
  else
    gp_ica_scope_set_channel_color( priv->scope, channel, red, green, blue );

}


void gp_cifro_scope_set_show_channel (GpCifroScope *cscope, gint channel, gboolean show)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gint i;

  if( channel < 0 )
    for( i = 0; i < priv->n_channels; i++ )
      gp_ica_scope_set_show_channel( priv->scope, i, show );
  else
    gp_ica_scope_set_show_channel( priv->scope, channel, show );

}


void gp_cifro_scope_set_channel_axis_name (GpCifroScope *cscope, gint channel, const gchar *axis_name)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gint i;

  if( channel < 0 )
    for( i = 0; i < priv->n_channels; i++ )
      gp_ica_scope_set_channel_axis_name( priv->scope, i, axis_name );
  else
    gp_ica_scope_set_channel_axis_name( priv->scope, channel, axis_name );

}


gboolean  gp_cifro_scope_set_zoom_scale (GpCifroScope *cscope, gdouble min_scale_time, gdouble max_scale_time,
                                         gdouble min_scale_value, gdouble max_scale_value)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  return gp_cifro_area_set_scale_limits (priv->carea, min_scale_time, max_scale_time, min_scale_value, max_scale_value);

}


gboolean gp_cifro_scope_set_shown (GpCifroScope *cscope, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  return gp_cifro_area_set_shown (priv->carea, from_x, to_x, from_y, to_y);

}


void gp_cifro_scope_set_data (GpCifroScope *cscope, gint channel, gint num, gfloat *values)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gp_ica_scope_set_data( priv->scope, channel, num, values );

}


void gp_cifro_scope_update (GpCifroScope *cscope)
{

  GpCifroScopePriv *priv = GP_CIFRO_SCOPE_GET_PRIVATE( cscope );

  gp_ica_scope_update( priv->scope );

}


GpCifroArea *gp_cifro_scope_get_cifro_area (GpCifroScope *cscope)
{

  return GP_CIFRO_SCOPE_GET_PRIVATE( cscope )->carea;

}


GpIcaStateRenderer *gp_cifro_scope_get_state_renderer (GpCifroScope *cscope)
{

  return GP_CIFRO_SCOPE_GET_PRIVATE( cscope )->state_renderer;

}
