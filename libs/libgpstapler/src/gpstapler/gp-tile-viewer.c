/*
 * Libgpstapler is a graphical tiles manipulation library.
 *
 * Copyright 2013 Sergey Volkhin.
 *
 * This file is part of Libgpstapler.
 *
 * Libgpstapler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Libgpstapler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Libgpstapler. If not, see <http://www.gnu.org/licenses/>.
 *
*/

/*!
 * \file tile-viewer.c
 *
 * \author Sergey Volkhin
 * \date 29.08.2014
 *
 * Реализация виджета GpTileViewer, предназначенного для отображения одной плитки,
 * сгенерированной объектом типа GpTiler.
 *
*/


#include <glib/gi18n-lib.h>
#include "gp-tile-viewer.h"


// TT: Time Test -->
  #if 0
    /// Максимальное количество точек.
    #define TT_MAX_NUM 4

    /// Инициализация переменных.
    #define TT_INIT GTimeVal tttv[TT_MAX_NUM]; guint tti, tt_point_num = 0;

    /// Точка фиксации времени.
    #define TT_POINT g_assert((tt_point_num) < TT_MAX_NUM); g_get_current_time(&tttv[tt_point_num]); tt_point_num++;

    /// Печать временных интервалов между всеми точками.
    #define TT_PRINT printf("TT GpTileViewer:\n"); for(tti = 0; tti < (tt_point_num - 1); tti++) printf( "\t[%d]: %.3lf\n", tti, ( ( tttv[tti + 1].tv_usec - tttv[tti].tv_usec ) + ( 1000000 * ( tttv[tti + 1].tv_sec - tttv[tti].tv_sec) ) ) / 1000000.0 );
  #else
    #define TT_MAX_NUM
    #define TT_INIT
    #define TT_POINT
    #define TT_PRINT
  #endif
// TT: Time Test <--


static const guint GP_TILE_VIEWER_GTK_BOX_PADDING = 5;
static const GtkIconSize GP_TILE_VIEWER_TOOLBAR_ICON_SIZE = GTK_ICON_SIZE_SMALL_TOOLBAR;

/// Список кнопок.
typedef enum
{
  BUTONO_REFRESH,
  BUTONO_GO_LEFT,
  BUTONO_GO_DOWN,
  BUTONO_GO_UP,
  BUTONO_GO_RIGHT,
  BUTONO_HOME,
  BUTONO_ZOOM_IN,
  BUTONO_ZOOM_OUT,

  BUTONO_NUM
}
Butono;


/*! Приватные данные объекта GpTileViewer.*/
struct _GpTileViewerPriv
{
  GtkWidget *tile_darea;
  GtkToolItem *refresh_button;

  /*!< Тулбар с кнопками. Ссылкой на тулбар владеет GpTileViewer.*/
  GtkWidget *reffed_toolbar;

  guchar *buf_for_tile;
  size_t buf_size;
  cairo_surface_t *tile_surface;

  GpTileStatus tile_status;
  GpTile tile_to_show;

  gboolean have_home_tile; /*!< Флаг, что хотя бы одна плитка была запрошена (и, соответственно, есть "домашняя" плитка).*/
  GpTile home_tile; /*!< Первая запрошенная плитка (считается "домашнeй").*/

  guint timeout_event_source_id;
  guint timeout_interval;

  ///\name Пара виджетов с кнопки перезагрузки, ссылками на которые владеет GpTileViewer.
  /// @{
    GtkWidget *reffed_refresh_image; /*!< Изображение с иконкой "view-refresh".*/
    GtkWidget *reffed_spinner; /*!< GtkSpinner.*/
  /// @}

  /* properties */
  GpTiler *tiler; /*!< Генератор плиток.*/
};


G_DEFINE_TYPE( GpTileViewer, gp_tile_viewer, GTK_TYPE_BOX );



enum
{
  PROP_O,

  PROP_TILER,

  N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };



/*! Сигналы GpTileViewer.*/
enum
{
  /*! Сигнал "refreshing-started", сигнализирующий о начале работы timeout'а с опросом GpTiler'а.*/
  REFRESHING_STARTED,

  /*! Сигнал "refreshing-done", сигнализирующий об окончании работы timeout'а с опросом GpTiler'а.*/
  REFRESHING_DONE,

  /*! Количество сигналов.*/
  SIGNALS_NUM
};

/*! Массив сигналов.*/
static guint gp_tile_viewer_signals[SIGNALS_NUM] = { 0 };


/// \name Статические функции.
/// @{
  /// Обработчик события draw для gp_tile_darea.
  ///
  /// \param widget - виджет GtkDrawingArea, который необходимо перерисовать;
  /// \param cr - cairo context;
  /// \param user_data - указатель на объект GpTileViewer, содержащий данный \a widget.
  ///
  /// \return Всегда FALSE (разрешение на выполнение других обработчиков).
  static gboolean gp_tile_darea_draw_cb( GtkWidget *widget, cairo_t *cr, gpointer user_data );

  /// Дефолтный обработчик сигнала refreshing-started.
  ///
  /// Запускает timeout с опросом GpTiler'а на предмет готовности требуемой плитки,
  /// рисует GtkSpinner на кнопке "view-refresh" на тулбаре виджета.
  ///
  /// \param gp_tile_viewer - указатель на объект GpTileViewer.
  static void refreshing_started( GpTileViewer *tile_viewer);

  /// Дефолтный обработчик сигнала refreshing-done.
  ///
  /// Останавливает timeout с опросом GpTiler'а на предмет готовности требуемой плитки,
  /// убирает GtkSpinner и рисует соответствующую иконку
  /// на кнопке "view-refresh" на тулбаре виджета.
  ///
  /// \param gp_tile_viewer - указатель на объект GpTileViewer.
  static void refreshing_done( GpTileViewer *tile_viewer);

  /// Предварительное освобождение памяти, занятой объектом.
  ///
  /// \param object - указатель на объект GpTileViewer.
  static void gp_tile_viewer_dispose( GObject *object );

  /// Окончательное освобождение памяти, занятой объектом.
  ///
  /// \param object - указатель на объект GpTileViewer.
  static void gp_tile_viewer_finalize( GObject *object );

  /// Получает виджет GpTileViewer, к которому относится данный \a widget.
  ///
  /// \param widget - виджет, входящий в виджет GpTileViewer.
  ///
  /// return Объект GpTileViewer.
  static GpTileViewer *gp_tile_viewer_get_by_widget( GtkWidget *widget );

  /// Функция получения параметров.
  ///
  /// \param object - указатель на объект GpTileViewer;
  /// \param prop_id - идентификатор параметра;
  /// \param value - значение параметра;
  /// \param pspec - описание параметра.
  static void gp_tile_viewer_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec );

  /// Функция установки параметров.
  ///
  /// \param object - указатель на объект GpTileViewer;
  /// \param prop_id - идентификатор параметра;
  /// \param value - новое значение параметра;
  /// \param pspec - описание параметра.
  static void gp_tile_viewer_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec );

  /// Реализация функции показа плитки \a tile на виджете \a gp_tile_viewer.
  ///
  /// Может быть запущена как из gp_tile_viewer_show_tile(),
  /// так и по таймауту (добавленном с помощью g_timeout_add).
  ///
  /// \note Функция имеет тип GSourceFunc.
  ///
  /// \param tile_viewer_ptr - указатель на объект для отображения плитки.
  ///
  /// \return TRUE -- в случае успеха, FALSE -- в случае ошибки.
  static gboolean gp_tile_viewer_show_tile_imp( gpointer tile_viewer_ptr );

  /// Обработчик события clicked по кнопке на toolbar.
  ///
  /// \param toolbutton - нажатая кнопка;
  /// \param butono - тип кнопки.
  static void toolbutton_clicked_cb(GtkToolButton *toolbutton, gpointer butono);
/// @}



gboolean gp_tile_darea_draw_cb( GtkWidget *widget, cairo_t *cr, gpointer user_data )
{
  g_return_val_if_fail(GTK_IS_DRAWING_AREA(widget), FALSE);

  GpTileViewer *tile_viewer = GP_TILE_VIEWER(user_data);
  g_return_val_if_fail(tile_viewer, FALSE);
  GpTileViewerPriv *priv = tile_viewer->priv;

  if(G_LIKELY(priv->tile_surface && cairo_surface_status(priv->tile_surface) == CAIRO_STATUS_SUCCESS))
  {
    cairo_set_source_rgb(cr, 0.7, 0.8, 1.0);
    cairo_paint(cr);

    cairo_set_source_surface(cr, priv->tile_surface, 0, 0);
    cairo_paint(cr);
  }

  return FALSE;
}



void refreshing_started( GpTileViewer *tile_viewer)
{
  GpTileViewerPriv *priv = tile_viewer->priv;

  if(priv->timeout_event_source_id == 0)
  {
    priv->timeout_event_source_id = g_timeout_add(
      priv->timeout_interval, gp_tile_viewer_show_tile_imp, tile_viewer);

    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(priv->refresh_button), priv->reffed_spinner);
    gtk_spinner_start(GTK_SPINNER(priv->reffed_spinner));
  }
}



void refreshing_done( GpTileViewer *tile_viewer)
{
  GpTileViewerPriv *priv = tile_viewer->priv;

  if(priv->timeout_event_source_id != 0)
  {
    g_source_remove(priv->timeout_event_source_id);
    priv->timeout_event_source_id = 0;

    gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(priv->refresh_button), priv->reffed_refresh_image);
    gtk_spinner_stop(GTK_SPINNER(priv->reffed_spinner));
  }
}



static void gp_tile_viewer_class_init( GpTileViewerClass *klass )
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->dispose = gp_tile_viewer_dispose;
  gobject_class->finalize = gp_tile_viewer_finalize;
  gobject_class->get_property = gp_tile_viewer_get_property;
  gobject_class->set_property = gp_tile_viewer_set_property;

  klass->refreshing_started = refreshing_started;
  klass->refreshing_done = refreshing_done;

  g_type_class_add_private(klass, sizeof( GpTileViewerPriv ));

  obj_properties[PROP_TILER] =
    g_param_spec_object("tiler", _("GpTiler"), _("GpTiler object for tile's generation"),
                        GP_TYPE_TILER, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(gobject_class, N_PROPERTIES, obj_properties);

  gp_tile_viewer_signals[REFRESHING_STARTED] =
    g_signal_new(
      "refreshing-started",     //< Имя сигнала.
      G_TYPE_FROM_CLASS(klass), //< Тип класса, которому сигнал принадлежит.
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, //< Флаги: дефолтный обработчик выполнять в начале, сигнал -- action.
      G_STRUCT_OFFSET(GpTileViewerClass, refreshing_started), //< Относительный указатель на функцию сигнала в структуре класса.
      NULL, NULL, //< Аккумулятор и его параметр (нужны например для сбора возвращаемых значений от коллбеков).
      g_cclosure_marshal_VOID__VOID,  //< Конвертер массивов параметров для вызова коллбеков.
      G_TYPE_NONE, 0);  //< Возвращаемое значение и количество входных параметров.

  gp_tile_viewer_signals[REFRESHING_DONE] =
    g_signal_new(
      "refreshing-done",        //< Имя сигнала.
      G_TYPE_FROM_CLASS(klass), //< Тип класса, которому сигнал принадлежит.
      G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION, //< Флаги: дефолтный обработчик выполнять в начале, сигнал -- action.
      G_STRUCT_OFFSET(GpTileViewerClass, refreshing_done), //< Относительный указатель на функцию сигнала в структуре класса.
      NULL, NULL, //< Аккумулятор и его параметр (нужны например для сбора возвращаемых значений от коллбеков).
      g_cclosure_marshal_VOID__VOID,  //< Конвертер массивов параметров для вызова коллбеков.
      G_TYPE_NONE, 0);  //< Возвращаемое значение и количество входных параметров.
}



static void gp_tile_viewer_dispose( GObject *object )
{
  GpTileViewer *tile_viewer = GP_TILE_VIEWER(object);
  GpTileViewerPriv *priv = tile_viewer->priv;

  if(priv->timeout_event_source_id)
  {
    g_source_remove(priv->timeout_event_source_id);
    priv->timeout_event_source_id = 0;
  }

  g_clear_object(&priv->reffed_toolbar);

  g_clear_object(&priv->reffed_refresh_image);
  g_clear_object(&priv->reffed_spinner);

  g_clear_object(&priv->tiler);

  G_OBJECT_CLASS(gp_tile_viewer_parent_class)->dispose(object);
}



void gp_tile_viewer_finalize( GObject *object )
{
  GpTileViewer *tile_viewer = GP_TILE_VIEWER(object);
  GpTileViewerPriv *priv = tile_viewer->priv;

  g_free(priv->buf_for_tile);
  priv->buf_for_tile = NULL;

  if(priv->tile_surface) cairo_surface_destroy(priv->tile_surface);

  G_OBJECT_CLASS(gp_tile_viewer_parent_class)->finalize(object);
}



GpTileViewer *gp_tile_viewer_get_by_widget( GtkWidget *widget )
{
  g_return_val_if_fail(GTK_IS_WIDGET(widget), NULL);

  do
  {
    if(G_UNLIKELY( GP_TILE_VIEWER_IS(widget)))
      return (GpTileViewer *)widget;
  }
  while(( widget = gtk_widget_get_parent(widget) ));

  g_critical("Passed to %s widget is not GpTileViewer child.", G_STRFUNC);
  return NULL;
}




void gp_tile_viewer_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec )
{
  GpTileViewer *tile_viewer = GP_TILE_VIEWER(object);
  GpTileViewerPriv *priv = tile_viewer->priv;

  switch (prop_id)
  {
    case PROP_TILER:
      g_value_set_object(value, priv->tiler);
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(tile_viewer, prop_id, pspec);
    break;
  }
}



static void gp_tile_viewer_init( GpTileViewer *tile_viewer )
{
  GtkToolItem *item;
  GpTileViewerPriv *priv = tile_viewer->priv = G_TYPE_INSTANCE_GET_PRIVATE(tile_viewer, G_TYPE_GP_TILE_VIEWER, GpTileViewerPriv);

  gtk_orientable_set_orientation(GTK_ORIENTABLE(tile_viewer), GTK_ORIENTATION_VERTICAL);

  priv->tile_darea = gtk_drawing_area_new();
  g_signal_connect(priv->tile_darea, "draw", G_CALLBACK( gp_tile_darea_draw_cb ), tile_viewer);

  gtk_box_pack_start(GTK_BOX(tile_viewer), priv->tile_darea, TRUE, TRUE, GP_TILE_VIEWER_GTK_BOX_PADDING );

  // Пара виджетов с кнопки перезагрузки, ссылками на которые владеет GpTileViewer -->
    priv->reffed_refresh_image = gtk_image_new_from_icon_name("view-refresh", GP_TILE_VIEWER_TOOLBAR_ICON_SIZE );
    priv->reffed_spinner = gtk_spinner_new();
    gtk_widget_show(priv->reffed_refresh_image);
    gtk_widget_show(priv->reffed_spinner);
    g_object_ref_sink(priv->reffed_refresh_image);
    g_object_ref_sink(priv->reffed_spinner);
  // Пара виджетов с кнопки перезагрузки, ссылками на которые владеет GpTileViewer <--

  priv->reffed_toolbar = gtk_toolbar_new();
  g_object_ref_sink(priv->reffed_toolbar);
  gtk_toolbar_set_style(GTK_TOOLBAR(priv->reffed_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_icon_size(GTK_TOOLBAR(priv->reffed_toolbar), GP_TILE_VIEWER_TOOLBAR_ICON_SIZE );
  gp_tile_viewer_set_toolbar_visible( tile_viewer, TRUE);

  item = priv->refresh_button = gtk_tool_button_new(priv->reffed_refresh_image, NULL);
  g_signal_connect(item, "clicked", G_CALLBACK(toolbutton_clicked_cb), GINT_TO_POINTER(BUTONO_REFRESH));
  gtk_toolbar_insert(GTK_TOOLBAR(priv->reffed_toolbar), item, -1);

  item = gtk_tool_button_new(
    gtk_image_new_from_icon_name("go-previous", GP_TILE_VIEWER_TOOLBAR_ICON_SIZE ), NULL);
  g_signal_connect(item, "clicked", G_CALLBACK(toolbutton_clicked_cb), GINT_TO_POINTER(BUTONO_GO_LEFT));
  gtk_toolbar_insert(GTK_TOOLBAR(priv->reffed_toolbar), item, -1);

  item = gtk_tool_button_new(
    gtk_image_new_from_icon_name("go-down", GP_TILE_VIEWER_TOOLBAR_ICON_SIZE ), NULL);
  g_signal_connect(item, "clicked", G_CALLBACK(toolbutton_clicked_cb), GINT_TO_POINTER(BUTONO_GO_DOWN));
  gtk_toolbar_insert(GTK_TOOLBAR(priv->reffed_toolbar), item, -1);

  item = gtk_tool_button_new(
    gtk_image_new_from_icon_name("go-up", GP_TILE_VIEWER_TOOLBAR_ICON_SIZE ), NULL);
  g_signal_connect(item, "clicked", G_CALLBACK(toolbutton_clicked_cb), GINT_TO_POINTER(BUTONO_GO_UP));
  gtk_toolbar_insert(GTK_TOOLBAR(priv->reffed_toolbar), item, -1);

  item = gtk_tool_button_new(
    gtk_image_new_from_icon_name("go-next", GP_TILE_VIEWER_TOOLBAR_ICON_SIZE ), NULL);
  g_signal_connect(item, "clicked", G_CALLBACK(toolbutton_clicked_cb), GINT_TO_POINTER(BUTONO_GO_RIGHT));
  gtk_toolbar_insert(GTK_TOOLBAR(priv->reffed_toolbar), item, -1);

  item = gtk_tool_button_new(
    gtk_image_new_from_icon_name("go-home", GP_TILE_VIEWER_TOOLBAR_ICON_SIZE ), NULL);
  g_signal_connect(item, "clicked", G_CALLBACK(toolbutton_clicked_cb), GINT_TO_POINTER(BUTONO_HOME));
  gtk_toolbar_insert(GTK_TOOLBAR(priv->reffed_toolbar), item, -1);

  item = gtk_separator_tool_item_new();
  gtk_tool_item_set_expand(item, TRUE);
  gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(item), FALSE);
  gtk_toolbar_insert(GTK_TOOLBAR(priv->reffed_toolbar), item, -1);

  item = gtk_tool_button_new(
    gtk_image_new_from_icon_name("zoom-in", GP_TILE_VIEWER_TOOLBAR_ICON_SIZE ), NULL);
  g_signal_connect(item, "clicked", G_CALLBACK(toolbutton_clicked_cb), GINT_TO_POINTER(BUTONO_ZOOM_IN));
  gtk_toolbar_insert(GTK_TOOLBAR(priv->reffed_toolbar), item, -1);

  item = gtk_tool_button_new(
    gtk_image_new_from_icon_name("zoom-out", GP_TILE_VIEWER_TOOLBAR_ICON_SIZE ), NULL);
  g_signal_connect(item, "clicked", G_CALLBACK(toolbutton_clicked_cb), GINT_TO_POINTER(BUTONO_ZOOM_OUT));
  gtk_toolbar_insert(GTK_TOOLBAR(priv->reffed_toolbar), item, -1);


  priv->timeout_interval = 100;
}



GpTileViewer *gp_tile_viewer_new( GpTiler *tiler )
{
  g_return_val_if_fail( GP_IS_TILER(tiler), NULL);

  return g_object_new(G_TYPE_GP_TILE_VIEWER,
    "tiler", tiler,
    NULL);
}



void gp_tile_viewer_refresh( GpTileViewer *tile_viewer )
{
  g_return_if_fail( GP_TILE_VIEWER_IS(tile_viewer));

  gp_tile_viewer_show_tile( tile_viewer, tile_viewer->priv->tile_to_show );
}



gboolean gp_tile_viewer_show_tile_imp( gpointer tile_viewer_ptr )
{
  GpTileViewer *tile_viewer = GP_TILE_VIEWER(tile_viewer_ptr);
  g_return_val_if_fail(tile_viewer, FALSE);
  GpTileViewerPriv *priv = tile_viewer->priv;

TT_INIT
TT_POINT

  GpTileStatus new_status = gp_tiler_get_tile(priv->tiler,
    priv->buf_for_tile, &priv->tile_to_show, priv->tile_status);

TT_POINT
TT_PRINT

  // Если плитка "актуальней" чем та, что уже отрисована, сгенерируем draw-event.
  if(new_status > priv->tile_status)
  {
    cairo_surface_mark_dirty(priv->tile_surface);
    if(gtk_widget_get_mapped(priv->tile_darea))
      gdk_window_invalidate_rect(gtk_widget_get_window(priv->tile_darea), NULL, FALSE);
  }

  // Плитка готова, останавливаем timeout.
  if(new_status == GP_TILE_STATUS_ACTUAL)
    g_signal_emit(tile_viewer, gp_tile_viewer_signals[REFRESHING_DONE], 0);

  priv->tile_status = new_status;
  return TRUE;
}



void gp_tile_viewer_show_tile( GpTileViewer *tile_viewer, GpTile tile )
{
  g_return_if_fail( GP_TILE_VIEWER_IS(tile_viewer));
  GpTileViewerPriv *priv = tile_viewer->priv;

  // Т.к. мы отображаем одну плитку, то нам важно отобразить
  // как можно быстрее и именно текущую, нечего генерить которые
  // запрашивались ранее и лежат в очереди.
  // Поэтому чистим очередь каждый раз.
  gp_tiler_drop_tasks(priv->tiler);

  if(G_UNLIKELY(priv->have_home_tile == FALSE))
  {
    priv->home_tile = tile;
    priv->have_home_tile = TRUE;
  }

  // Запросим новую плитку из GpTiler'а.
  priv->tile_status = GP_TILE_STATUS_NOT_INIT;
  priv->tile_to_show = tile;
  gp_tile_viewer_show_tile_imp( tile_viewer );

  // Плитка не готова, запускаем timeout с опросом GpTiler'а.
  if(priv->tile_status != GP_TILE_STATUS_ACTUAL)
    g_signal_emit(tile_viewer, gp_tile_viewer_signals[REFRESHING_STARTED], 0);
}



void gp_tile_viewer_set_home_tile( GpTileViewer *tile_viewer, GpTile tile )
{
  g_return_if_fail( GP_TILE_VIEWER_IS(tile_viewer));
  tile_viewer->priv->have_home_tile = TRUE;
  tile_viewer->priv->home_tile = tile;
}



void gp_tile_viewer_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec )
{
  GpTileViewer *tile_viewer = GP_TILE_VIEWER(object);
  GpTileViewerPriv *priv = tile_viewer->priv;

  switch(prop_id)
  {
    case PROP_TILER:
      priv->tiler = g_value_get_object(value);

      if( priv->tiler != NULL )
      {
        g_object_ref_sink(priv->tiler);

        gtk_widget_set_size_request(priv->tile_darea, GP_TILE_SIDE, GP_TILE_SIDE);

        gint stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, GP_TILE_SIDE);

        g_free(priv->buf_for_tile);
        priv->buf_size = GP_TILE_SIDE * stride;
        priv->buf_for_tile = g_malloc0(priv->buf_size);

        if(priv->tile_surface) cairo_surface_destroy(priv->tile_surface);
        priv->tile_surface = cairo_image_surface_create_for_data(priv->buf_for_tile,
          CAIRO_FORMAT_ARGB32, GP_TILE_SIDE, GP_TILE_SIDE, stride);
      }
      else
      {
        g_critical("GpTiler == NULL.");
        g_object_unref(tile_viewer);
      }
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(tile_viewer, prop_id, pspec);
    break;
  }
}



void gp_tile_viewer_set_toolbar_visible( GpTileViewer *tile_viewer, gboolean toolbar_visible )
{
  g_return_if_fail( GP_TILE_VIEWER_IS(tile_viewer));
  GpTileViewerPriv *priv = tile_viewer->priv;

  if(toolbar_visible)
  {
    if(!gtk_widget_get_parent(priv->reffed_toolbar))
    {
      gtk_widget_set_visible(priv->reffed_toolbar, TRUE);
      gtk_box_pack_start(GTK_BOX(tile_viewer),
        priv->reffed_toolbar, FALSE, FALSE, 0);
    }
  }
  else
  {
    if(gtk_widget_get_parent(priv->reffed_toolbar))
      gtk_container_remove(GTK_CONTAINER(tile_viewer), priv->reffed_toolbar);
  }
}



void toolbutton_clicked_cb(GtkToolButton *toolbutton, gpointer butono)
{
  GpTileViewer *tile_viewer = gp_tile_viewer_get_by_widget(GTK_WIDGET( toolbutton ));
  g_return_if_fail(tile_viewer);
  GpTileViewerPriv *priv = tile_viewer->priv;

  if(priv->tile_to_show.l == 0)
  {
    g_debug("GpTile.l == 0, looks like user didn't set a tile to show.");
    return;
  }

  switch(GPOINTER_TO_INT(butono))
  {
    case BUTONO_REFRESH:
      ;
    break;

    case BUTONO_GO_LEFT:
      priv->tile_to_show.x--;
    break;

    case BUTONO_GO_DOWN:
      priv->tile_to_show.y--;
    break;

    case BUTONO_GO_UP:
      priv->tile_to_show.y++;
    break;

    case BUTONO_GO_RIGHT:
      priv->tile_to_show.x++;
    break;

    case BUTONO_HOME:
      if(priv->have_home_tile)
        priv->tile_to_show = priv->home_tile;
    break;

    case BUTONO_ZOOM_IN:
      if(priv->tile_to_show.l == 1)
        return;

      priv->tile_to_show.l /= 2;
      priv->tile_to_show.x *= 2;
      priv->tile_to_show.y *= 2;
    break;

    case BUTONO_ZOOM_OUT:
      priv->tile_to_show.l *= 2;
      priv->tile_to_show.x /= 2;
      priv->tile_to_show.y /= 2;
    break;
  }

  gp_tile_viewer_refresh( tile_viewer );
}

