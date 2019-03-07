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
 * \file icascope.c
 *
 * \author Andrei Fadeev
 * \date 2.06.2014
 * \brief Исходный файл класса отображения осциллографа.
 *
*/

#include "gp-icascope.h"
#include "cairosdline.h"
#include "icaaxis.h"

#include <math.h>
#include <string.h>
#include <glib/gprintf.h>


enum {
  PROP_O,
  PROP_STATE_RENDERER,
  PROP_N_CHANNELS,
  PROP_GROUND_COLOR,
  PROP_BORDER_COLOR,
  PROP_AXIS_COLOR,
  PROP_ZERO_AXIS_COLOR,
  PROP_TEXT_COLOR
};

enum { SCOPE_SURFACE = 0, X_AXIS_SURFACE, X_POS_SURFACE, Y_AXIS_SURFACE, Y_POS_SURFACE, INFO_SURFACE, SURFACES_NUM };


typedef struct GpCifroScopeValues {

  gfloat               *data;              // Данные для отображения.
  gint                  size;              // Размер массива данных для отображения.

  gint                  num;               // Число данных для отображения.

  gfloat                time_shift;        // Смещение данных по времени.
  gfloat                time_step;         // Шаг времени.
  gfloat                value_shift;       // Коэффициент смещения данных.
  gfloat                value_scale;       // Коэффициент масштабирования данных.

  gboolean              show;              // "Выключатели" каналов осциллографа.
  guint32               color;             // Цвета данных канала.
  gchar                *name;              // Имя оси абсцисс канала.
  GpIcaScopeDrawType draw_type;         // Тип отображения осциллограмм.

} GpCifroScopeValues;


typedef struct GpIcaScopePriv {

  GpIcaStateRenderer *state_renderer;   // Псевдо отрисовщик для учёта состояния областей.

  guint                  n_channels;       // Число каналов осциллографа.
  GpCifroScopeValues **channels;         // Данные каналов осциллографа.

  gboolean               show_info;        // Показывать или нет информацию о значениях под курсором.

  GpIcaAxis *axis_info;        // Параметры отрисовки используемые для осциллографа.

  PangoFontDescription  *font_desc;        // Описание шрифта для Pango.

  cairo_sdline_surface  *scope_surface;    // Поверхность для рисования осциллограмм.

  cairo_sdline_surface  *x_axis_surface;   // Поверхность для оси абсцисс.
  cairo_sdline_surface  *y_axis_surface;   // Поверхность для оси ординат.

  PangoLayout           *x_axis_font;      // Раскладка шрифта для оси абсцисс.
  PangoLayout           *y_axis_font;      // Раскладка шрифта для оси ординат.

  cairo_sdline_surface  *x_pos_surface;    // Поверхность для отображения положения по оси абсцисс.
  cairo_sdline_surface  *y_pos_surface;    // Поверхность для отображения положения по оси ординат.

  cairo_sdline_surface  *info_surface;     // Поверхность для информационных сообщений.
  PangoLayout           *info_font;        // Раскладка шрифта для информационных сообщений.

  gboolean               scope_update;     // Признак необходимости перерисовки осциллограмм.
  gboolean               x_axis_update;    // Признак необходимости перерисовки оси абсцисс.
  gboolean               x_pos_update;     // Признак необходимости перерисовки местоположения по оси абсцисс.
  gboolean               y_axis_update;    // Признак необходимости перерисовки оси ординат.
  gboolean               y_pos_update;     // Признак необходимости перерисовки местоположения по оси ординат.
  gboolean               info_update;      // Признак необходимости перерисовки информационного сообщения.

  gboolean               colors_up_to_date;// Цвета настроены. Считаются настроенными, если были настроены автоматом
                                           // по данным из темы, либо если пользователь указал хотя бы один из цветов через свойства.
} GpIcaScopePriv;

#define GP_ICA_SCOPE_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), G_TYPE_GP_ICA_SCOPE, GpIcaScopePriv ) )


static void gp_ica_scope_interface_init( GpIcaRendererInterface *iface );
static void gp_ica_scope_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static GObject *gp_ica_scope_constructor( GType g_type, guint n_construct_properties,
                                         GObjectConstructParam *construct_properties );
static void gp_ica_scope_finalize(GObject *object);
static void gp_ica_scope_update_colors( GpIcaScope *scope );

G_DEFINE_TYPE_WITH_CODE(GpIcaScope, gp_ica_scope, G_TYPE_INITIALLY_UNOWNED,
  G_IMPLEMENT_INTERFACE(G_TYPE_GP_ICA_RENDERER, gp_ica_scope_interface_init));


static void gp_ica_scope_init( GpIcaScope *scope )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  priv->axis_info = g_new( GpIcaAxis, 1 );

}


static void gp_ica_scope_class_init( GpIcaScopeClass *klass )
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  g_type_class_add_private( klass, sizeof( GpIcaScopePriv ) );

  this_class->set_property = gp_ica_scope_set_property;
  this_class->constructor = gp_ica_scope_constructor;
  this_class->finalize = gp_ica_scope_finalize;

  g_object_class_install_property( this_class, PROP_STATE_RENDERER,
    g_param_spec_object( "state-renderer", "State renderer", "State Renderer", G_TYPE_GP_ICA_STATE_RENDERER, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_N_CHANNELS,
    g_param_spec_uint( "n_channels", "Number of channels", "Number of cifroscope channels", 1, 64, 1, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

  g_object_class_install_property( this_class, PROP_GROUND_COLOR,
    g_param_spec_uint( "ground-color", "Ground color", "Ground color", 0, G_MAXUINT32, 0xFF000000, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT ) );

  g_object_class_install_property( this_class, PROP_BORDER_COLOR,
    g_param_spec_uint( "border-color", "Border color", "Border color", 0, G_MAXUINT32, 0xFFA8A8A8, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT ) );

  g_object_class_install_property( this_class, PROP_AXIS_COLOR,
    g_param_spec_uint( "axis-color", "Axis color", "Axis color", 0, G_MAXUINT32, 0xFF333333, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT ) );

  g_object_class_install_property( this_class, PROP_ZERO_AXIS_COLOR,
    g_param_spec_uint( "zero-axis-color", "Zero axis color", "Zero axis color", 0, G_MAXUINT32, 0xFF808080, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT ) );

  g_object_class_install_property( this_class, PROP_TEXT_COLOR,
    g_param_spec_uint( "text-color", "Text color", "Text color", 0, G_MAXUINT32, 0xFFA8A8A8, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT ) );

}


static void gp_ica_scope_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec )
{
  GpIcaScope *scope = GP_ICA_SCOPE(object);
  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  switch ( prop_id )
    {

    case PROP_STATE_RENDERER:
      priv->state_renderer = g_value_get_object( value );
      break;

    case PROP_N_CHANNELS:
      priv->n_channels = g_value_get_uint( value );
      break;

    case PROP_GROUND_COLOR:
      priv->axis_info->ground_color = g_value_get_uint( value );
      priv->colors_up_to_date = TRUE;
      break;

    case PROP_BORDER_COLOR:
      priv->axis_info->border_color = g_value_get_uint( value );
      priv->colors_up_to_date = TRUE;
      break;

    case PROP_AXIS_COLOR:
      priv->axis_info->axis_color = g_value_get_uint( value );
      priv->colors_up_to_date = TRUE;
      break;

    case PROP_ZERO_AXIS_COLOR:
      priv->axis_info->zero_axis_color = g_value_get_uint( value );
      priv->colors_up_to_date = TRUE;
      break;

    case PROP_TEXT_COLOR:
      priv->axis_info->text_color = g_value_get_uint( value );
      priv->colors_up_to_date = TRUE;
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( scope, prop_id, pspec );
      break;

    }

}


static GObject*gp_ica_scope_constructor( GType g_type, guint n_construct_properties,
                                         GObjectConstructParam *construct_properties )
{

  GObject *scope = G_OBJECT_CLASS( gp_ica_scope_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( scope == NULL ) return NULL;

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  guint i;

  priv->axis_info->x_axis_name = g_strdup( "X" );
  priv->axis_info->y_axis_name = g_strdup( "Y" );

  priv->channels = g_new( GpCifroScopeValues*, priv->n_channels );
  for( i = 0; i < priv->n_channels; i++ )
    {

    priv->channels[i] = g_new( GpCifroScopeValues, 1 );
    priv->channels[i]->size = 1024;
    priv->channels[i]->data = g_new( gfloat, priv->channels[i]->size );

    priv->channels[i]->num = 0;

    priv->channels[i]->time_shift = 0.0;
    priv->channels[i]->time_step = 1.0;

    priv->channels[i]->value_shift = 0.0;
    priv->channels[i]->value_scale = 1.0;

    priv->channels[i]->show = FALSE;
    priv->channels[i]->color = cairo_sdline_color( g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), g_random_double_range( 0.5, 1.0 ), 1.0 );
    priv->channels[i]->name = NULL;
    priv->channels[i]->draw_type = GP_ICA_SCOPE_LINED;

    }

  priv->font_desc = NULL;

  priv->scope_surface = NULL;

  priv->x_axis_surface = NULL;
  priv->y_axis_surface = NULL;

  priv->x_axis_font = NULL;
  priv->y_axis_font = NULL;

  priv->x_pos_surface = NULL;
  priv->y_pos_surface = NULL;

  priv->info_surface = NULL;
  priv->info_font = NULL;

  priv->show_info = TRUE;

  priv->scope_update = TRUE;
  priv->x_axis_update = TRUE;
  priv->y_axis_update = TRUE;
  priv->info_update = TRUE;

  priv->colors_up_to_date = FALSE;

  return scope;

}


static void gp_ica_scope_finalize(GObject *object)
{
  GpIcaScope *scope = GP_ICA_SCOPE(object);
  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  guint i;

  for( i = 0; i < priv->n_channels; i++ )
    {
    g_free( priv->channels[i]->data );
    g_free( priv->channels[i]->name );
    g_free( priv->channels[i] );
    }

  g_free( priv->channels );

  pango_font_description_free( priv->font_desc );

  cairo_sdline_surface_destroy( priv->scope_surface );

  cairo_sdline_surface_destroy( priv->x_axis_surface );
  cairo_sdline_surface_destroy( priv->y_axis_surface );

  if( priv->x_axis_font != NULL ) g_object_unref( priv->x_axis_font );
  if( priv->y_axis_font != NULL ) g_object_unref( priv->y_axis_font );

  cairo_sdline_surface_destroy( priv->x_pos_surface );
  cairo_sdline_surface_destroy( priv->y_pos_surface );

  cairo_sdline_surface_destroy( priv->info_surface );
  if( priv->info_font != NULL ) g_object_unref( priv->info_font );

  g_free( priv->axis_info->x_axis_name );
  g_free( priv->axis_info->y_axis_name );
  g_free( priv->axis_info );


  G_OBJECT_CLASS( gp_ica_scope_parent_class )->finalize( G_OBJECT( scope ) );

}


static void gp_ica_scope_update_colors( GpIcaScope *scope )
{
  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  GdkRGBA color_bg;
  GdkRGBA color_fg;

  GtkStyleContext *style_context = gtk_style_context_new();

  {
    GtkWidgetPath *wpath = gtk_widget_path_new();
    gtk_widget_path_append_type(wpath, GTK_TYPE_WINDOW);
    gtk_style_context_set_path(style_context, wpath);
    gtk_widget_path_unref(wpath);
  }

  if(G_UNLIKELY(gtk_style_context_lookup_color(style_context, "theme_bg_color", &color_bg) == FALSE))
  {
    g_critical("Failed to get bg color.");
    goto end;
  }

  if(G_UNLIKELY(gtk_style_context_lookup_color(style_context, "theme_fg_color", &color_fg) == FALSE))
  {
    g_critical("Failed to get fg color.");
    goto end;
  }

  priv->axis_info->ground_color = cairo_sdline_color( color_bg.red, color_bg.green, color_bg.blue, 1.0 );
  priv->axis_info->border_color = cairo_sdline_color( color_fg.red, color_fg.green, color_fg.blue, 1.0 );
  priv->axis_info->text_color = cairo_sdline_color( color_fg.red, color_fg.green, color_fg.blue, 1.0 );

  double red = ( color_bg.red ) + 0.4 * ( ( color_fg.red ) - ( color_bg.red ) );
  double green = ( color_bg.green ) + 0.4 * ( ( color_fg.green ) - ( color_bg.green) );
  double blue = ( color_bg.blue) + 0.4 * ( ( color_fg.blue) - ( color_bg.blue ) );
  priv->axis_info->axis_color = cairo_sdline_color( red, green, blue, 1.0 );

  red = ( color_bg.red ) + 0.8 * ( ( color_fg.red ) - ( color_bg.red ) );
  green = ( color_bg.green ) + 0.8 * ( ( color_fg.green ) - ( color_bg.green) );
  blue = ( color_bg.blue) + 0.8 * ( ( color_fg.blue) - ( color_bg.blue ) );
  priv->axis_info->zero_axis_color = cairo_sdline_color( red, green, blue, 1.0 );

end:
  g_clear_object(&style_context);
  priv->colors_up_to_date = TRUE;
}


static void gp_ica_scope_draw_lined_data( cairo_sdline_surface *surface, GpIcaStateRenderer *state_renderer,
                                          GpCifroScopeValues *values )
{

  const GpIcaState *state = gp_ica_state_renderer_get_state( state_renderer );

  gint      width = state->visible_width;

  gfloat    x_min = state->from_x;
  gfloat    x_max = state->to_x;
  gfloat    y_max = state->to_y;

  gfloat    x_scale = state->cur_scale_x;
  gfloat    y_scale = state->cur_scale_y;

  gfloat   *values_data = values->data;
  gint      values_num = values->num;

  gfloat    times_shift = values->time_shift;
  gfloat    times_step = values->time_step;
  gfloat    values_scale = values->value_scale;
  gfloat    values_shift = values->value_shift;

  guint32   values_color = values->color;

#define VALUES_DATA(i) ( ( values_data[i] * values_scale ) + values_shift )

  gint      i, j;
  gint      i_range_begin, i_range_end;
  gfloat    x_range_begin, x_range_end;
  gfloat    y_start, y_end;
  gfloat    x1, x2, y1, y2;
  gboolean  draw = FALSE;

  x_range_begin = x_min - x_scale;
  x_range_end = x_min;

  x1 = -1.0; y1 = -1.0;
  y_start = y_end = 0.0;

  for( i = 0; i <= width; i++ )
  {

    draw = FALSE;
    x2 = i;

    // Диапазон значений X между двумя точками осциллограммы.
    x_range_begin += x_scale;
    x_range_end += x_scale;

    // Диапазон индексов значений между двумя точками осциллограммы.
    i_range_begin = ( x_range_begin - times_shift ) / times_step;
    i_range_end = ( x_range_end - times_shift ) / times_step;

    // Проверка индексов на попадание в границы осциллограммы.
    if( ( i_range_begin < 0 ) && ( i_range_end <= 0 ) ) continue;
    if( i_range_begin >= values_num ) break;

    if( i_range_begin < 0 ) i_range_begin = 0;
    if( i_range_end >= values_num ) i_range_end = values_num - 1;

    if( ( i_range_end == i_range_begin ) && ( i != width ) ) continue;

    // Последня точка не попала в границу осциллограммы.
    if( i_range_end == i_range_begin )
      {

      if( i_range_begin == 0 ) continue;

      if( i_range_end == values_num - 1 ) break;

      if( isnan( values_data[ i_range_begin ] ) || isnan( values_data[ i_range_end + 1 ] ) ) continue;

      // Предыдущей точки нет, расчитаем значение и расстояние до нее от начала видимой осциллограммы.
      // В этом случае все точки лежат за границей видимой области.
      if( x1 < 0 )
        {
        x1 = ( ( times_step * floor( x_min / times_step ) ) - x_min ) / x_scale;
        y1 = ( y_max - VALUES_DATA( i_range_begin ) ) / y_scale;
        x1 = CLAMP( x1, -32000.0, 32000.0 );
        y1 = CLAMP( y1, -32000.0, 32000.0 );
        }

      // Значение и расстояние до текущей точки от конца видимой осциллограммы.
      x2 = width + ( ( times_step * ceil( x_max / times_step ) ) - x_max ) / x_scale;
      y2 = ( y_max - VALUES_DATA( i_range_end + 1 ) ) / y_scale;
      x2 = CLAMP( x2, -32000.0, 32000.0 );
      y2 = CLAMP( y2, -32000.0, 32000.0 );
      draw = TRUE;

      }

    // Индекс для этой точки осциллограммы изменился на 1.
    // Необходимо нарисовать линию из предыдущей точки в текущую.
    else if( ( i_range_end - i_range_begin ) == 1 )
      {

      // Предыдущей точки нет, расчитаем значение и расстояние до нее от текущей позиции.
      if( ( x1 < 0 ) && ( i_range_begin >= 0 ) )
        {
        x1 = (gfloat)i - ( times_step / x_scale ) + 1.0;
        y1 = ( y_max - VALUES_DATA( i_range_begin ) ) / y_scale;
        x1 = CLAMP( x1, -32000.0, 32000.0 );
        y1 = CLAMP( y1, -32000.0, 32000.0 );
        }

      y2 = ( y_max - VALUES_DATA( i_range_end ) ) / y_scale;
      y2 = CLAMP( y2, -32000.0, 32000.0 );
      if( !isnan( values_data[ i_range_begin ] ) && !isnan( values_data[ i_range_end ] ) ) draw = TRUE;

      }

    // В одну точку осциллограммы попадает несколько значений.
    // Берем максимум и минимум из них и рисуем вертикальной линией.
    else
      {

      // Нарисуем линию от предыдущей точки.
      if( !isnan( values_data[ i_range_begin ] ) && !isnan( values_data[ i_range_begin + 1 ] ) )
        {
        y1 = ( y_max - VALUES_DATA( i_range_begin ) ) / y_scale;
        y2 = ( y_max - VALUES_DATA( i_range_begin + 1 ) ) / y_scale;
        y1 = CLAMP( y1, -32000.0, 32000.0 );
        y2 = CLAMP( y2, -32000.0, 32000.0 );
        cairo_sdline( surface, x2 - 1, y1, x2, y2, values_color );
        draw = TRUE;
        }

      for( j = i_range_begin + 1; j <= i_range_end; j++ )
        {
        if( isnan( values_data[j] ) ) continue;
        y_start = VALUES_DATA( j );
        y_end = y_start;
        draw = TRUE;
        break;
        }
      for( ; j <= i_range_end; j++ )
        {
        if( isnan( values_data[j] ) ) continue;
        if( VALUES_DATA( j ) < y_start ) y_start = VALUES_DATA( j );
        if( VALUES_DATA( j ) > y_end ) y_end = VALUES_DATA( j );
        draw = TRUE;
        }

      x1 = i;
      y1 = ( y_max - y_start ) / y_scale;
      y2 = ( y_max - y_end ) / y_scale;
      y1 = CLAMP( y1, -32000.0, 32000.0 );
      y2 = CLAMP( y2, -32000.0, 32000.0 );

      }

    if( draw )
      cairo_sdline( surface, x1, y1, x2, y2, values_color );

    if( ( i_range_end - i_range_begin ) == 1 )
      { x1 = x2; y1 = y2; }
    else
      { x1 = -1; y1 = -1; }

  }

  cairo_surface_mark_dirty( surface->cairo_surface );

}


static void gp_ica_scope_draw_dotted_data( cairo_sdline_surface *surface, GpIcaStateRenderer *state_renderer,
                                           GpCifroScopeValues *values )
{

  const GpIcaState *state = gp_ica_state_renderer_get_state( state_renderer );

  gint      width = state->visible_width;
  gint      height = state->visible_height;

  gfloat    x_min = state->from_x;
  gfloat    x_max = state->to_x;
  gfloat    y_min = state->from_y;
  gfloat    y_max = state->to_y;

  gfloat   *values_data = values->data;
  gfloat    times_shift = values->time_shift;
  gfloat    times_step = values->time_step;
  gfloat    values_scale = values->value_scale;
  gfloat    values_shift = values->value_shift;

  guint32   values_color = values->color;

#define VALUES_TIME(i) ( ( i * times_step ) + times_shift )
#define VALUES_DATA(i) ( ( values_data[i] * values_scale ) + values_shift )

  gint      i;
  gfloat    x_scale = state->cur_scale_x;
  gfloat    y_scale = state->cur_scale_y;
  gint      i_range_begin, i_range_end;
  gfloat    x, y;

  i_range_begin = ( state->from_x - values->time_shift ) / values->time_step;
  i_range_end = ( state->to_x - values->time_shift ) / values->time_step;

  i_range_begin = CLAMP( i_range_begin, 0, values->num );
  i_range_end = CLAMP( i_range_end, 0, values->num );

  x_scale = ( x_max - x_min ) / width;
  y_scale = ( y_max - y_min ) / height;

  if( i_range_begin > i_range_end ) return;

  for( i = i_range_begin; i < i_range_end; i++ )
    {
    if( isnan( values_data[i] ) ) continue;
    x = VALUES_TIME( i );
    x = ( x - x_min ) / x_scale;
    y = VALUES_DATA( i );
    y = ( y_max - y ) / y_scale;
    cairo_sdline_dot( surface, x, y, values_color );
    }

  cairo_surface_mark_dirty( surface->cairo_surface );

}

static void gp_ica_scope_draw_mix_data( cairo_sdline_surface *surface, GpIcaStateRenderer *state_renderer,
                                           GpCifroScopeValues *values )
{
  const GpIcaState *state = gp_ica_state_renderer_get_state( state_renderer );

  gint      width = state->visible_width;
  gint      height = state->visible_height;

  gfloat    x_min = state->from_x;
  gfloat    y_min = state->from_y;
  gfloat    x_max = state->to_x;
  gfloat    y_max = state->to_y;

  gfloat    x_scale = state->cur_scale_x;
  gfloat    y_scale = state->cur_scale_y;

  gfloat   *values_data = values->data;
  gint      values_num = values->num;

  gfloat    times_shift = values->time_shift;
  gfloat    times_step = values->time_step;
  gfloat    values_scale = values->value_scale;
  gfloat    values_shift = values->value_shift;

  guint32   values_color = values->color;

#define VALUES_DATA(i) ( ( values_data[i] * values_scale ) + values_shift )

  gint      i, j;
  gint      i_range_begin, i_range_end;
  gfloat    x_range_begin, x_range_end;
  gfloat    y_start, y_end;
  gfloat    x1, x2, y1, y2;
  gboolean  draw = FALSE;

  x_range_begin = x_min - x_scale;
  x_range_end = x_min;

  x1 = -1.0; y1 = -1.0;
  y_start = y_end = 0.0;

  for( i = 0; i <= width; i++ )
  {

    draw = FALSE;
    x2 = i;

    // Диапазон значений X между двумя точками осциллограммы.
    x_range_begin += x_scale;
    x_range_end += x_scale;

    // Диапазон индексов значений между двумя точками осциллограммы.
    i_range_begin = ( x_range_begin - times_shift ) / times_step;
    i_range_end = ( x_range_end - times_shift ) / times_step;

    // Проверка индексов на попадание в границы осциллограммы.
    if( ( i_range_begin < 0 ) && ( i_range_end <= 0 ) ) continue;
    if( i_range_begin >= values_num ) break;

    if( i_range_begin < 0 ) i_range_begin = 0;
    if( i_range_end >= values_num ) i_range_end = values_num - 1;

    if( ( i_range_end == i_range_begin ) && ( i != width ) ) continue;

    // Последня точка не попала в границу осциллограммы.
    if( i_range_end == i_range_begin )
    {

      if( i_range_begin == 0 ) continue;

      if( i_range_end == values_num - 1 ) break;

      if( isnan( values_data[ i_range_begin ] ) || isnan( values_data[ i_range_end + 1 ] ) ) continue;

      // Предыдущей точки нет, расчитаем значение и расстояние до нее от начала видимой осциллограммы.
      // В этом случае все точки лежат за границей видимой области.
      if( x1 < 0 )
        {
        x1 = ( ( times_step * floor( x_min / times_step ) ) - x_min ) / x_scale;
        y1 = ( y_max - VALUES_DATA( i_range_begin ) ) / y_scale;
        x1 = CLAMP( x1, -32000.0, 32000.0 );
        y1 = CLAMP( y1, -32000.0, 32000.0 );
        }

      // Значение и расстояние до текущей точки от конца видимой осциллограммы.
      x2 = width + ( ( times_step * ceil( x_max / times_step ) ) - x_max ) / x_scale;
      y2 = ( y_max - VALUES_DATA( i_range_end + 1 ) ) / y_scale;
      x2 = CLAMP( x2, -32000.0, 32000.0 );
      y2 = CLAMP( y2, -32000.0, 32000.0 );
      draw = TRUE;

    }

    // Индекс для этой точки осциллограммы изменился на 1.
    // Необходимо нарисовать линию из предыдущей точки в текущую.
    else if( ( i_range_end - i_range_begin ) == 1 )
    {

      // Предыдущей точки нет, расчитаем значение и расстояние до нее от текущей позиции.
      if( ( x1 < 0 ) && ( i_range_begin >= 0 ) )
        {
        x1 = (gfloat)i - ( times_step / x_scale ) + 1.0;
        y1 = ( y_max - VALUES_DATA( i_range_begin ) ) / y_scale;
        x1 = CLAMP( x1, -32000.0, 32000.0 );
        y1 = CLAMP( y1, -32000.0, 32000.0 );
        }

      y2 = ( y_max - VALUES_DATA( i_range_end ) ) / y_scale;
      y2 = CLAMP( y2, -32000.0, 32000.0 );
      if( !isnan( values_data[ i_range_begin ] ) && !isnan( values_data[ i_range_end ] ) ) draw = TRUE;

    }

    // В одну точку осциллограммы попадает несколько значений.
    // Берем максимум и минимум из них и рисуем вертикальной линией.
    else
    {

      // Нарисуем линию от предыдущей точки.
      if( !isnan( values_data[ i_range_begin ] ) && !isnan( values_data[ i_range_begin + 1 ] ) )
        {
        y1 = ( y_max - VALUES_DATA( i_range_begin ) ) / y_scale;
        y2 = ( y_max - VALUES_DATA( i_range_begin + 1 ) ) / y_scale;
        y1 = CLAMP( y1, -32000.0, 32000.0 );
        y2 = CLAMP( y2, -32000.0, 32000.0 );
        cairo_sdline( surface, x2 - 1, y1, x2, y2, values_color );
        draw = TRUE;
        }

      for( j = i_range_begin + 1; j <= i_range_end; j++ )
        {
        if( isnan( values_data[j] ) ) continue;
        y_start = VALUES_DATA( j );
        y_end = y_start;
        draw = TRUE;
        break;
        }
      for( ; j <= i_range_end; j++ )
        {
        if( isnan( values_data[j] ) ) continue;
        if( VALUES_DATA( j ) < y_start ) y_start = VALUES_DATA( j );
        if( VALUES_DATA( j ) > y_end ) y_end = VALUES_DATA( j );
        draw = TRUE;
        }

      x1 = i;
      y1 = ( y_max - y_start ) / y_scale;
      y2 = ( y_max - y_end ) / y_scale;
      y1 = CLAMP( y1, -32000.0, 32000.0 );
      y2 = CLAMP( y2, -32000.0, 32000.0 );

    }

    if( draw )
      cairo_sdline( surface, x1, y1, x2, y2, values_color );

    if( ( i_range_end - i_range_begin ) == 1 )
      { x1 = x2; y1 = y2; }
    else
      { x1 = -1; y1 = -1; }
  }
    ///line<--

#define VALUES_TIME(i) ( ( i * times_step ) + times_shift )
//~ #define VALUES_DATA(i) ( ( values_data[i] * values_scale ) + values_shift )

  gfloat    x, y;

  i_range_begin = ( state->from_x - values->time_shift ) / values->time_step;
  i_range_end = ( state->to_x - values->time_shift ) / values->time_step;

  i_range_begin = CLAMP( i_range_begin, 0, values->num );
  i_range_end = CLAMP( i_range_end, 0, values->num );

  x_scale = ( x_max - x_min ) / width;
  y_scale = ( y_max - y_min ) / height;

  if( i_range_begin > i_range_end ) return;

  cairo_t *cr = cairo_create(surface->cairo_surface);

  //~ gdouble a = (gdouble)((guchar)(values_color >> 24)) / 255;
  gdouble r = (gdouble)((guchar)(values_color >> 16)) / 255;
  gdouble g = (gdouble)((guchar)(values_color >> 8)) / 255;
  gdouble b = (gdouble)((guchar)(values_color >> 0)) / 255;

  cairo_set_source_rgb(cr, r, g, b);
  cairo_set_line_width(cr, 1.0);
  for( i = i_range_begin; i < i_range_end; i++ )
  {
    if( isnan( values_data[i] ) ) continue;
    x = VALUES_TIME( i );
    x = ( x - x_min ) / x_scale;
    y = VALUES_DATA( i );
    y = ( y_max - y ) / y_scale;
    //~ cairo_sdline_dot( surface, x, y, values_color );

    cairo_rectangle(cr, x - 2, y - 2, 5, 5);
    cairo_stroke(cr);
  }
  cairo_destroy(cr);

  cairo_surface_mark_dirty( surface->cairo_surface );

}

static gboolean gp_ica_scope_draw_info( GpIcaScopePriv *priv, gint *x, gint *y, gint *width, gint *height )
{

  const GpIcaState *state = gp_ica_state_renderer_get_state( priv->state_renderer );

  gint      n_labels;

  gint      mark_width;
  gint      label_width;
  gint      font_height;

  gint      info_center;
  gint      info_width;
  gint      info_height;

  gint      x1, y1, x2, y2;

  gint      label_top;

  gdouble   value_x;
  gdouble   value_y;

  gint      value_power;
  gchar     text_format[ 128 ];
  gchar     text_str[ 128 ];

  gint      text_width;
  gint      text_height;
  gint      text_spacing = state->border_top / 4;

  guint i;

  GpCifroScopeValues **channels = priv->channels;

  cairo_sdline_surface *surface = priv->info_surface;
  PangoLayout *info_font = priv->info_font;

  GpIcaAxis *defaults = priv->axis_info;

  // Вычисляем максимальную ширину и высоту строки с текстом.
  pango_layout_set_text( info_font, defaults->x_axis_name, -1 );
  pango_layout_get_size( info_font, &text_width, &text_height );
  mark_width = text_width;
  font_height = text_height;

  pango_layout_set_text( info_font, defaults->y_axis_name, -1 );
  pango_layout_get_size( info_font, &text_width, &text_height );
  if( text_width > mark_width ) mark_width = text_width;
  if( text_height > font_height ) font_height = text_height;

  n_labels = 2;
  label_width = 0;
  for( i = 0; i < priv->n_channels; i++ )
    {

    if( channels[i]->value_scale == 1.0 && channels[i]->name == NULL && label_width != 0 ) continue;

    value_y = state->value_y / channels[i]->value_scale - channels[i]->value_shift;
      gp_ica_state_get_axis_step( state->cur_scale_y, 1, &value_y, NULL, NULL, &value_power );
    if( value_power > 0 ) value_power = 0;
    g_sprintf( text_format, "-%%.%df", (gint)fabs( value_power ) );
    value_y = MAX( ABS( state->from_y ), ABS( state->to_y ) );
    value_y = value_y / channels[i]->value_scale - channels[i]->value_shift;
    g_sprintf( text_str, text_format, value_y );

    if( channels[i]->name != NULL )
      {
      pango_layout_set_text( info_font, channels[i]->name, -1 );
      pango_layout_get_size( info_font, &text_width, &text_height );
      if( text_width > mark_width ) mark_width = text_width;
      if( text_height > font_height ) font_height = text_height;
      }

    pango_layout_set_text( info_font, text_str, -1 );
    pango_layout_get_size( info_font, &text_width, &text_height );
    if( text_width > label_width ) label_width = text_width;
    if( text_height > font_height ) font_height = text_height;

    if( channels[i]->value_scale != 1.0 || channels[i]->name != NULL ) n_labels += 1;

    }

  value_x = state->value_x;
  gp_ica_state_get_axis_step( state->cur_scale_x, 1, &value_x, NULL, NULL, &value_power );
  if( value_power > 0 ) value_power = 0;
  g_sprintf( text_format, "-%%.%df", (gint)fabs( value_power ) );
  g_sprintf( text_str, text_format, MAX( ABS( state->from_x ), ABS( state->to_x ) ) );

  pango_layout_set_text( info_font, text_str, -1 );
  pango_layout_get_size( info_font, &text_width, &text_height );
  if( text_width > label_width ) label_width = text_width;
  if( text_height > font_height ) font_height = text_height;

  // Ширина текста с названием величины, с её значением и высота строки.
  mark_width /= PANGO_SCALE;
  label_width /= PANGO_SCALE;
  font_height /= PANGO_SCALE;

  // Размер места для отображения информации.
  info_width = 5 * text_spacing + label_width + mark_width;
  info_height = n_labels * ( font_height + text_spacing ) + 3 * text_spacing;
  if( n_labels > 2 ) info_height += text_spacing;

  // Проверяем размеры области отображения.
  if( info_width > state->area_width - 12 * text_spacing ) return FALSE;
  if( info_height > state->area_height - 12 * text_spacing ) return FALSE;

  // Место для отображения информации.
  if( state->pointer_x > state->area_width - 8 * text_spacing - info_width &&
      state->pointer_y < 8 * text_spacing + info_height )
    x1 = 6 * text_spacing;
  else
    x1 = state->area_width - 6 * text_spacing - info_width;
  y1 = 6 * text_spacing;
  x2 = x1 + info_width;
  y2 = y1 + info_height;

  cairo_sdline_bar( surface, x1, y1, x2, y2, 0x44000000 );
  cairo_sdline_h( surface, x1 + 2, x2 - 2, y1, defaults->axis_color );
  cairo_sdline_h( surface, x1 + 2, x2 - 2, y2, defaults->axis_color );
  cairo_sdline_v( surface, x1, y1 + 2, y2 - 2, defaults->axis_color );
  cairo_sdline_v( surface, x2, y1 + 2, y2 - 2, defaults->axis_color );

  cairo_surface_mark_dirty( surface->cairo_surface );

  cairo_sdline_set_cairo_color( surface, defaults->text_color );
  cairo_set_line_width( surface->cairo, 1.0 );

  label_top = y1 + 2 * text_spacing;
  info_center = x1 + 3 * text_spacing + label_width;

  // Значение по оси абсцисс.
  value_x = state->value_x;
  gp_ica_state_get_axis_step( state->cur_scale_x, 1, &value_x, NULL, NULL, &value_power );
  if( value_power > 0 ) value_power = 0;
  g_sprintf( text_format, "%%.%df", (gint)fabs( value_power ) );
  g_sprintf( text_str, text_format, state->value_x );

  pango_layout_set_text( info_font, text_str, -1 );
  pango_layout_get_size( info_font, &text_width, &text_height );
  cairo_move_to( surface->cairo, info_center - text_width / PANGO_SCALE - text_spacing / 2, label_top );
  pango_cairo_show_layout( surface->cairo, info_font );

  pango_layout_set_text( info_font, defaults->x_axis_name, -1 );
  cairo_move_to( surface->cairo, info_center, label_top );
  pango_cairo_show_layout( surface->cairo, info_font );
  label_top += font_height + text_spacing;

  // Значение по оси ординат.
  value_y = state->value_y;
  gp_ica_state_get_axis_step( state->cur_scale_y, 1, &value_y, NULL, NULL, &value_power );
  if( value_power > 0 ) value_power = 0;
  g_sprintf( text_format, "%%.%df", (gint)fabs( value_power ) );
  g_sprintf( text_str, text_format, state->value_y );

  pango_layout_set_text( info_font, text_str, -1 );
  pango_layout_get_size( info_font, &text_width, &text_height );
  cairo_move_to( surface->cairo, info_center - text_width / PANGO_SCALE - text_spacing / 2, label_top );
  pango_cairo_show_layout( surface->cairo, info_font );

  pango_layout_set_text( info_font, defaults->y_axis_name, -1 );
  cairo_move_to( surface->cairo, info_center, label_top );
  pango_cairo_show_layout( surface->cairo, info_font );

  // Значения для каналов с отличным от 1 масштабом.
  if( n_labels > 2 )
    {

    label_top += font_height + text_spacing;
    cairo_sdline_h( surface, x1 + 4, x2 - 4, label_top, defaults->axis_color );
    label_top += text_spacing;

    for( i = 0; i < priv->n_channels; i++ )
      {

      if( channels[i]->value_scale == 1.0 && channels[i]->name == NULL ) continue;

      value_y = ( state->value_y - channels[i]->value_shift ) / channels[i]->value_scale;
        gp_ica_state_get_axis_step( state->cur_scale_y, 1, &value_y, NULL, NULL, &value_power );
      if( value_power > 0 ) value_power = 0;
      g_sprintf( text_format, "%%.%df", (gint)fabs( value_power ) );
      g_sprintf( text_str, text_format, ( state->value_y - channels[i]->value_shift ) / channels[i]->value_scale );

      cairo_sdline_set_cairo_color( surface, channels[i]->color );

      pango_layout_set_text( info_font, text_str, -1 );
      pango_layout_get_size( info_font, &text_width, &text_height );
      cairo_move_to( surface->cairo, info_center - text_width / PANGO_SCALE - text_spacing / 2, label_top );
      pango_cairo_show_layout( surface->cairo, info_font );

      pango_layout_set_text( info_font, channels[i]->name == NULL ? defaults->y_axis_name : channels[i]->name, -1 );
      cairo_move_to( surface->cairo, info_center, label_top );
      pango_cairo_show_layout( surface->cairo, info_font );

      label_top += font_height + text_spacing;

      }

    }

  cairo_surface_flush( surface->cairo_surface );

  *x = x1;
  *y = y1;
  *width = x2 - x1 + 1;
  *height = y2 - y1 + 1;

  return TRUE;

}


static gint gp_ica_scope_get_renderers_num( GpIcaRenderer *scope )
{

  return SURFACES_NUM;

}


static GpIcaRendererType gp_ica_scope_get_renderer_type( GpIcaRenderer *scope, gint renderer_id )
{

  switch( renderer_id )
    {
    case SCOPE_SURFACE: return GP_ICA_RENDERER_VISIBLE;
    case X_AXIS_SURFACE:
    case X_POS_SURFACE:
    case Y_AXIS_SURFACE:
    case Y_POS_SURFACE:
    case INFO_SURFACE:  return GP_ICA_RENDERER_AREA;
    }

  return GP_ICA_RENDERER_UNKNOWN;

}


static const gchar *gp_ica_scope_get_name( GpIcaRenderer *scope, gint renderer_id )
{

  switch( renderer_id )
    {
    case SCOPE_SURFACE:  return "Cifro scope main renderer";
    case X_AXIS_SURFACE: return "Cifro scope x axis renderer";
    case X_POS_SURFACE: return "Cifro scope x position renderer";
    case Y_AXIS_SURFACE: return "Cifro scope y axis renderer";
    case Y_POS_SURFACE: return "Cifro scope y position renderer";
    case INFO_SURFACE:   return "Cifro scope info renderer";
    }

  return "Unknown renderer.";

}


static void gp_ica_scope_set_surface( GpIcaRenderer *scope, gint renderer_id, guchar *data, gint width, gint height,
                                      gint stride )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  cairo_surface_t *surface;

  if( priv->font_desc == NULL ) gp_ica_scope_configure(GP_ICA_SCOPE( scope ));

  switch( renderer_id )
    {

    case SCOPE_SURFACE:

      // Поверхность для рисования осциллограмм.
      cairo_sdline_surface_destroy( priv->scope_surface );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->scope_surface = cairo_sdline_surface_create_for( surface );
      priv->scope_surface->self_create = 1;

      break;

    case X_AXIS_SURFACE:

      // Поверхность для рисования оцифровки оси абсцисс.
      cairo_sdline_surface_destroy( priv->x_axis_surface );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->x_axis_surface = cairo_sdline_surface_create_for( surface );
      priv->x_axis_surface->self_create = 1;

      // Шрифт.
      if( priv->x_axis_font != NULL ) g_object_unref( priv->x_axis_font );
      priv->x_axis_font = pango_cairo_create_layout( priv->x_axis_surface->cairo );
      pango_layout_set_font_description( priv->x_axis_font, priv->font_desc );

      break;

    case X_POS_SURFACE:

      // Поверхность для рисования местоположения по оси абсцисс.
      cairo_sdline_surface_destroy( priv->x_pos_surface );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->x_pos_surface = cairo_sdline_surface_create_for( surface );
      priv->x_pos_surface->self_create = 1;

      break;

    case Y_AXIS_SURFACE:

      // Поверхность для рисования оцифровки оси ординат.
      cairo_sdline_surface_destroy( priv->y_axis_surface );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->y_axis_surface = cairo_sdline_surface_create_for( surface );
      priv->y_axis_surface->self_create = 1;

      // Шрифт.
      if( priv->y_axis_font != NULL ) g_object_unref( priv->y_axis_font );
      priv->y_axis_font = pango_cairo_create_layout( priv->y_axis_surface->cairo );
      pango_layout_set_font_description( priv->y_axis_font, priv->font_desc );

      break;

    case Y_POS_SURFACE:

      // Поверхность для рисования местоположения по оси ординат.
      cairo_sdline_surface_destroy( priv->y_pos_surface );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->y_pos_surface = cairo_sdline_surface_create_for( surface );
      priv->y_pos_surface->self_create = 1;

      break;

    case INFO_SURFACE:

      // Поверхность для рисования информации о значениях.
      cairo_sdline_surface_destroy( priv->info_surface );
      surface = cairo_image_surface_create_for_data( data, CAIRO_FORMAT_ARGB32, width, height, stride );
      priv->info_surface = cairo_sdline_surface_create_for( surface );
      priv->info_surface->self_create = 1;

      // Шрифт.
      if( priv->info_font != NULL ) g_object_unref( priv->info_font );
      priv->info_font = pango_cairo_create_layout( priv->info_surface->cairo );
      pango_layout_set_font_description( priv->info_font, priv->font_desc );

      break;

    }

  priv->scope_update = TRUE;
  priv->x_axis_update = TRUE;
  priv->y_axis_update = TRUE;
  priv->info_update = TRUE;

}


static GpIcaRendererAvailability gp_ica_scope_render( GpIcaRenderer *scope, gint renderer_id, gint *x, gint *y,
                                                      gint *width, gint *height )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );
  const GpIcaState *state = gp_ica_state_renderer_get_state( priv->state_renderer );

  guint i;

  switch( renderer_id )
    {

    case SCOPE_SURFACE:

      if( priv->scope_surface == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( !priv->scope_update ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
      priv->scope_update = FALSE;

      *x = 0;
      *y = 0;
      *width = state->visible_width;
      *height = state->visible_height;

      cairo_sdline_clear_color( priv->scope_surface, priv->axis_info->ground_color );
        gp_ica_axis_draw_axis( priv->scope_surface, priv->state_renderer, priv->axis_info );

      for( i = 0; i < priv->n_channels; i++ )
        if( priv->channels[i]->show )
          {
            switch(priv->channels[i]->draw_type)
            {
              case GP_ICA_SCOPE_LINED:
                gp_ica_scope_draw_lined_data( priv->scope_surface, priv->state_renderer, priv->channels[i] );
              break;
              case GP_ICA_SCOPE_DOTTED:
                gp_ica_scope_draw_dotted_data( priv->scope_surface, priv->state_renderer, priv->channels[i] );
              break;
              case GP_ICA_SCOPE_MIX:
                gp_ica_scope_draw_mix_data( priv->scope_surface, priv->state_renderer, priv->channels[i] );
              break;
              default:
                gp_ica_scope_draw_lined_data( priv->scope_surface, priv->state_renderer, priv->channels[i] );
              break;
            }
          }

      cairo_sdline_h( priv->scope_surface, 0, state->visible_width - 1, 0, priv->axis_info->border_color );
      cairo_sdline_v( priv->scope_surface, 0, 0, state->visible_height - 1, priv->axis_info->border_color );
      cairo_sdline_h( priv->scope_surface, 0, state->visible_width - 1, state->visible_height - 1, priv->axis_info->border_color );
      cairo_sdline_v( priv->scope_surface, state->visible_width - 1, 0, state->visible_height - 1, priv->axis_info->border_color );

      return GP_ICA_RENDERER_AVAIL_ALL;

    case X_AXIS_SURFACE:

      if( priv->x_axis_surface == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( !priv->x_axis_update ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
      priv->x_axis_update = FALSE;

      *x = 0;
      *y = 0;
      *width = state->area_width;
      *height = state->border_top;
      cairo_sdline_bar( priv->x_axis_surface, *x, *y, *width - 1, *height - 1, priv->axis_info->ground_color );

        gp_ica_axis_draw_hruler( priv->x_axis_surface, priv->x_axis_font, priv->state_renderer, priv->axis_info );

      return GP_ICA_RENDERER_AVAIL_ALL;

    case X_POS_SURFACE:

      if( priv->x_pos_surface == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( !priv->x_pos_update ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
      priv->x_pos_update = FALSE;

      *x = state->border_left;
      *y = state->area_height - state->border_bottom;
      *width = state->area_width - state->border_left - state->border_right;
      *height = state->border_bottom;
      cairo_sdline_bar( priv->x_pos_surface, *x, *y, *x + *width - 1, *y + *height - 1, priv->axis_info->ground_color );

        gp_ica_axis_draw_x_pos( priv->x_pos_surface, priv->state_renderer, priv->axis_info );

      return GP_ICA_RENDERER_AVAIL_ALL;

    case Y_AXIS_SURFACE:

      if( priv->y_axis_surface == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( !priv->y_axis_update ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
      priv->y_axis_update = FALSE;

      *x = 0;
      *y = 0;
      *width = state->border_left;
      *height = state->area_height;
      cairo_sdline_bar( priv->y_axis_surface, *x, *y, *width - 1, *height - 1, priv->axis_info->ground_color );

        gp_ica_axis_draw_vruler( priv->y_axis_surface, priv->y_axis_font, priv->state_renderer, priv->axis_info );

      return GP_ICA_RENDERER_AVAIL_ALL;

    case Y_POS_SURFACE:

      if( priv->y_pos_surface == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( !priv->y_pos_update ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
      priv->y_pos_update = FALSE;

      *x = state->area_width - state->border_right;
      *y = state->border_top;
      *width = state->border_right;
      *height = state->area_height - state->border_top;
      cairo_sdline_bar( priv->y_pos_surface, *x, *y, *x + *width - 1, *y + *height - 1, priv->axis_info->ground_color );

        gp_ica_axis_draw_y_pos( priv->y_pos_surface, priv->state_renderer, priv->axis_info );

      return GP_ICA_RENDERER_AVAIL_ALL;

    case INFO_SURFACE:

      if( !priv->show_info ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( priv->info_surface == NULL ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( state->pointer_x < 0 || state->pointer_y < 0 ) return GP_ICA_RENDERER_AVAIL_NONE;
      if( state->value_x < state->from_x || state->value_x > state->to_x ||
          state->value_y < state->from_y || state->value_y > state->to_y )
        return GP_ICA_RENDERER_AVAIL_NONE;

      if( !priv->info_update ) return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;

      if( !gp_ica_scope_draw_info( priv, x, y, width, height ) ) return GP_ICA_RENDERER_AVAIL_NONE;
      priv->info_update = FALSE;

      return GP_ICA_RENDERER_AVAIL_ALL;

    }

  return GP_ICA_RENDERER_AVAIL_NONE;

}


static void gp_ica_scope_set_shown( GpIcaRenderer *scope, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  priv->scope_update = TRUE;
  priv->x_axis_update = TRUE;
  priv->x_pos_update = TRUE;
  priv->y_axis_update = TRUE;
  priv->y_pos_update = TRUE;
  priv->info_update = TRUE;

}


static void gp_ica_scope_set_pointer( GpIcaRenderer *scope, gint pointer_x, gint pointer_y, gdouble value_x,
                                      gdouble value_y )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  priv->x_axis_update = TRUE;
  priv->y_axis_update = TRUE;
  priv->info_update = TRUE;

}


GpIcaScope *gp_ica_scope_new(GpIcaStateRenderer *state_renderer, guint n_channels)
{

  return g_object_new(G_TYPE_GP_ICA_SCOPE, "state-renderer", state_renderer, "n_channels", n_channels, NULL );

}


void gp_ica_scope_set_axis_name( GpIcaScope *scope, const gchar *x_axis_name, const gchar *y_axis_name )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  g_free( priv->axis_info->x_axis_name );
  g_free( priv->axis_info->y_axis_name );

  priv->axis_info->x_axis_name = g_strdup( x_axis_name );
  priv->axis_info->y_axis_name = g_strdup( y_axis_name );

  priv->x_axis_update = TRUE;
  priv->y_axis_update = TRUE;

}


void gp_ica_scope_set_show_info( GpIcaScope *scope, gboolean show )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  priv->show_info = show;

}


void gp_ica_scope_set_channel_time_param( GpIcaScope *scope, gint channel, gfloat time_shift, gfloat time_step )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  if( channel >= priv->n_channels ) return;

  priv->channels[ channel ]->time_shift = time_shift;
  priv->channels[ channel ]->time_step = time_step;

  priv->scope_update = TRUE;

}


void gp_ica_scope_set_channel_value_param( GpIcaScope *scope, gint channel, gfloat value_shift, gfloat value_scale )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  if( channel >= priv->n_channels ) return;

  priv->channels[ channel ]->value_shift = value_shift;
  priv->channels[ channel ]->value_scale = value_scale;

  priv->scope_update = TRUE;

}


void gp_ica_scope_set_channel_axis_name( GpIcaScope *scope, gint channel, const gchar *axis_name )
{


  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  if( channel >= priv->n_channels ) return;

  g_free( priv->channels[ channel ]->name );
  priv->channels[ channel ]->name = g_strdup( axis_name );

}


void gp_ica_scope_set_channel_draw_type( GpIcaScope *scope, gint channel, GpIcaScopeDrawType draw_type )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  if( channel >= priv->n_channels ) return;

  priv->channels[channel]->draw_type = draw_type;

}


void gp_ica_scope_set_channel_color( GpIcaScope *scope, gint channel, gdouble red, gdouble green, gdouble blue )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  if( channel >= priv->n_channels ) return;

  priv->channels[ channel ]->color = cairo_sdline_color( red, green, blue, 1.0 );

}


void gp_ica_scope_set_show_channel( GpIcaScope *scope, gint channel, gboolean show )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  if( channel >= priv->n_channels ) return;

  priv->channels[ channel ]->show = show;

}


void gp_ica_scope_set_data( GpIcaScope *scope, guint channel, guint num, gfloat *values )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  if( channel >= priv->n_channels ) return;

  if( num > priv->channels[ channel ]->size )
    {
    priv->channels[ channel ]->data = g_renew( float, priv->channels[ channel ]->data, num );
    priv->channels[ channel ]->size = num;
    }

  priv->channels[ channel ]->num = num;
  if( num > 0 )
    {
    memcpy( priv->channels[ channel ]->data, values, num * sizeof( gfloat ) );
    priv->channels[ channel ]->show = TRUE;
    }

}


void gp_ica_scope_update( GpIcaScope *scope )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  priv->scope_update = TRUE;

}


// Это НЕ configure-event.
void gp_ica_scope_configure( GpIcaScope *scope )
{

  GpIcaScopePriv *priv = GP_ICA_SCOPE_GET_PRIVATE( scope );

  gchar *font_name;
  g_object_get( gtk_settings_get_default(), "gtk-font-name", &font_name, NULL );

  pango_font_description_free( priv->font_desc );
  priv->font_desc = pango_font_description_from_string( font_name );

  if( priv->x_axis_font != NULL ) pango_layout_set_font_description( priv->x_axis_font, priv->font_desc );
  if( priv->y_axis_font != NULL ) pango_layout_set_font_description( priv->y_axis_font, priv->font_desc );
  if( priv->info_font != NULL ) pango_layout_set_font_description( priv->info_font, priv->font_desc );

  if(G_UNLIKELY(!priv->colors_up_to_date))
    gp_ica_scope_update_colors(scope);

  priv->scope_update = TRUE;
  priv->x_axis_update = TRUE;
  priv->x_pos_update = TRUE;
  priv->y_axis_update = TRUE;
  priv->y_pos_update = TRUE;
  priv->info_update = TRUE;

  g_free( font_name );

}


static void gp_ica_scope_interface_init( GpIcaRendererInterface *iface )
{

  iface->get_renderers_num = gp_ica_scope_get_renderers_num;
  iface->get_renderer_type = gp_ica_scope_get_renderer_type;
  iface->get_name = gp_ica_scope_get_name;

  iface->set_surface = gp_ica_scope_set_surface;
  iface->set_area_size = NULL;
  iface->set_visible_size = NULL;

  iface->get_completion = NULL;
  iface->set_total_completion = NULL;
  iface->render = gp_ica_scope_render;
  iface->set_shown_limits = NULL;

  iface->set_border = NULL;
  iface->set_swap = NULL;
  iface->set_angle = NULL;
  iface->set_shown = gp_ica_scope_set_shown;
  iface->set_pointer = gp_ica_scope_set_pointer;

  iface->button_press_event = NULL;
  iface->button_release_event = NULL;
  iface->key_press_event = NULL;
  iface->key_release_event = NULL;
  iface->motion_notify_event = NULL;

}
