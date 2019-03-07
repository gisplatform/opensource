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
 * \file cifroarea.c
 *
 * \author Andrei Fadeev
 * \date 5.04.2013
 * \brief Исходный файл GTK+ виджета показа изображений сформированных интерфейсом IcaRenderer.
 *
*/

#include "gp-cifroarea.h"

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <math.h>

typedef enum
{
  GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_PRESS = 0,
  GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_RELEASE,
  GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_PRESS,
  GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_RELEASE,
  GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_MTN_NOTIFY,

  GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_AMOUNT
}
GpCifroAreaaLayerSignalHendlerType;

typedef struct GpCifroAreaLayer {

  GpIcaRenderer *renderer;                  // Отрисовщик слоя.
  gint               renderer_id;               // Идентификатор отрисовщика.
  gint               renderer_type;             // Тип отрисовщика.

  cairo_surface_t   *surface;                   // Область для отрисовки данных.
  gint               render_state;              // Состояние отрисовки данных.
  gint               x;                         // Кордината x начала области отрисованных данных.
  gint               y;                         // Кордината y начала области отрисованных данных.
  gint               width;                     // Ширина отрисованных данных.
  gint               height;                    // Высота отрисованных данных.

  gboolean           drawed;                    // Нарисован этот слой на экране или нет.
  gboolean           dont_draw;                 // Признак отображения слоя

  gulong             handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_AMOUNT];              //Идентификатор обработчика сигнала

} GpCifroAreaLayer;


typedef struct GpCifroAreaPriv {

  GList             *layers;                    // Указатели на отрисовщики слоев.
  gint               layers_num;                // Число отрисовщиков слоев.

  gboolean           check_layers;              // Принудительно проверить слои на необходимость отрисовки.

  gint               update_interval;           // Время в мс между проверкой на возможность перерисовки.
  guint              update_event_source_id;    // Идентификатор GSource функции проверки на возможность перерисовки, работающей по таймауту.

  gint               completion_total;          // Общий прогресс формирования изображения.

  gint               border_left;               // Размер области обрамления слева.
  gint               border_right;              // Размер области обрамления справа.
  gint               border_top;                // Размер области обрамления сверху.
  gint               border_bottom;             // Размер области обрамления снизу.

  gboolean           clip;                      // Ограничивать или нет отрисовку областью внутри элемента обрамления.
  gint               clip_x;                    // Начало области ограничения слева.
  gint               clip_y;                    // Начало области ограничения сверху.
  gint               clip_width;                // Ширина области ограничения.
  gint               clip_height;               // Высота области ограничения.

  gboolean           swap_x;                    // TRUE - ось x направлена в лево, FALSE - в право.
  gboolean           swap_y;                    // TRUE - ось y направлена в низ, FALSE - в верх.

  gboolean           scale_on_resize;           // Изменять == TRUE или нет масштаб при изменении размера окна.
  gdouble            scale_aspect;              // Соотношение масштабов: scale_x / scale_y. В случае <= 0 - свободное соотношение.

  gboolean           rotation;                  // Разрешён поворот или нет.
  gdouble            angle;                     // Угол поворота изображения в радианах.
  gdouble            angle_cos;                 // Косинус угла поворота изображения.
  gdouble            angle_sin;                 // Синус угла поворота изображения.

  gdouble            from_x;                    // Граница отображения по оси x слева.
  gdouble            to_x;                      // Граница отображения по оси x справа.
  gdouble            from_y;                    // Граница отображения по оси y снизу.
  gdouble            to_y;                      // Граница отображения по оси y сверху.

  gdouble            min_x;                     // Минимально возможное значение по оси x.
  gdouble            max_x;                     // Максимально возможное значение по оси x.
  gdouble            min_y;                     // Минимально возможное значение по оси y.
  gdouble            max_y;                     // Максимально возможное значение по оси y.

  gdouble            cur_scale_x;               // Текущий коэффициент масштаба по оси x.
  gdouble            min_scale_x;               // Минимально возможный коэффициент масштаба по оси x (приближение).
  gdouble            max_scale_x;               // Максимально возможный коэффициент масштаба по оси x (отдаление).
  gdouble            cur_scale_y;               // Текущий коэффициент масштаба по оси y.
  gdouble            min_scale_y;               // Минимально возможный коэффициент масштаба по оси y (приближение).
  gdouble            max_scale_y;               // Максимально возможный коэффициент масштаба по оси y (отдаление).

  gint               widget_width;              // Ширина окна GpCifroArea.
  gint               widget_height;             // Высота окна GpCifroArea.
  gint               visible_width;             // Ширина видимой области лля отрисовки данных.
  gint               visible_height;            // Высота видимой области для отрисовки данных.
  gint               min_drawing_area_width;    // Минимальная ширина видимой области при которой происходит отрисовка.
  gint               min_drawing_area_height;   // Минимальная высота видимой области при которой происходит отрисовка.

  gint               pointer_x;                 // Текущие координаты курсора или -1 при нахождении за пределами окна.
  gint               pointer_y;                 // Текущие координаты курсора или -1 при нахождении за пределами окна.

  gboolean           zoom_on_center;            // Устанавливать центр области масштабирования по курсору или нет.
  gdouble            zoom_scale;                // Коэффициент изменения масштаба.

  gdouble           *zoom_x_scales;             // Набор коэффициентов масштабирования по оси x.
  gdouble           *zoom_y_scales;             // Набор коэффициентов масштабирования по оси y.
  gint               num_scales;                // Число коэффициентов масштабирования.
  gint               cur_zoom_index;            // Текущий индекс коэффициента масштабирования.

  gboolean           draw_focus;                // Рисовать или нет индикатор наличия фокуса ввода.
  gdouble            focus_color_red;           // Красный компонент цвета индикатора наличия фокуса ввода.
  gdouble            focus_color_green;         // Зелёный компонент цвета индикатора наличия фокуса ввода.
  gdouble            focus_color_blue;          // Синий компонент цвета индикатора наличия фокуса ввода.
  gdouble            focus_color_alpha;         // Прозрачность цвета индикатора наличия фокуса ввода.

  gdouble            move_multiplier;           // Значение множителя скорости перемещения при нажатой клавише control.
  gdouble            rotate_multiplier;         // Значение множителя скорости вращения при нажатой клавише control.

  gboolean           move_area;                 // Признак перемещения при нажатой клавише мыши.
  gint               move_start_x;              // Начальная координата x перемещения.
  gint               move_start_y;              // Начальная координата y перемещения.

  GdkCursor         *current_cursor;            // Текущий курсор.
  GdkCursor         *point_cursor;              // Курсор используемый при нахождении мышки в видимой области.
  GdkCursor         *move_cursor;               // Курсор используемый при перемещении видимой области.

} GpCifroAreaPriv;

#define GP_CIFRO_AREA_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), GTK_TYPE_GP_CIFRO_AREA, GpCifroAreaPriv ) )


static void gp_cifro_area_set_property (GpCifroArea *carea, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gp_cifro_area_finalize (GpCifroArea *carea);
static GObject* gp_cifro_area_constructor (GType type, guint n_construct_properties,
                                          GObjectConstructParam *construct_properties);

static gint gp_cifro_area_get_visible_width (gdouble width, gdouble height, gdouble angle);
static gint gp_cifro_area_get_visible_height (gdouble width, gdouble height, gdouble angle);
static gboolean gp_cifro_area_fix_scale_aspect (gdouble scale_aspect, gdouble *min_scale_x, gdouble *max_scale_x,
                                                gdouble *min_scale_y, gdouble *max_scale_y);
static gboolean gp_cifro_area_check_drawing (GpCifroAreaPriv *priv, gint widget_width, gint widget_height);
static void gp_cifro_area_update_visible (GpCifroAreaPriv *priv);

static gboolean gp_cifro_area_check_layers (GpCifroArea *carea);

static gboolean gp_cifro_area_leave (GtkWidget *widget, GdkEventCrossing *event, GpCifroAreaPriv *priv);
static gboolean gp_cifro_area_key_press (GtkWidget *widget, GdkEventKey *event, GpCifroAreaPriv *priv);
static gboolean gp_cifro_area_button_press_release (GtkWidget *widget, GdkEventButton *event, GpCifroAreaPriv *priv);
static gboolean gp_cifro_area_motion (GtkWidget *widget, GdkEventMotion *event, GpCifroAreaPriv *priv);
static gboolean gp_cifro_area_scroll (GtkWidget *widget, GdkEventScroll *event, GpCifroAreaPriv *priv);

static gboolean gp_cifro_area_configure (GtkWidget *widget, GdkEventConfigure *event, GpCifroAreaPriv *priv);
static gboolean draw_cb(GtkWidget *widget, cairo_t *event, GpCifroAreaPriv *priv);
static void     draw_layers(GpCifroAreaPriv *priv, cairo_t *cairo);

static void gp_cifro_area_size_allocate (GtkWidget *widget, GtkAllocation *allocation, GpCifroAreaPriv *priv);
static void gp_cifro_area_realize (GtkWidget *widget, GpCifroAreaPriv *priv);
static void gp_cifro_area_send_configure (GpCifroArea *darea);


enum { PROP_O, PROP_UPDATE_INTERVAL };

G_DEFINE_TYPE( GpCifroArea, gp_cifro_area, GTK_TYPE_EVENT_BOX )


// Настройка класса.
static void gp_cifro_area_class_init (GpCifroAreaClass *klass)
{

  GObjectClass *this_class = G_OBJECT_CLASS( klass );

  g_type_class_add_private( klass, sizeof(GpCifroAreaPriv) );

  this_class->constructor = (void*) gp_cifro_area_constructor;
  this_class->set_property = (void*) gp_cifro_area_set_property;
  this_class->finalize = (void*) gp_cifro_area_finalize;

  g_object_class_install_property( this_class, PROP_UPDATE_INTERVAL,
    g_param_spec_int( "update-interval", "Update interval", "Time interval between updates", 1, 1000, 40, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY ) );

}


// Инициализация объекта.
static void gp_cifro_area_init (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  memset( priv, 0, sizeof(GpCifroAreaPriv) );

}


// Функция установки параметров.
static void gp_cifro_area_set_property (GpCifroArea *carea, guint prop_id, const GValue *value, GParamSpec *pspec)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  switch ( prop_id )
    {

    case PROP_UPDATE_INTERVAL:
      priv->update_interval = g_value_get_int( value );
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID( carea, prop_id, pspec );
      break;

    }

}


// Конструктор объекта.
static GObject* gp_cifro_area_constructor (GType g_type, guint n_construct_properties,
                                          GObjectConstructParam *construct_properties)
{

  GObject *carea = G_OBJECT_CLASS( gp_cifro_area_parent_class )->constructor( g_type, n_construct_properties, construct_properties );
  if( carea == NULL ) return NULL;

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  gint event_mask = 0;

  priv->scale_on_resize = FALSE;
  priv->scale_aspect = 0.0;

  priv->rotation = TRUE;
  priv->angle = 0.0;
  priv->angle_cos = cos( priv->angle );
  priv->angle_sin = sin( priv->angle );

  priv->cur_scale_x = 0.0;
  priv->min_scale_x = 0.001;
  priv->max_scale_x = 1000.0;
  priv->cur_scale_y = 0.0;
  priv->min_scale_y = 0.001;
  priv->max_scale_y = 1000.0;

  priv->from_x = -1.0;
  priv->to_x =    1.0;
  priv->from_y = -1.0;
  priv->to_y =    1.0;

  priv->min_x = -1.0;
  priv->max_x =  1.0;
  priv->min_y = -1.0;
  priv->max_y =  1.0;

  priv->min_drawing_area_width = 32;
  priv->min_drawing_area_height = 32;

  priv->pointer_x = -1;
  priv->pointer_y = -1;

  priv->zoom_on_center = FALSE;
  priv->zoom_scale = 10.0;

  priv->draw_focus = TRUE;
  priv->focus_color_red = 0.33;
  priv->focus_color_green = 0.33;
  priv->focus_color_blue = 0.33;
  priv->focus_color_alpha = 1.0;

  priv->move_multiplier = 10.0;
  priv->rotate_multiplier = 2.0;

  priv->current_cursor = NULL;
  priv->point_cursor = gdk_cursor_new_for_display( gdk_display_get_default(), GDK_CROSSHAIR );
  priv->move_cursor = gdk_cursor_new_for_display( gdk_display_get_default(), GDK_FLEUR );

  event_mask |= GDK_LEAVE_NOTIFY_MASK;
  event_mask |= GDK_KEY_PRESS_MASK;
  event_mask |= GDK_BUTTON_PRESS_MASK;
  event_mask |= GDK_BUTTON_RELEASE_MASK;
  event_mask |= GDK_POINTER_MOTION_MASK;
  event_mask |= GDK_POINTER_MOTION_HINT_MASK;
  event_mask |= GDK_SCROLL_MASK;
  gtk_widget_add_events( GTK_WIDGET( carea ), event_mask );
  gtk_widget_set_can_focus( GTK_WIDGET( carea ), TRUE );

  // G_PRIORITY_DEFAULT_IDLE - имеет приоритет меньше чем операции перерисовки GTK+.
  priv->update_event_source_id = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,
                                                     priv->update_interval, (GSourceFunc) gp_cifro_area_check_layers,
                                                     carea, NULL);

  g_signal_connect( carea, "configure-event", G_CALLBACK (gp_cifro_area_configure), priv );
  g_signal_connect_after( carea , "leave-notify-event", G_CALLBACK (gp_cifro_area_leave), priv );
  g_signal_connect_after( carea , "key-press-event", G_CALLBACK (gp_cifro_area_key_press), priv );
  g_signal_connect_after( carea , "button-press-event", G_CALLBACK (gp_cifro_area_button_press_release), priv );
  g_signal_connect_after( carea , "button-release-event", G_CALLBACK (gp_cifro_area_button_press_release), priv );
  g_signal_connect_after( carea , "motion-notify-event", G_CALLBACK (gp_cifro_area_motion), priv );
  g_signal_connect_after( carea , "scroll-event", G_CALLBACK (gp_cifro_area_scroll), priv );
  g_signal_connect( carea , "draw", G_CALLBACK (draw_cb), priv );

  g_signal_connect( carea, "realize", G_CALLBACK (gp_cifro_area_realize), priv );
  g_signal_connect( carea, "size-allocate", G_CALLBACK (gp_cifro_area_size_allocate), priv );

  return carea;

}


static void gp_cifro_area_realize (GtkWidget *widget, GpCifroAreaPriv *priv)
{
  GtkAllocation allocation;
  GdkWindow *window;
  GdkWindowAttr attributes;
  gint attributes_mask;

  if (!gtk_widget_get_has_window (widget))
  {
    G_OBJECT_CLASS( gp_cifro_area_parent_class )->finalize( G_OBJECT(GP_CIFRO_AREA(widget)));
  }
  else
  {
    gtk_widget_set_realized (widget, TRUE);
    gtk_widget_get_allocation (widget, &allocation);
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

    window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
    gtk_widget_register_window (widget, window);
    gtk_widget_set_window (widget, window);

    gtk_style_context_set_background (gtk_widget_get_style_context (widget), window);
  }

  gp_cifro_area_send_configure (GP_CIFRO_AREA (widget));

}


static void gp_cifro_area_size_allocate (GtkWidget *widget, GtkAllocation *allocation, GpCifroAreaPriv *priv)
{
  //g_return_if_fail (GP_IS_CIFRO_AREA (widget));
  g_return_if_fail (allocation != NULL);

  gtk_widget_set_allocation (widget, allocation);

  if (gtk_widget_get_realized (widget))
  {
    if (gtk_widget_get_has_window (widget))
      gdk_window_move_resize (gtk_widget_get_window (widget), allocation->x, allocation->y,
                              allocation->width, allocation->height);

    if (allocation->width != priv->widget_width || allocation->height != priv->widget_height)
      gp_cifro_area_send_configure (GP_CIFRO_AREA (widget));
  }

}


static void gp_cifro_area_send_configure (GpCifroArea *carea)
{
  GtkAllocation allocation;
  GtkWidget *widget;
  GdkEvent *event = gdk_event_new (GDK_CONFIGURE);

  widget = GTK_WIDGET (carea);
  gtk_widget_get_allocation (widget, &allocation);

  event->configure.window = g_object_ref (gtk_widget_get_window (widget));
  event->configure.send_event = TRUE;
  event->configure.x = allocation.x;
  event->configure.y = allocation.y;
  event->configure.width = allocation.width;
  event->configure.height = allocation.height;

  gtk_widget_event (widget, event);
  gdk_event_free (event);

}


// Освобождение памяти, занятой объектом.
static void gp_cifro_area_finalize (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  GpCifroAreaLayer *layer;
  gint i;

  g_source_remove( priv->update_event_source_id );

  // Уничтожаем в обратном порядке,
  // чтобы в деструктуре GpIcaRenderer'а можно было использовать
  // GpIcaRenderer'ы, добавленные до него (например, тот же GpIcaStateRenderer).
  for( i = priv->layers_num - 1; i >= 0; i-- )
    {
    layer = g_list_nth_data( priv->layers, i );
    g_object_unref( layer->renderer );
    if( layer->renderer_type != GP_ICA_RENDERER_STATE)
      cairo_surface_destroy( layer->surface );
    }

  g_list_free( priv->layers );

  if( priv->point_cursor )
    g_object_unref( priv->point_cursor );
  if( priv->move_cursor )
    g_object_unref( priv->move_cursor );

  G_OBJECT_CLASS( gp_cifro_area_parent_class )->finalize( G_OBJECT( carea ) );

}


/* Функция расчёта ширины видимой области для прямоугольника width x height, повёрнутого на угол angle. */
static gint gp_cifro_area_get_visible_width (gdouble width, gdouble height, gdouble angle)
{

  gint visible_width;
  angle = fabs( angle );
  if( angle > G_PI_2 ) angle = G_PI - angle;
  visible_width = round( fabs( cos( G_PI_2 - atan( width / height ) - angle ) ) * sqrt( width * width + height * height ) );
  visible_width += ( visible_width % 2 );
  return visible_width;

}


/* Функция расчёта высоты видимой области для прямоугольника width x height, повёрнутого на угол angle. */
static gint gp_cifro_area_get_visible_height (gdouble width, gdouble height, gdouble angle)
{

  gint visible_height;
  angle = fabs( angle );
  if( angle > G_PI_2 ) angle = G_PI - angle;
  visible_height = round( fabs( cos( atan( width / height ) - angle ) ) * sqrt( width * width + height * height ) );
  visible_height += ( visible_height % 2 );
  return visible_height;

}


/* Функция коррекции диапазонов масштабов при фиксированых соотношениях. */
static gboolean gp_cifro_area_fix_scale_aspect (gdouble scale_aspect, gdouble *min_scale_x, gdouble *max_scale_x,
                                                gdouble *min_scale_y, gdouble *max_scale_y)
{

  gdouble new_min_scale_x;
  gdouble new_max_scale_x;
  gdouble new_min_scale_y;
  gdouble new_max_scale_y;

  if( scale_aspect <= 0.0 ) return TRUE;

  new_min_scale_y = CLAMP( *min_scale_x * scale_aspect, *min_scale_y, *max_scale_y );
  new_max_scale_y = CLAMP( *max_scale_x * scale_aspect, *min_scale_y, *max_scale_y );

  if( new_min_scale_y == new_max_scale_y ) return FALSE;

  new_min_scale_x = CLAMP( new_min_scale_y / scale_aspect, *min_scale_x, *max_scale_x );
  new_max_scale_x = CLAMP( new_max_scale_y / scale_aspect, *min_scale_x, *max_scale_x );

  if( new_min_scale_x == new_max_scale_x ) return FALSE;

  *min_scale_x = new_min_scale_x;
  *max_scale_x = new_max_scale_x;
  *min_scale_y = new_min_scale_y;
  *max_scale_y = new_max_scale_y;

  return TRUE;

}


/* Функция расчёта значения в точке. */
static void gp_cifro_area_point_to_value (GpCifroAreaPriv *priv, gdouble x, gdouble y, gdouble *x_val, gdouble *y_val)
{

  gdouble x_val_tmp;
  gdouble y_val_tmp;

  x = x - priv->widget_width / 2.0;
  y = priv->widget_height / 2.0 - y;

  if( priv->angle != 0.0 )
    {
    x_val_tmp = ( x * priv->angle_cos - y * priv->angle_sin ) * priv->cur_scale_x;
    y_val_tmp = ( y * priv->angle_cos + x * priv->angle_sin ) * priv->cur_scale_y;
    }
  else
    {
    x_val_tmp = x * priv->cur_scale_x;
    y_val_tmp = y * priv->cur_scale_y;
    }

  if( priv->swap_x ) x_val_tmp = -x_val_tmp;
  if( priv->swap_y ) y_val_tmp = -y_val_tmp;

  x_val_tmp = ( ( priv->to_x - priv->from_x ) / 2.0 ) + priv->from_x + x_val_tmp;
  y_val_tmp = ( ( priv->to_y - priv->from_y ) / 2.0 ) + priv->from_y + y_val_tmp;

  *x_val = x_val_tmp;
  *y_val = y_val_tmp;

}


/* Функция проверки возможности отрисовки для текущего размера виджета. */
static gboolean gp_cifro_area_check_drawing (GpCifroAreaPriv *priv, gint widget_width, gint widget_height)
{

  gint area_width = widget_width - priv->border_left - priv->border_right;
  gint area_height = widget_height - priv->border_top - priv->border_bottom;

  if( area_width <= priv->min_drawing_area_width || area_height <= priv->min_drawing_area_height ) return FALSE;

  return TRUE;

}


/* Функция перерасчёта параметров отображения. */
static void gp_cifro_area_update_visible (GpCifroAreaPriv *priv)
{
  gdouble width;
  gdouble height;
  gdouble visible_width;
  gdouble visible_height;
  gdouble new_scale_x;
  gdouble new_scale_y;
  gdouble x_width;
  gdouble y_height;
  gdouble x, y;

  GpCifroAreaLayer *layer;
  gint i;

  if( !gp_cifro_area_check_drawing (priv, priv->widget_width, priv->widget_height) ) return;

  // Размер видимой области отображения.
  width = priv->widget_width - priv->border_left - priv->border_right;
  height = priv->widget_height - priv->border_top - priv->border_bottom;

  // Размер видимой области отображения с учётом поворота.
  visible_width = gp_cifro_area_get_visible_width (width, height, priv->angle);
  visible_height = gp_cifro_area_get_visible_height (width, height, priv->angle);

  // Расчёт масштабов в начале и при задании области видимости.
  if( ( priv->cur_scale_x == 0.0 ) && ( priv->cur_scale_y == 0.0 ) )
    {
    priv->cur_scale_x = ( priv->to_x - priv->from_x ) / visible_width;
    priv->cur_scale_y = ( priv->to_y - priv->from_y ) / visible_height;
    }

  // Коррекция масштабов в случае заданного соотношения.
  if( priv->scale_aspect > 0.0 && priv->num_scales == 0 )
    {

    new_scale_x = priv->cur_scale_y / priv->scale_aspect;
    new_scale_y = priv->cur_scale_x * priv->scale_aspect;
    new_scale_x = CLAMP( new_scale_x, priv->min_scale_x, priv->max_scale_x );
    new_scale_y = CLAMP( new_scale_y, priv->min_scale_y, priv->max_scale_y );

    if( new_scale_x < new_scale_y )
      {
      priv->cur_scale_x = new_scale_x;
      priv->cur_scale_y = new_scale_x / priv->scale_aspect;
      }
    else
      {
      priv->cur_scale_y = new_scale_y;
      priv->cur_scale_x = new_scale_y * priv->scale_aspect;
      }

    }

  // Проверка на выход за границы, кроме случая фиксированных значений.
  if( priv->num_scales == 0 )
    {
    priv->cur_scale_x = CLAMP( priv->cur_scale_x, priv->min_scale_x, priv->max_scale_x );
    priv->cur_scale_y = CLAMP( priv->cur_scale_y, priv->min_scale_y, priv->max_scale_y );
    }

  // Расчёт области видимости для получившихся масштабов и проверка
  // на выход за границы допустимых значений.
  x = priv->from_x + ( priv->to_x - priv->from_x ) / 2.0;
  y = priv->from_y + ( priv->to_y - priv->from_y ) / 2.0;

  x_width = priv->cur_scale_x * visible_width;
  y_height = priv->cur_scale_y * visible_height;

  if( x_width >= priv->max_x - priv->min_x )
    {
    priv->from_x = priv->min_x;
    priv->to_x = priv->max_x;
    }
  else
    {
    priv->from_x = x - x_width / 2.0;
    priv->to_x = x + x_width / 2.0;
    if( priv->from_x < priv->min_x )
      {
      priv->from_x = priv->min_x;
      priv->to_x = priv->min_x + x_width;
      }
    if( priv->to_x > priv->max_x )
      {
      priv->from_x = priv->max_x - x_width;
      priv->to_x = priv->max_x;
      }
    }

  if( y_height >= priv->max_y - priv->min_y )
    {
    priv->from_y = priv->min_y;
    priv->to_y = priv->max_y;
    }
  else
    {
    priv->from_y = y - y_height / 2.0;
    priv->to_y = y + y_height / 2.0;
    if( priv->from_y < priv->min_y )
      {
      priv->from_y = priv->min_y;
      priv->to_y = priv->min_y + y_height;
      }
    if( priv->to_y > priv->max_y )
      {
      priv->from_y = priv->max_y - y_height;
      priv->to_y = priv->max_y;
      }
    }

  // Перерасчёт размеров видимой области в соответствии со скорректированной областью видимости.
  visible_width = ( priv->to_x - priv->from_x ) / priv->cur_scale_x;
  visible_height = ( priv->to_y - priv->from_y ) / priv->cur_scale_y;
  priv->visible_width = round( visible_width );
  priv->visible_height = round( visible_height );

  // Установка параметров отрисовщиков GpIcaRenderer.
  for( i = 0; i < priv->layers_num; i++ )
    {

    gint surface_width;
    gint surface_height;

    layer = g_list_nth_data( priv->layers, i );

    surface_width = ( layer->renderer_type == GP_ICA_RENDERER_AREA) ? priv->widget_width : visible_width;
    surface_height = ( layer->renderer_type == GP_ICA_RENDERER_AREA) ? priv->widget_height : visible_height;

    if( layer->renderer_type != GP_ICA_RENDERER_STATE)
      {

      if( layer->surface )
        if( ( surface_width > cairo_image_surface_get_width( layer->surface ) ) || ( surface_height > cairo_image_surface_get_height( layer->surface ) ) )
          {
          cairo_surface_destroy( layer->surface );
          layer->surface = NULL;
          }

      if( !layer->surface )
        {

        layer->surface = cairo_image_surface_create( CAIRO_FORMAT_ARGB32, surface_width, surface_height );
          gp_ica_renderer_set_surface(layer->renderer, layer->renderer_id,
                                      cairo_image_surface_get_data(layer->surface),
                                      surface_width, surface_height,
                                      cairo_image_surface_get_stride(layer->surface));
        priv->check_layers = TRUE;

        }

      }

    if( layer->renderer_id > 0 ) continue;

      gp_ica_renderer_set_shown(layer->renderer, priv->from_x, priv->to_x, priv->from_y, priv->to_y);
      gp_ica_renderer_set_visible_size(layer->renderer, surface_width, surface_height);
      gp_ica_renderer_set_angle(layer->renderer, priv->angle);

    }

  // Границы маски рисования слоёв с поворотом.
  if( priv->border_left || priv->border_right || priv->border_top || priv->border_bottom )
    {
    priv->clip_x = priv->border_left;
    priv->clip_y = priv->border_top;
    priv->clip_width = priv->widget_width - priv->border_left - priv->border_right;
    priv->clip_height = priv->widget_height - priv->border_top - priv->border_bottom;
    priv->clip = TRUE;
    }
  else
    priv->clip = FALSE;

}


/* Функция проверки слоёв на необходимость отрисовки. */
static gboolean gp_cifro_area_check_layers (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );
  GpCifroAreaLayer *layer;

  gint completion_total;
  gint completion_cur;
  gint completion_nums;

  gint new_renderer_state;
  gboolean queue_draw = FALSE;
  gint i;

  completion_total = 0;
  completion_nums = 0;

  for( i = 0; i < priv->layers_num; i++ )
    {

    layer = g_list_nth_data( priv->layers, i );

    if(layer->dont_draw == TRUE) continue;

    completion_cur = gp_ica_renderer_get_completion(layer->renderer, layer->renderer_id);
    if( completion_cur >= 0 )
      {
      completion_total += completion_cur;
      completion_nums ++;
      }

    }

  if( completion_nums )
    completion_total = (gint)( (gdouble)completion_total / (gdouble)completion_nums );
  else
    completion_total = 1000;

  for( i = 0; i < priv->layers_num; i++ )
    {

    layer = g_list_nth_data( priv->layers, i );

    if(layer->dont_draw == TRUE) continue;

    if( priv->completion_total != completion_total )
      gp_ica_renderer_set_total_completion(layer->renderer, completion_total);

    if( !layer->surface ) continue;
    if( layer->renderer_type == GP_ICA_RENDERER_STATE) continue;

    cairo_surface_flush( layer->surface );
    new_renderer_state = gp_ica_renderer_render(layer->renderer, layer->renderer_id, &layer->x, &layer->y,
                                                &layer->width, &layer->height);
    cairo_surface_mark_dirty( layer->surface );

    if( new_renderer_state == GP_ICA_RENDERER_AVAIL_ALL) queue_draw = TRUE;
    if( new_renderer_state != layer->render_state && new_renderer_state != GP_ICA_RENDERER_AVAIL_NOT_CHANGED) queue_draw = TRUE;
    layer->render_state = new_renderer_state;

    }

  priv->completion_total = completion_total;

  if( queue_draw && !priv->check_layers )
    gtk_widget_queue_draw( GTK_WIDGET( carea ) );

  priv->check_layers = FALSE;

  return TRUE;

}


/* Обработчик события выхода мышки за пределы виджета. */
static gboolean gp_cifro_area_leave (GtkWidget *widget, GdkEventCrossing *event, GpCifroAreaPriv *priv)
{

  GpCifroAreaLayer *layer;
  gint i;

  for( i = 0; i < priv->layers_num; i++ )
    {
    layer = g_list_nth_data( priv->layers, i );

    if(layer->dont_draw == TRUE) continue;

    if( layer->renderer_id > 0 ) continue;
      gp_ica_renderer_set_pointer(layer->renderer, -1, -1, 0.0, 0.0);
    }

  priv->pointer_x = -1;
  priv->pointer_y = -1;

  return TRUE;

}


/* Обработчик нажатия кнопок клавиатуры. */
static gboolean gp_cifro_area_key_press (GtkWidget *widget, GdkEventKey *event, GpCifroAreaPriv *priv)
{

  /* Перемещение области. */
  if( ( ( event->keyval == GDK_KEY_Left ) || ( event->keyval == GDK_KEY_Right  ) || ( event->keyval == GDK_KEY_Up ) || ( event->keyval == GDK_KEY_Down ) ) && !( event->state & GDK_SHIFT_MASK ) )
    {

    gdouble multiplier = ( event->state & GDK_CONTROL_MASK ) ? priv->move_multiplier : 1.0;
    gdouble step_x = 0;
    gdouble step_y = 0;

    if( event->keyval == GDK_KEY_Left ) { step_x = multiplier; }
    if( event->keyval == GDK_KEY_Right ) { step_x = -multiplier; }
    if( event->keyval == GDK_KEY_Up ) { step_y = -multiplier; }
    if( event->keyval == GDK_KEY_Down ) { step_y = multiplier; }

    gp_cifro_area_move (GP_CIFRO_AREA(widget), step_x, step_y);

    return TRUE;

    }

  /* Поворот области. */
  if( ( event->state & GDK_SHIFT_MASK ) && ( ( event->keyval == GDK_KEY_Left ) || ( event->keyval == GDK_KEY_Right  ) ) )
    {

    gdouble multiplier = ( event->state & GDK_CONTROL_MASK ) ? priv->rotate_multiplier : 1.0;
    gdouble angle = 0.0;

    if( event->keyval == GDK_KEY_Left ) angle = -G_PI / 180.0 * multiplier;
    if( event->keyval == GDK_KEY_Right ) angle = G_PI / 180.0 * multiplier;

    gp_cifro_area_rotate (GP_CIFRO_AREA(widget), angle);

    return TRUE;

    }

  /* Масштабирование области. */
  if( ( event->keyval == GDK_KEY_KP_Add ) || ( event->keyval == GDK_KEY_KP_Subtract ) || ( event->keyval == GDK_KEY_plus ) || ( event->keyval == GDK_KEY_minus ) )
    {

    gboolean zoom_x = TRUE;
    gboolean zoom_y = TRUE;
    gboolean zoom_in = TRUE;
    gdouble val_x, val_y;

    if( event->state & GDK_CONTROL_MASK ) zoom_x = FALSE;
    if( event->state & GDK_MOD1_MASK ) zoom_y = FALSE;
    if( ( event->keyval == GDK_KEY_KP_Subtract ) || ( event->keyval == GDK_KEY_minus ) ) zoom_in = FALSE;

    val_x = priv->from_x + ( priv->to_x - priv->from_x ) / 2.0;
    val_y = priv->from_y + ( priv->to_y - priv->from_y ) / 2.0;

    if( priv->num_scales )
      gp_cifro_area_fixed_zoom (GP_CIFRO_AREA(widget), val_x, val_y, zoom_in);
    else
      gp_cifro_area_zoom (GP_CIFRO_AREA(widget), zoom_x, zoom_y, val_x, val_y, zoom_in, priv->zoom_scale);

    return TRUE;

    }

  return FALSE;

}


/* Обработчик нажатия кнопок мышки. */
static gboolean gp_cifro_area_button_press_release (GtkWidget *widget, GdkEventButton *event, GpCifroAreaPriv *priv)
{

  if( event->type == GDK_BUTTON_PRESS && ( event->button == 1 ) )
  {
    gtk_widget_grab_focus(widget);

    // Нажата левая клавиша мышки в области обрамления - переходим в режим перемещения.
    if( ( event->x > priv->clip_x ) && ( event->x < priv->clip_x + priv->clip_width ) &&
        ( event->y > priv->clip_y ) && ( event->y < priv->clip_y + priv->clip_height ) )
      {

      priv->move_area = TRUE;
      priv->move_start_x = event->x;
      priv->move_start_y = event->y;

      gdk_window_set_cursor( gtk_widget_get_window(widget), priv->move_cursor );

      return TRUE;

      }
   }

  /* Выключаем режим перемещения. */
  if( event->type == GDK_BUTTON_RELEASE && ( event->button == 1 ) )
    {

    priv->move_area = FALSE;

    gdk_window_set_cursor( gtk_widget_get_window(widget), priv->point_cursor );

    return TRUE;

    }

  return FALSE;

}


/* Обработчик перемещений курсора мыши. */
static gboolean gp_cifro_area_motion (GtkWidget *widget, GdkEventMotion *event, GpCifroAreaPriv *priv)
{

  gint x = event->x;
  gint y = event->y;

  /* Запоминаем текущие координаты курсора. */
  priv->pointer_x = x;
  priv->pointer_y = y;

  /* Устанавливаем вид курсора в зависимости от области нахождения. */
  if( !priv->move_area )
    {
    if( ( x > priv->clip_x ) && ( x < priv->clip_x + priv->clip_width ) &&
        ( y > priv->clip_y ) && ( y < priv->clip_y + priv->clip_height ) )
      gdk_window_set_cursor( gtk_widget_get_window(widget), priv->point_cursor );
    else
      {
      gdk_window_set_cursor( gtk_widget_get_window(widget), NULL );
      }
    }

  /* Режим перемещения - сдвигаем область. */
  if( priv->move_area )
    {

      gp_cifro_area_move (GP_CIFRO_AREA(widget), priv->move_start_x - x, y - priv->move_start_y);

    priv->move_start_x = x;
    priv->move_start_y = y;

    }
  /* Режим информирования о координатах курсора - сообщаем отрисовщикам. */
  else
    {

    GpCifroAreaLayer *layer;
    gdouble val_x, val_y;
    gint i;

    gp_cifro_area_point_to_value (priv, x, y, &val_x, &val_y);

    for( i = 0; i < priv->layers_num; i++ )
      {
        layer = g_list_nth_data( priv->layers, i );

        if( layer->renderer_id > 0 || layer->dont_draw == TRUE) continue;

        gp_ica_renderer_set_pointer(layer->renderer, x, y, val_x, val_y);
      }

    }

  gdk_event_request_motions( event );

  return FALSE;

}


/* Обработчик событий прокрутки колёсика мышки - масштабирование. */
static gboolean gp_cifro_area_scroll (GtkWidget *widget, GdkEventScroll *event, GpCifroAreaPriv *priv)
{
  gboolean zoom_x;
  gboolean zoom_y;
  gboolean zoom_in;
  gdouble val_x, val_y;

  // Обрабатываем только события внутри области отображения данных.
  if( ( event->x < priv->clip_x ) || ( event->x > priv->clip_x + priv->clip_width ) ) return TRUE;
  if( ( event->y < priv->clip_y ) || ( event->y > priv->clip_y + priv->clip_height ) ) return TRUE;

  if( !( event->state & GDK_SHIFT_MASK ) && !( event->state & GDK_CONTROL_MASK ) && !( event->state & GDK_MOD1_MASK ) )
    return TRUE;

  // Параметры масштабирования.
  zoom_x = ( event->state & GDK_SHIFT_MASK ) || ( event->state & GDK_MOD1_MASK );
  zoom_y = ( event->state & GDK_SHIFT_MASK ) || ( event->state & GDK_CONTROL_MASK );
  zoom_in = ( event->direction == GDK_SCROLL_UP ) ? TRUE : FALSE;

  // Точка, относительно которой будет производится масштабирование.
  if( priv->zoom_on_center )
    {
    val_x = priv->from_x + ( priv->to_x - priv->from_x ) / 2.0;
    val_y = priv->from_y + ( priv->to_y - priv->from_y ) / 2.0;
    }
  else
    {
      gp_cifro_area_point_to_value (priv, event->x, event->y, &val_x, &val_y);
    if( ( val_x < priv->min_x ) || ( val_x > priv->max_x ) ) return TRUE;
    if( ( val_y < priv->min_y ) || ( val_y > priv->max_y ) ) return TRUE;
    }

  // Масштабирование.
  if( priv->num_scales )
    gp_cifro_area_fixed_zoom (GP_CIFRO_AREA(widget), val_x, val_y, zoom_in);
  else
    gp_cifro_area_zoom (GP_CIFRO_AREA(widget), zoom_x, zoom_y, val_x, val_y, zoom_in, priv->zoom_scale);

  return TRUE;

}


/* Обработчик изменения размеров виджета. */
static gboolean gp_cifro_area_configure (GtkWidget *widget, GdkEventConfigure *event, GpCifroAreaPriv *priv)
{

  gint widget_width = event->width - ( event->width % 2 );
  gint widget_height = event->height - ( event->height % 2 );

  GpCifroAreaLayer *layer;
  gint i;

  if( priv->widget_width == widget_width && priv->widget_height == widget_height ) return TRUE;
  if( !gp_cifro_area_check_drawing (priv, widget_width, widget_height) ) return TRUE;

  // Новые размеры виджета.
  priv->widget_width = widget_width;
  priv->widget_height = widget_height;

  // Информируем о них отрисовщиков.
  for( i = 0; i < priv->layers_num; i++ )
  {
    layer = g_list_nth_data( priv->layers, i );

    if( layer->renderer_id > 0 ) continue;
      gp_ica_renderer_set_area_size(layer->renderer, widget_width, widget_height);
  }

  if( priv->scale_on_resize && ( priv->scale_aspect < 0.0 ) )
    gp_cifro_area_set_shown (GP_CIFRO_AREA(widget), priv->from_x, priv->to_x, priv->from_y, priv->to_y);
  else
    gp_cifro_area_update_visible (priv);

  return FALSE;

}


cairo_surface_t *gp_cifro_area_draw_to_surface(GpCifroArea *carea)
{
  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE(carea);
  g_return_val_if_fail(priv, NULL);

  cairo_surface_t *rval = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, priv->widget_width, priv->widget_height);

  cairo_t *cr = cairo_create(rval);
    draw_layers(priv, cr);
  cairo_destroy(cr);

  return rval;
}


static void draw_layers(GpCifroAreaPriv *priv, cairo_t *cairo)
{
  GpCifroAreaLayer *layer;
  gdouble cairo_width;
  gdouble cairo_height;
  gdouble shift_width;
  gdouble shift_height;
  gdouble angle;
  gdouble db_w, db_h;
  gint i;

  cairo_width = priv->widget_width;
  cairo_height = priv->widget_height;
  shift_width = ( ( priv->widget_width - priv->visible_width ) / 2.0 );
  shift_height = ( ( priv->widget_height - priv->visible_height ) / 2.0 );

  db_w = (priv->border_left - priv->border_right) / 2.0;
  db_h = (priv->border_top - priv->border_bottom) / 2.0;

  angle = priv->angle;
  if( priv->swap_x ) angle = -angle;
  if( priv->swap_y ) angle = -angle;

  cairo_set_operator( cairo, CAIRO_OPERATOR_OVER );

  for( i = 0; i < priv->layers_num; i++ )
  {
    layer = g_list_nth_data( priv->layers, i );

    if( layer->renderer_type == GP_ICA_RENDERER_STATE) continue;

    if( layer->render_state == GP_ICA_RENDERER_AVAIL_NONE ) continue;

    if( layer->dont_draw ) continue;

    cairo_save( cairo );

    if( layer->renderer_type == GP_ICA_RENDERER_VISIBLE)
    {

      if( priv->clip )
      {
        cairo_rectangle( cairo, priv->clip_x, priv->clip_y, priv->clip_width, priv->clip_height );
        cairo_clip( cairo );
      }

      if( priv->swap_x )
      {
        cairo_scale( cairo, -1.0, 1.0 );
        cairo_translate( cairo, -cairo_width - 2 * db_w, 0 );
      }

      if( priv->swap_y )
      {
        cairo_scale( cairo, 1.0, -1.0 );
        cairo_translate( cairo, 0, -cairo_height - 2 * db_h );
      }

      if( priv->angle != 0.0 )
      {
        cairo_translate( cairo, cairo_width / 2.0 + db_w, cairo_height / 2.0 + db_h );
        cairo_rotate( cairo, angle );
        cairo_translate( cairo, -cairo_width / 2.0 - db_w, -cairo_height / 2.0 - db_h );
      }

      cairo_rectangle( cairo, layer->x + shift_width + db_w, layer->y + shift_height + db_h, layer->width, layer->height );
      cairo_clip( cairo );

      cairo_set_source_surface( cairo, layer->surface, shift_width + db_w, shift_height + db_h );
    }
    else if( layer->renderer_type == GP_ICA_RENDERER_AREA)
    {
      cairo_rectangle( cairo, layer->x, layer->y, layer->width, layer->height );
      cairo_clip( cairo );

      cairo_set_source_surface( cairo, layer->surface, 0, 0 );
    }

    cairo_paint( cairo );
    cairo_restore( cairo );
  }
}


/* Обработчик рисования содержимого виджета. */
static gboolean draw_cb(GtkWidget *widget, cairo_t *cairo, GpCifroAreaPriv *priv)
{
  if( priv->check_layers )
    gp_cifro_area_check_layers (GP_CIFRO_AREA(widget));

  cairo_set_operator( cairo, CAIRO_OPERATOR_SOURCE );
  cairo_set_source_rgb( cairo, 0.0, 0.0, 0.0 );
  cairo_paint( cairo );

  draw_layers(priv, cairo);

  if( priv->draw_focus && gtk_widget_is_focus( widget ) )
    {
    gdouble dashes[] = { 4.0, 2.0 };
    cairo_set_source_rgba( cairo, priv->focus_color_red, priv->focus_color_green, priv->focus_color_blue, priv->focus_color_alpha );
    cairo_set_dash( cairo, dashes, 2, 0.0 );
    cairo_rectangle( cairo, 0.5, 0.5, priv->widget_width - 1, priv->widget_height - 1 );
    cairo_stroke( cairo );
    }

  if( gtk_bin_get_child( GTK_BIN(widget)) != NULL)
    gtk_container_propagate_draw( GTK_CONTAINER(widget), gtk_bin_get_child( GTK_BIN(widget)), cairo);

  return FALSE;

}

/* Функция создания объекта GpCifroArea. */
GtkWidget *gp_cifro_area_new (gint update_interval)
{

  GpCifroArea *carea = g_object_new(GTK_TYPE_GP_CIFRO_AREA, "update-interval", update_interval, NULL );
  return GTK_WIDGET( carea );

}


/* Функция добавляет отрисовщик слоя в объект GpCifroArea. */
gint gp_cifro_area_add_layer (GpCifroArea *carea, GpIcaRenderer *layer_renderer)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );
  gint i;
  gulong handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_AMOUNT];

  for( i = 0; i < gp_ica_renderer_get_renderers_num(layer_renderer); i++ )
    {

    gint renderer_type;
    GpCifroAreaLayer *layer;

    renderer_type = gp_ica_renderer_get_renderer_type(layer_renderer, i);
    if( ( renderer_type != GP_ICA_RENDERER_AREA) && ( renderer_type != GP_ICA_RENDERER_VISIBLE) && ( renderer_type !=
                                                                                                     GP_ICA_RENDERER_STATE) ) continue;

    layer = g_malloc( sizeof(GpCifroAreaLayer) );
    layer->surface = NULL;
    layer->dont_draw = FALSE;
    layer->renderer_id = i;
    layer->renderer = layer_renderer;
    layer->renderer_type = renderer_type;
    layer->render_state = GP_ICA_RENDERER_AVAIL_NONE;
    priv->layers = g_list_append( priv->layers, layer );
    priv->layers_num += 1;

    g_object_ref_sink( layer_renderer );

    if( layer->renderer_id > 0 ) continue;

      gp_ica_renderer_set_swap(layer->renderer, priv->swap_x, priv->swap_y);
      gp_ica_renderer_set_shown(layer->renderer, priv->from_x, priv->to_x, priv->from_y, priv->to_y);

    if(i == 0)
    {
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_PRESS] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_PRESS] =
                                  g_signal_connect_swapped( carea, "button-press-event", G_CALLBACK(gp_ica_renderer_button_press_event), layer_renderer );
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_RELEASE] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_RELEASE] =
                                  g_signal_connect_swapped( carea, "button-release-event", G_CALLBACK(gp_ica_renderer_button_release_event), layer_renderer );
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_PRESS] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_PRESS] =
                                  g_signal_connect_swapped( carea, "key-press-event", G_CALLBACK(gp_ica_renderer_key_press_event), layer_renderer );
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_RELEASE] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_RELEASE] =
                                  g_signal_connect_swapped( carea, "key-release-event", G_CALLBACK(gp_ica_renderer_key_release_event), layer_renderer );
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_MTN_NOTIFY] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_MTN_NOTIFY] =
                                  g_signal_connect_swapped( carea, "motion-notify-event", G_CALLBACK(gp_ica_renderer_motion_notify_event), layer_renderer );
    }
    else
    {
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_PRESS] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_PRESS];
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_RELEASE] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_RELEASE];
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_PRESS] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_PRESS];
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_RELEASE] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_RELEASE];
      layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_MTN_NOTIFY] = handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_MTN_NOTIFY];
    }
  }

  gp_cifro_area_update_visible (priv);

  return priv->layers_num;

}

void gp_cifro_area_remove_layer(GpCifroArea *carea, GpIcaRenderer *layer_renderer)
{
  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  GpCifroAreaLayer *layer;
  GList *l = g_list_first(priv->layers);

  while (l != NULL)
  {
    GList *next = l->next;
    layer = g_list_nth_data( l, 0 );

    if(layer->renderer == layer_renderer)
    {
      priv->layers = g_list_delete_link (priv->layers, l);
      priv->layers_num--;
    }

    l = next;
  }
}

/* Функция устанавливает флаг отображения для конкретного слоя*/
void gp_cifro_area_set_layer_dont_draw_flag(GpCifroArea *carea, GpIcaRenderer *layer_renderer, gboolean flag)
{
  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );
  GpCifroAreaLayer *layer;

  GList *l = g_list_first(priv->layers);
  gint n_res = 0;

  while (l != NULL)
  {
    GList *next = l->next;
    layer = g_list_nth_data( l, 0 );

    if(layer->renderer == layer_renderer)
    {
      layer->dont_draw = flag;
      if(!flag)
      {
        if(layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_PRESS] == 0)
          layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_PRESS] = g_signal_connect_swapped( carea, "button-press-event", G_CALLBACK(gp_ica_renderer_button_press_event), layer_renderer );
        if(layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_RELEASE] == 0)
          layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_BTN_RELEASE] = g_signal_connect_swapped( carea, "button-release-event", G_CALLBACK(gp_ica_renderer_button_release_event), layer_renderer );
        if(layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_PRESS] == 0)
          layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_PRESS] = g_signal_connect_swapped( carea, "key-press-event", G_CALLBACK(gp_ica_renderer_key_press_event), layer_renderer );
        if(layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_RELEASE] == 0)
          layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_KEY_RELEASE] = g_signal_connect_swapped( carea, "key-release-event", G_CALLBACK(gp_ica_renderer_key_release_event), layer_renderer );
        if(layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_MTN_NOTIFY] == 0)
          layer->handler_id[GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_MTN_NOTIFY] = g_signal_connect_swapped( carea, "motion-notify-event", G_CALLBACK(gp_ica_renderer_motion_notify_event), layer_renderer );
      }
      else
      {
        int i;
        for(i = 0; i < GP_CIFRO_AREA_LAYER_SIGNAL_HENDLER_TYPE_AMOUNT; i++)
        {
          g_signal_handler_disconnect (carea, layer->handler_id[i]);
          layer->handler_id[i] = 0;
        }
      }
      n_res++;
    }
    l = next;
  }

  if(n_res == priv->layers_num || n_res == 0)
    g_warning("Layer to set flag not found");

  gp_cifro_area_update_visible(priv);
}

/* Функция возвращает флаг отображения для конкретного слоя*/
gboolean gp_cifro_area_get_layer_dont_draw_flag(GpCifroArea *carea, GpIcaRenderer *layer_renderer)
{
  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );
  GpCifroAreaLayer *layer;

  GList *l = g_list_first(priv->layers);
  while (l != NULL)
  {
    GList *next = l->next;
    layer = g_list_nth_data( l, 0 );

    if(layer->renderer == layer_renderer)
      return layer->dont_draw;

    l = next;
  }

  g_warning("Layer to get flag not found");
  return FALSE;
}

/* Функция задаёт размер области для отрисовки элементов обрамления по периметру. */
gboolean gp_cifro_area_set_border (GpCifroArea *carea, gint left, gint right, gint top, gint bottom)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  GpCifroAreaLayer *layer;
  gint i;

  g_return_val_if_fail( left >= 0 && left < 1024, FALSE );
  g_return_val_if_fail( right >= 0 && right < 1024, FALSE );
  g_return_val_if_fail( top >= 0 && top < 1024, FALSE );
  g_return_val_if_fail( bottom >= 0 && bottom < 1024, FALSE );

  priv->border_left = left;
  priv->border_right = right;
  priv->border_top = top;
  priv->border_bottom = bottom;

  for( i = 0; i < priv->layers_num; i++ )
    {
    layer = g_list_nth_data( priv->layers, i );
    if( layer->renderer_id > 0 ) continue;
      gp_ica_renderer_set_border(layer->renderer, left, right, top, bottom);
    }

  gp_cifro_area_update_visible (priv);

  return TRUE;

}


/* Функция возвращает текущий размер области для отрисовки элементов обрамления по периметру. */
void gp_cifro_area_get_border (GpCifroArea *carea, gint *left, gint *right, gint *top, gint *bottom)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( left ) *left = priv->border_left;
  if( right ) *right = priv->border_right;
  if( top ) *top = priv->border_top;
  if( bottom ) *bottom = priv->border_bottom;

}


/* Функция задаёт минимальные ширину и высоту видимой области виджета. */
gboolean gp_cifro_area_set_minimum_visible (GpCifroArea *carea, gint min_width, gint min_height)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  g_return_val_if_fail( min_width > 32 && min_width < 1024, FALSE );
  g_return_val_if_fail( min_height > 32 && min_height < 1024, FALSE );

  priv->min_drawing_area_width = min_width;
  priv->min_drawing_area_height = min_height;

  return TRUE;

}


/* Функция возвращает текущие минимальные ширину и высоту видимой области виджета. */
void gp_cifro_area_get_minimum_visible (GpCifroArea *carea, gint *min_width, gint *min_height)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( min_width ) *min_width = priv->min_drawing_area_width;
  if( min_height ) *min_height = priv->min_drawing_area_height;

}


/* Функция задаёт отражение по осям. */
void gp_cifro_area_set_swap (GpCifroArea *carea, gboolean swap_x, gboolean swap_y)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  GpCifroAreaLayer *layer;
  gint i;

  priv->swap_x = swap_x;
  priv->swap_y = swap_y;

  for( i = 0; i < priv->layers_num; i++ )
    {
    layer = g_list_nth_data( priv->layers, i );
    if( layer->renderer_id > 0 ) continue;
      gp_ica_renderer_set_swap(layer->renderer, swap_x, swap_y);
    }

  gp_cifro_area_update_visible (priv);

}


/* Функция возвращает текущее состояние отражения по осям. */
void gp_cifro_area_get_swap (GpCifroArea *carea, gboolean *swap_x, gboolean *swap_y)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( swap_x ) *swap_x = priv->swap_x;
  if( swap_y ) *swap_y = priv->swap_y;

}


/* Функция устанавливает необходимости рисования рамки при наличии фокуса ввода в объект GpCifroArea. */
void gp_cifro_area_set_draw_focus (GpCifroArea *carea, gboolean draw_focus)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  priv->draw_focus = draw_focus;

}


/* Функция возвращает необходимости рисования рамки при наличии фокуса ввода в объект GpCifroArea. */
gboolean gp_cifro_area_get_draw_focus (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  return priv->draw_focus;

}


/* Функция устанавливает цвета рамки при наличии фокуса ввода в объект GpCifroArea. */
void gp_cifro_area_set_focus_color (GpCifroArea *carea, gdouble red, gdouble green, gdouble blue, gdouble alpha)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  priv->focus_color_red = red;
  priv->focus_color_green = green;
  priv->focus_color_blue = blue;
  priv->focus_color_alpha = alpha;

}


/* Функция возвращает цвета рамки при наличии фокуса ввода в объект GpCifroArea. */
void gp_cifro_area_get_focus_color (GpCifroArea *carea, gdouble *red, gdouble *green, gdouble *blue, gdouble *alpha)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( red ) *red = priv->focus_color_red;
  if( green ) *green = priv->focus_color_green;
  if( blue ) *blue = priv->focus_color_blue;
  if( alpha ) *alpha = priv->focus_color_alpha;

}


/* Функция задает курсор используемый при нахождении мышки в видимой области. */
void gp_cifro_area_set_point_cursor (GpCifroArea *carea, GdkCursor *cursor)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( priv->point_cursor )
    g_object_unref( priv->point_cursor );

  priv->point_cursor = cursor;

}


/* Функция задает курсор используемый при перемещении видимой области. */
void gp_cifro_area_set_move_cursor (GpCifroArea *carea, GdkCursor *cursor)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( priv->move_cursor )
    g_object_unref( priv->move_cursor );

  priv->move_cursor = cursor;

}


/* Функция задаёт возможность поворота изображения вокруг оси. */
void gp_cifro_area_set_rotation (GpCifroArea *carea, gboolean rotation)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  priv->rotation = rotation;

}


/* Функция возвращает информацию о возможности поворота изображения вокруг оси. */
gboolean gp_cifro_area_get_rotation (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  return priv->rotation;

}


/* Функция определяет: изменять масштаб при изменении размеров окна или изменять видимую область. */
void gp_cifro_area_set_scale_on_resize (GpCifroArea *carea, gboolean scale_on_resize)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  priv->scale_on_resize = scale_on_resize;

}


/* Функция возвращает состояние поведения масштабирования при изменении размеров окна. */
gboolean gp_cifro_area_get_scale_on_resize (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  return priv->scale_on_resize;

}


/* Функция задаёт режим масштабирования: по центру или по курсору. */
void gp_cifro_area_set_zoom_on_center (GpCifroArea *carea, gboolean zoom_on_center)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  priv->zoom_on_center = zoom_on_center;

}


/* Функция возвращает текущий режим масштабирования. */
gboolean gp_cifro_area_get_zoom_on_center (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  return priv->zoom_on_center;

}


/* Функция устанавливает параметры масштабирования. */
gboolean gp_cifro_area_set_zoom_scale (GpCifroArea *carea, gdouble zoom_scale)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  g_return_val_if_fail( zoom_scale > 0.0, FALSE );

  priv->zoom_scale = zoom_scale;

  return TRUE;

}


/* Функция возвращает параметры масштабирования. */
gdouble gp_cifro_area_get_zoom_scale (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  return priv->zoom_scale;

}


/* Функция задаёт набор фиксированных значений масштабов. */
gboolean gp_cifro_area_set_fixed_zoom_scales (GpCifroArea *carea, gdouble *zoom_x_scales, gdouble *zoom_y_scales,
                                              gint num_scales)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );
  gint i;

  if( num_scales == 0 )
    {
    g_free( priv->zoom_x_scales );
    g_free( priv->zoom_y_scales );
    priv->zoom_x_scales = NULL;
    priv->zoom_y_scales = NULL;
    priv->num_scales = 0;
    return TRUE;
    }

  for( i = 0; i < num_scales; i++ )
    {
    g_return_val_if_fail( zoom_x_scales[ i ] > 0.0, FALSE );
    g_return_val_if_fail( zoom_y_scales[ i ] > 0.0, FALSE );
    }

  g_free( priv->zoom_x_scales );
  g_free( priv->zoom_y_scales );

  priv->zoom_x_scales = g_malloc( sizeof( gdouble ) * num_scales );
  priv->zoom_y_scales = g_malloc( sizeof( gdouble ) * num_scales );

  memcpy( priv->zoom_x_scales, zoom_x_scales, sizeof( gdouble ) * num_scales );
  memcpy( priv->zoom_y_scales, zoom_y_scales, sizeof( gdouble ) * num_scales );
  priv->num_scales = num_scales;

  gp_cifro_area_set_shown (carea, priv->from_x, priv->to_x, priv->from_y, priv->to_y);

  return TRUE;

}


/* Функция возвращает текущий набор фиксированых значений масштабов. */
void gp_cifro_area_get_fixed_zoom_scales (GpCifroArea *carea, gdouble **zoom_x_scales, gdouble **zoom_y_scales,
                                          gint *num_scales)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( priv->num_scales )
    {

    if( zoom_x_scales )
      {
      *zoom_x_scales = g_malloc( sizeof( gdouble ) * priv->num_scales );
      memcpy( *zoom_x_scales, priv->zoom_x_scales, sizeof( gdouble ) * priv->num_scales );
      }

    if( zoom_y_scales )
      {
      *zoom_y_scales = g_malloc( sizeof( gdouble ) * priv->num_scales );
      memcpy( *zoom_y_scales, priv->zoom_y_scales, sizeof( gdouble ) * priv->num_scales );
      }

    }
  else
    {

    if( zoom_x_scales ) *zoom_x_scales = NULL;
    if( zoom_y_scales ) *zoom_y_scales = NULL;

    }

  if( num_scales ) *num_scales = priv->num_scales;

}


/* Функция задаёт соотношение масштабов по осям при изменении размеров. */
gboolean gp_cifro_area_set_scale_aspect (GpCifroArea *carea, gdouble scale_aspect)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  g_return_val_if_fail(
      gp_cifro_area_fix_scale_aspect (scale_aspect, &priv->min_scale_x, &priv->max_scale_x, &priv->min_scale_y,
                                      &priv->max_scale_y), FALSE );

  priv->scale_aspect = scale_aspect;

  gp_cifro_area_update_visible (priv);

  return TRUE;

}


/* Функция возвращает текущее соотношение масштабов по осям при изменении размеров. */
gdouble gp_cifro_area_get_scale_aspect (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  return priv->scale_aspect;

}


/* Функция задаёт значение умножителя скорости перемещения при нажатой клавише control. */
gboolean gp_cifro_area_set_move_multiplier (GpCifroArea *carea, gdouble move_multiplier)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  g_return_val_if_fail( move_multiplier > 0.0, FALSE );

  priv->move_multiplier = move_multiplier;

  return TRUE;

}


/* Функция возвращает текущее значение умножителя скорости перемещения при нажатой клавише control. */
gdouble gp_cifro_area_get_move_multiplier (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  return priv->move_multiplier;

}


/* Функция задаёт значение умножителя скорости вращения при нажатой клавише control. */
gboolean gp_cifro_area_set_rotate_multiplier (GpCifroArea *carea, gdouble rotate_multiplier)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  g_return_val_if_fail( rotate_multiplier > 0.0, FALSE );

  priv->rotate_multiplier = rotate_multiplier;

  return TRUE;

}


/* Функция возвращает текущее значение умножителя скорости вращения при нажатой клавише control. */
gdouble gp_cifro_area_get_rotate_multiplier (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  return priv->rotate_multiplier;

}


/* Функция задаёт угол поворота изображения в радианах. */
void gp_cifro_area_set_angle (GpCifroArea *carea, gdouble angle)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( priv->rotation )
    {
    angle = fmodf( angle, 2.0 * G_PI );
    if( angle > G_PI ) angle = angle - 2.0 * G_PI;
    if( angle < -G_PI ) angle = angle + 2.0 * G_PI;
    priv->angle = angle;
    priv->angle_cos = cos( priv->angle );
    priv->angle_sin = sin( priv->angle );
      gp_cifro_area_update_visible (priv);
    }

}


/* Функция возвращает текущий угол поворота изображения в радианах. */
gdouble gp_cifro_area_get_angle (GpCifroArea *carea)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  return priv->angle;

}


/* Функция задаёт границу текущей видимости. */
gboolean gp_cifro_area_set_shown (GpCifroArea *carea, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  g_return_val_if_fail( from_x < to_x, FALSE );
  g_return_val_if_fail( from_y < to_y, FALSE );

  if( from_x < priv->min_x ) from_x = priv->min_x;
  if( to_x > priv->max_x ) to_x = priv->max_x;

  if( from_y < priv->min_y ) from_y = priv->min_y;
  if( to_y > priv->max_y ) to_y = priv->max_y;

  priv->from_x = from_x;
  priv->to_x = to_x;
  priv->from_y = from_y;
  priv->to_y = to_y;

  if( priv->num_scales == 0 )
    {

    priv->cur_scale_x = 0.0;
    priv->cur_scale_y = 0.0;

    }
  else
    {

    gdouble opt_scale_x;
    gdouble opt_scale_y;
    gdouble scale_diff;
    gdouble min_scale_diff;
    gint i, opt_i;

    if( priv->widget_width && priv->widget_height )
      {
      opt_scale_x = ( priv->to_x - priv->from_x ) /
                    gp_cifro_area_get_visible_width (priv->widget_width, priv->widget_height, priv->angle);
      opt_scale_y = ( priv->to_y - priv->from_y ) /
                    gp_cifro_area_get_visible_height (priv->widget_width, priv->widget_height, priv->angle);
      }
    else
      {
      opt_scale_x = 1.0;
      opt_scale_y = 1.0;
      }

    min_scale_diff = fabs( priv->zoom_x_scales[ 0 ] - opt_scale_x ) + fabs( priv->zoom_y_scales[ 0 ] - opt_scale_y );
    opt_i = 0;

    for( i = 1; i < priv->num_scales; i++ )
      {
      scale_diff = fabs( priv->zoom_x_scales[ i ] - opt_scale_x ) + fabs( priv->zoom_y_scales[ i ] - opt_scale_y );
      if( scale_diff < min_scale_diff ) { min_scale_diff = scale_diff; opt_i = i; }
      }

    priv->cur_scale_x = priv->zoom_x_scales[ opt_i ];
    priv->cur_scale_y = priv->zoom_y_scales[ opt_i ];
    priv->cur_zoom_index = opt_i;

    }

  gp_cifro_area_update_visible (priv);
  priv->check_layers = TRUE;
  gtk_widget_queue_draw( GTK_WIDGET( carea ) );

  return TRUE;

}


/* Функция возвращает границу текущей видимости. */
void gp_cifro_area_get_shown (GpCifroArea *carea, gdouble *from_x, gdouble *to_x, gdouble *from_y, gdouble *to_y)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( from_x ) *from_x = priv->from_x;
  if( to_x ) *to_x = priv->to_x;
  if( from_y ) *from_y = priv->from_y;
  if( to_y ) *to_y = priv->to_y;

}


/* Функция задаёт пределы возможных отображаемых значений. */
gboolean gp_cifro_area_set_shown_limits (GpCifroArea *carea, gdouble min_x, gdouble max_x, gdouble min_y, gdouble max_y)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  GpCifroAreaLayer *layer;
  gint i;

  g_return_val_if_fail( min_x < max_x, FALSE );
  g_return_val_if_fail( min_y < max_y, FALSE );

  priv->min_x = min_x;
  priv->max_x = max_x;
  priv->min_y = min_y;
  priv->max_y = max_y;

  for( i = 0; i < priv->layers_num; i++ )
    {
    layer = g_list_nth_data( priv->layers, i );
    if( layer->renderer_id > 0 ) continue;
      gp_ica_renderer_set_shown_limits(layer->renderer, min_x, max_x, min_y, max_y);
    }

  gp_cifro_area_update_visible (priv);

  return TRUE;

}


/* Функция возвращает пределы возможных отображаемых значений. */
void gp_cifro_area_get_shown_limits (GpCifroArea *carea, gdouble *min_x, gdouble *max_x, gdouble *min_y, gdouble *max_y)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( min_x ) *min_x = priv->min_x;
  if( max_x ) *max_x = priv->max_x;
  if( min_y ) *min_y = priv->min_y;
  if( max_y ) *max_y = priv->max_y;

}


/* Функция задаёт пределы возможных коэффициентов масштабирования. */
gboolean gp_cifro_area_set_scale_limits (GpCifroArea *carea, gdouble min_scale_x, gdouble max_scale_x,
                                         gdouble min_scale_y, gdouble max_scale_y)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  g_return_val_if_fail(
      gp_cifro_area_fix_scale_aspect (priv->scale_aspect, &min_scale_x, &max_scale_x, &min_scale_y, &max_scale_y), FALSE );

  priv->min_scale_x = min_scale_x;
  priv->max_scale_x = max_scale_x;
  priv->min_scale_y = min_scale_y;
  priv->max_scale_y = max_scale_y;

  gp_cifro_area_update_visible (priv);

  return TRUE;

}


/* Функция возвращает пределы возможных коэффициентов масштабирования. */
void gp_cifro_area_get_scale_limits (GpCifroArea *carea, gdouble *min_scale_x, gdouble *max_scale_x,
                                     gdouble *min_scale_y, gdouble *max_scale_y)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  if( min_scale_x ) *min_scale_x = priv->min_scale_x;
  if( max_scale_x ) *max_scale_x = priv->max_scale_x;
  if( min_scale_y ) *min_scale_y = priv->min_scale_y;
  if( max_scale_y ) *max_scale_y = priv->max_scale_y;

}


/* Функция смещает видимую область на x_step, y_step. */
void gp_cifro_area_move (GpCifroArea *carea, gdouble step_x, gdouble step_y)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  gdouble shift_x, shift_y;

  shift_x = step_x * priv->angle_cos - step_y * priv->angle_sin;
  shift_y = step_y * priv->angle_cos + step_x * priv->angle_sin;

  shift_x *= priv->cur_scale_x;
  shift_y *= priv->cur_scale_y;

  if( priv->swap_x ) shift_x = -shift_x;
  if( priv->swap_y ) shift_y = -shift_y;

  if( shift_x < 0 )
    {
    if( priv->from_x + shift_x < priv->min_x ) shift_x = priv->min_x - priv->from_x;
    }
  else
    {
    if( priv->to_x + shift_x > priv->max_x ) shift_x = priv->max_x - priv->to_x;
    }

  if( shift_y < 0 )
    {
    if( priv->from_y + shift_y < priv->min_y ) shift_y = priv->min_y - priv->from_y;
    }
  else
    {
    if( priv->to_y + shift_y > priv->max_y ) shift_y = priv->max_y - priv->to_y;
    }

  priv->from_x += shift_x;
  priv->to_x += shift_x;
  priv->from_y += shift_y;
  priv->to_y += shift_y;

  gp_cifro_area_update_visible (priv);
  priv->check_layers = TRUE;
  gtk_widget_queue_draw( GTK_WIDGET( carea ) );

}


/* Функция поворачивает видимую область на угол angle. */
void gp_cifro_area_rotate (GpCifroArea *carea, gdouble angle)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  angle += priv->angle;

  gp_cifro_area_set_angle (carea, angle);
  priv->check_layers = TRUE;
  gtk_widget_queue_draw( GTK_WIDGET( carea ) );

}


/* Функция масштабирования видимой области. */
void gp_cifro_area_zoom (GpCifroArea *carea, gboolean zoom_x, gboolean zoom_y, gdouble center_val_x,
                         gdouble center_val_y, gboolean zoom_in, gdouble zoom_scale)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  gdouble new_scale_x;
  gdouble new_scale_y;

  gdouble from_x, to_x;
  gdouble from_y, to_y;

  from_x = priv->from_x;
  to_x = priv->to_x;
  from_y = priv->from_y;
  to_y = priv->to_y;

  zoom_scale = 1.0 + ( priv->zoom_scale / 100.0 );
  if( !zoom_in ) zoom_scale = 1.0 / zoom_scale;

  if( priv->scale_aspect > 0.0 ) { zoom_x = TRUE; zoom_y = TRUE; }

  new_scale_x = priv->cur_scale_x / zoom_scale;
  new_scale_y = priv->cur_scale_y / zoom_scale;

  if( new_scale_x < priv->min_scale_x || new_scale_x > priv->max_scale_x ) zoom_x = FALSE;
  if( new_scale_y < priv->min_scale_y || new_scale_y > priv->max_scale_y ) zoom_y = FALSE;

  if( priv->scale_aspect > 0.0 && ( zoom_x == FALSE || zoom_y == FALSE ) ) return;
  if( !zoom_x && !zoom_y ) return;

  if( zoom_x )
    {
    from_x = center_val_x - ( center_val_x - from_x ) / zoom_scale;
    to_x = center_val_x + ( to_x - center_val_x ) / zoom_scale;
    }

  if( zoom_y )
    {
    from_y = center_val_y - ( center_val_y - from_y ) / zoom_scale;
    to_y = center_val_y + ( to_y - center_val_y ) / zoom_scale;
    }

  if( from_x < priv->min_x ) from_x = priv->min_x;
  if( to_x > priv->max_x ) to_x = priv->max_x;
  if( from_y < priv->min_y ) from_y = priv->min_y;
  if( to_y > priv->max_y ) to_y = priv->max_y;

  gp_cifro_area_set_shown (carea, from_x, to_x, from_y, to_y);

}


/* Функция переключения фиксированного масштаба. */
void gp_cifro_area_fixed_zoom (GpCifroArea *carea, gdouble center_val_x, gdouble center_val_y, gboolean zoom_in)
{

  GpCifroAreaPriv *priv = GP_CIFRO_AREA_GET_PRIVATE( carea );

  gdouble length_x;
  gdouble length_y;
  gdouble from_x, to_x;
  gdouble from_y, to_y;

  if( priv->num_scales == 0 ) return;

  if( zoom_in && ( priv->cur_zoom_index == priv->num_scales - 1 ) ) return;
  if( !zoom_in && ( priv->cur_zoom_index == 0 ) ) return;

  if( zoom_in ) priv->cur_zoom_index++;
  else priv->cur_zoom_index--;

  priv->cur_scale_x = priv->zoom_x_scales[ priv->cur_zoom_index ];
  priv->cur_scale_y = priv->zoom_y_scales[ priv->cur_zoom_index ];

  length_x = priv->to_x - priv->from_x;
  length_y = priv->to_y - priv->from_y;

  from_x = center_val_x - ( priv->cur_scale_x * priv->visible_width * ( center_val_x - priv->from_x ) / length_x );
  to_x = center_val_x + ( priv->cur_scale_x * priv->visible_width * ( priv->to_x - center_val_x ) / length_x );
  from_y = center_val_y - ( priv->cur_scale_y * priv->visible_height * ( center_val_y - priv->from_y ) / length_y );
  to_y = center_val_y + ( priv->cur_scale_y * priv->visible_height * ( priv->to_y - center_val_y ) / length_y );

  if( from_x < priv->min_x ) from_x = priv->min_x;
  if( to_x > priv->max_x ) to_x = priv->max_x;
  if( from_y < priv->min_y ) from_y = priv->min_y;
  if( to_y > priv->max_y ) to_y = priv->max_y;

  gp_cifro_area_set_shown (carea, from_x, to_x, from_y, to_y);

}
