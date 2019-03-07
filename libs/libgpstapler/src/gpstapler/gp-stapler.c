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
 * \file stapler.c
 *
 * \author Sergey Volkhin
 * \date 19.09.2013
 *
 * Реализация объекта GpStapler, предназначенного для формирования изображений из плиток,
 * сгенерированных объектами типа Tiler.
 *
*/


#include "gp-core-gui.h"
#include "gp-stapler.h"
#include "layer.h"

#include <gtk/gtk.h> //< Для GtkTreeModel.
#include <string.h> //< Для memset.
#include <stdio.h>
#include <stdlib.h> //< Для atoi.
#include <pixman.h>
#include <glib/gi18n-lib.h>



// TT: Time Test -->
  #if 0
    /// Максимальное количество точек.
    #define TT_MAX_NUM 4

    /// Инициализация переменных.
    #define TT_INIT GTimeVal tttv[TT_MAX_NUM]; guint tti, tt_point_num = 0;

    /// Точка фиксации времени.
    #define TT_POINT g_assert((tt_point_num) < TT_MAX_NUM); g_get_current_time(&tttv[tt_point_num]); tt_point_num++;

    /// Печать временных интервалов между всеми точками.
    #define TT_PRINT printf("TT GpStapler:\n"); for(tti = 0; tti < (tt_point_num - 1); tti++) printf( "\t[%d]: %.3lf\n", tti, ( ( tttv[tti + 1].tv_usec - tttv[tti].tv_usec ) + ( 1000000 * ( tttv[tti + 1].tv_sec - tttv[tti].tv_sec) ) ) / 1000000.0 );
  #else
    #define TT_MAX_NUM
    #define TT_INIT
    #define TT_POINT
    #define TT_PRINT
  #endif
// TT: Time Test <--



/// Приватные данные объекта GpStapler.
struct _GpStaplerPriv
{
  const GpIcaState *state; /*!< Состояние областей отрисовки.*/

  GPtrArray *layers; /*!< Слои с tiler'ами (один слой -- один GpTiler). Индекс в массиве слоев == user_data2 в GtkTreeIter.*/
  GPtrArray *tags; /*!< Тэги (один тэг -- несколько слоев). Индекс в массиве тэгов == user_data3 в GtkTreeIter.*/

  pixman_image_t *icarenderer_data_pimage; /*!< Pixman image для REN_DATA.*/

  pixman_image_t *tmp_data_pimage; /*!< Временный image для данных: возможно перед отрисовкой данных с ними нужно что-то сделать. Сделать можно именно в этом временном pimage.*/

  gboolean data_force_update; /*!< Флаг того, что REN_DATA необходимо обязательно перерисовать (например изменился масштаб, добавился tiler или сдвинулись оси).*/
  gboolean force_restack_layers; /*!< Флаг того, что REN_DATA необходимо заново сложить слои (без обновления самих слоев), например если один из слоев был удален.*/

  pixman_image_t *background_pimage; /*!< Pixman image для фона (под всеми слоями).*/
  pixman_image_t *highlight_pimage; /*!< Pixman image для подсвечивания (выделения) слоя.*/

  /* properties */
  GpIcaStateRenderer *state_renderer; /*!< Псевдоотрисовщик для учета состояния областей.*/
};



/// Типы итераторов GtkTreeModel, они же типы строк,
/// которые GpStapler предоставляет через интерфейс GtkTreeModel.
///
/// Тип итератора хранится в поле user_data объекта GtkTreeIter.
/// В поле user_data2 хранится индекс в массиве _GpStaplerPriv::layers.
/// В поле user_data3 хранится индекс в массиве _GpStaplerPriv::tags.
typedef enum
{
  ITER_NOT_INIT,/*!< Неинициализированный итератор.*/
  ITER_INVALID, /*!< Невалидный итератор.*/
  ITER_LAYER,   /*!< Итератор соответствует слою с GpTiler'ом.*/
  ITER_TAG,     /*!< Итератор соответствует тэгу со слоями.*/
}
IterTypes;


enum
{
  PROP_O,

  PROP_STATE_RENDERER,

  N_PROPERTIES
};


enum
{
  REN_DATA,

  N_RENDERERS
};


// Сигналы.
enum
{
  // Сигнал "restacked", сигнализирующий о том, что GpStapler заново наложил слои друг на друга.
  RESTACKED,

  // Количество сигналов.
  SIGNALS_NUM
};

// Массив сигналов.
static guint gp_stapler_signals[SIGNALS_NUM] = { 0 };



/// Типы колонок, которые GpStapler предоставляет через интерфейс GtkTreeModel.
static GType GP_STAPLER_COL_TYPES[] =
{
  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_TILER.
  G_TYPE_STRING,  //< GP_TILER_TREE_MODEL_COLS_NAME.
  G_TYPE_BOOLEAN, //< GP_TILER_TREE_MODEL_COLS_VISIBLE.
  G_TYPE_BOOLEAN, //< GP_TILER_TREE_MODEL_COLS_HIGHLIGHT.
  G_TYPE_UINT,    //< GP_TILER_TREE_MODEL_COLS_UPD_TIMEOUT.
  G_TYPE_DOUBLE,  //< GP_TILER_TREE_MODEL_COLS_DELTA_X.
  G_TYPE_DOUBLE,  //< GP_TILER_TREE_MODEL_COLS_DELTA_Y.

  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_USER_OBJ_1.
  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_USER_OBJ_2.
  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_USER_OBJ_3.
  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_USER_OBJ_4.
  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_USER_OBJ_5.
  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_USER_OBJ_6.
  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_USER_OBJ_7.
  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_USER_OBJ_8.
  G_TYPE_OBJECT,  //< GP_TILER_TREE_MODEL_COLS_USER_OBJ_9.
};
G_STATIC_ASSERT(G_N_ELEMENTS( GP_STAPLER_COL_TYPES ) == GP_TILER_TREE_MODEL_COLS_NUM );



/// \name Интерфейс GpIcaRenderer.
/// @{
  /// Прогресс формирования изображения.
  ///
  /// Возвращает численое значение прогреса формирования изображения 0 - 1000,
  /// либо -1, если колчество слоев равно 0.
  ///
  /// \param renderer - указатель на интерфейс GpIcaRenderer;
  /// \param renderer_id - идентификатор формирователя изображения.
  ///
  /// \return Число от 0 до 1000, либо -1, если количество слоев равно 0.
  static gint gp_stapler_get_completion( GpIcaRenderer *renderer, gint renderer_id );

  static const gchar *gp_stapler_get_name( GpIcaRenderer *stapler, gint renderer_id );

  static GpIcaRendererType gp_stapler_get_renderer_type( GpIcaRenderer *stapler, gint renderer_id );

  static gint gp_stapler_get_renderers_num( GpIcaRenderer *stapler );

  /// Функция записывает в буфер памяти сформированное изображение, одновременно
  /// возвращая начало и границу области затронутой отрисовкой.
  ///
  /// \param stapler - указатель на объект GpStapler;
  /// \param renderer_id - идентификатор формирователя изображения;
  /// \param x - координата по оси x начала области изменения;
  /// \param y - координата по оси y начала области изменения;
  /// \param width - ширина области изменения;
  /// \param height - высота области изменения.
  static GpIcaRendererAvailability gp_stapler_render( GpIcaRenderer *stapler, gint renderer_id, gint *x, gint *y,
                                                      gint *width, gint *height );

  static void gp_stapler_set_visible_size( GpIcaRenderer *stapler, gint width, gint height );

  static void gp_stapler_set_shown( GpIcaRenderer *stapler, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y );

  static void gp_stapler_set_surface( GpIcaRenderer *stapler, gint renderer_id, guchar *data, gint width, gint height,
                                      gint stride );
/// @}


/// \name Интерфейс GtkTreeDragDest.
/// @{
  /// Гововит \a drag_source вставить строку до \a dest с содержимым из \a selection_data.
  ///
  /// \param drag_dest - объект GtkTreeDragDest;
  /// \param dest_path - указатель на строку, перед которой следует вставить сроку \a selection_data;
  /// \param selection_data - данные, представляющие вставляемую строку.
  ///
  /// \return TRUE, если строка удачно вставлена, иначе -- FALSE.
  static gboolean gp_stapler_drag_data_received( GtkTreeDragDest *drag_dest, GtkTreePath *dest_path,
                                                 GtkSelectionData *selection_data );

  /// Позволяет узнать можно ли вставить строку перед \a drag_dest на том же уровне.
  ///
  /// \param drag_dest - объект GtkTreeDragDest;
  /// \param dest_path - указатель на строку, перед которой следует вставить строку \a selection_data;
  /// \param selection_data - данные, представляющие вставляемую строку.
  ///
  /// \return TRUE, если можно вставить строку перед \a drag_dest, иначе -- FALSE.
  static gboolean gp_stapler_row_drop_possible( GtkTreeDragDest *drag_dest, GtkTreePath *dest_path,
                                                GtkSelectionData *selection_data );
/// @}


/// \name Интерфейс GtkTreeDragSource.
/// @{
  /// Позволяет узнать можно ли использовать указанную строку в качестве источника DND.
  ///
  /// \param drag_source - объект GtkTreeDragSource;
  /// \param path - указатель на строку.
  ///
  /// \return TRUE, если можно использовать в качестве источника DND, иначе -- FALSE.
  static gboolean gp_stapler_row_draggable( GtkTreeDragSource *drag_source, GtkTreePath *path );

  /// Гововит \a drag_source заполнить \a selection_data данными, представляющими строку \a path.
  ///
  /// \param drag_source - объект GtkTreeDragSource;
  /// \param path - указатель на строку;
  /// \param selection_data - данные, представляющие строку.
  ///
  /// \return TRUE, если данные были предоставлены, иначе -- FALSE.
  static gboolean gp_stapler_drag_data_get( GtkTreeDragSource *drag_source, GtkTreePath *path,
                                            GtkSelectionData *selection_data );

  /// Гововит \a drag_source удалить строку, т.к. она была перемещена на новое место с помощью DND.
  ///
  /// \param drag_source - объект GtkTreeDragSource;
  /// \param path - указатель на строку.
  ///
  /// \return TRUE, если строка удачно удалена, иначе -- FALSE.
  static gboolean gp_stapler_drag_data_delete( GtkTreeDragSource *drag_source, GtkTreePath *path );
/// @}


/// \name Интерфейс GtkTreeModel.
/// @{
  /// Возвращает флаги, описывающие особенности данной реализации GtkTreeModel.
  ///
  /// \param tree_model - указатель на объект GpStapler.
  ///
  /// \return Флаги, описывающие особенности данной реализации GtkTreeModel.
  static GtkTreeModelFlags gp_stapler_get_flags( GtkTreeModel *tree_model );

  /// Возвращает количество колонок, которые GpStapler предоставляет через интерфейс GtkTreeModel.
  ///
  /// \param tree_model - указатель на объект GpStapler.
  ///
  /// \return Значение GP_TILER_TREE_MODEL_COLS_NUM.
  static gint gp_stapler_get_n_columns( GtkTreeModel *tree_model );

  /// Возвращает тип данных для колонки.
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param index - номер колонки.
  ///
  /// \return Тип данных в колонке.
  static GType gp_stapler_get_column_type( GtkTreeModel *tree_model, gint index );

  /// Устанавливает итератор \a iter в итератор, указывающий на \a path.
  ///
  /// Если \a path не существует, итератор устанавливается в невалидный итератор (#ITER_INVALID)
  /// и возвращается FALSE.
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param iter - возвращаемый итератор;
  /// \param path - элемент, итератор указывающий на который необходимо получить.
  ///
  /// \return TRUE, если \a iter установлен в валидный итератор, иначе -- FALSE.
  static gboolean gp_stapler_get_iter( GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path );

  /// Возвращает объект GtkTreePath, на который указывает \a iter.
  ///
  /// Возвращаемый объект GtkTreePath должен быть удален с помощью gtk_tree_path_free().
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param iter - требуемый элемент.
  ///
  /// \return Только что созданный объект GtkTreePath.
  static GtkTreePath *gp_stapler_get_path( GtkTreeModel *tree_model, GtkTreeIter *iter );

  /// Позволяет получить значение в колонке \a column в строке \a iter.
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param iter - итератор, указывающий на строку;
  /// \param column - номер колонки;
  /// \param value - возвращаемое значение.
  static void gp_stapler_get_value( GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, GValue *value );

  /// Позволяет получить GtkTreeIter для следующего элемента данного уровня.
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param iter - указатель на итератор, указывающий на текущий элемент,
  /// туда же будет записан итератор, указывающий на следующий элемент.
  ///
  /// \return TRUE, если итератор успешно записан, FALSE, если следующего элемента не существует.
  static gboolean gp_stapler_iter_next( GtkTreeModel *tree_model, GtkTreeIter *iter );

  /// Позволяет получить первый потомок элемента \a parent.
  ///
  /// Возвратит первый корневой элемент, если \a parent == NULL.
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param iter - возвращаемый элемент;
  /// \param parent - элемент-родитель, либо NULL.
  ///
  /// \return TRUE, если удалось получить элемент, иначе -- FALSE.
  static gboolean gp_stapler_iter_children( GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent );

  /// Позволяет узнать есть ли у элемента \a iter потомки.
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param iter - требуемый элемент;
  ///
  /// \return TRUE, если у элемента есть потомки, иначе -- FALSE.
  static gboolean gp_stapler_iter_has_child( GtkTreeModel *tree_model, GtkTreeIter *iter );

  /// Позволяет получить количество потомков элемента \a iter.
  ///
  /// Возвратит количество элементов в корне (т.е. элементов без потомков), если \a iter == NULL.
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param iter - указатель на нужный элемент-родитель.
  ///
  /// \return Количество потомков элемента, соответствующего итератору \a iter,
  /// либо, если \a iter == NULL, количество корневых элементов.
  static gint gp_stapler_iter_n_children( GtkTreeModel *tree_model, GtkTreeIter *iter );

  /// Позволяет получить потомок элемента \a parent по порядковому номеру \a n.
  ///
  /// Возвратит корневой элемент с порядковым номером \a n,
  /// если \a parent == NULL.
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param iter - возвращаемый элемент;
  /// \param parent - элемент-родитель, либо NULL;
  /// \param n - порядковый номер нужного элемента.
  ///
  /// \return TRUE, если удалось получить элемент, иначе -- FALSE.
  static gboolean gp_stapler_iter_nth_child( GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent, gint n );

  /// Устанавливает в \a iter указатель на родителя \a child.
  ///
  /// Если \a child -- корневой элемент и не имеет родителя,
  /// тогда \a iter устанавливается в невалидный итератор (#ITER_INVALID)
  /// и возвращается FALSE.
  ///
  /// \param tree_model - указатель на объект GpStapler;
  /// \param iter - возвращаемый элемент;
  /// \param child - элемент, родителя которого необходимо получить.
  ///
  /// \return TRUE, если в \a iter записан родитель \a child.
  static gboolean gp_stapler_iter_parent( GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *child );
/// @}


/// \name Инициализация интерфейсов.
/// @{
  /// Инициализация интерфейса GtkTreeDragDest.
  ///
  /// \param iface - интерфейс.
  static void gp_stapler_drag_dest_init( GtkTreeDragDestIface *iface );

  /// Инициализация интерфейса GtkTreeDragSource.
  ///
  /// \param iface - интерфейс.
  static void gp_stapler_drag_source_init( GtkTreeDragSourceIface *iface );

  /// Инициализация интерфейса GpIcaRenderer.
  ///
  /// \param iface - интерфейс.
  static void gp_stapler_ica_renderer_init( GpIcaRendererInterface *iface );

  /// Инициализация интерфейса GtkTreeModel.
  ///
  /// \param iface - интерфейс.
  static void gp_stapler_tree_model_init( GtkTreeModelIface *iface );
/// @}


/// \name Статические функции (не относящиеся к интерфейсам).
/// @{
  // TODO Пока функция в тестовом виде, может и совсем не нужна.
  static gboolean gp_stapler_button_press_event( GpIcaRenderer *self, GdkEvent *event, GtkWidget *widget );

  /// Позволяет получить заведомо невалидный итератор.
  ///
  /// \param stapler - указатель на объект GpStapler.
  static GtkTreeIter gp_stapler_get_iter_invalid( GpStapler *stapler );

  /// Позволяет получить итератор, указывающий на слой.
  ///
  /// \param stapler - указатель на объект GpStapler;
  /// \param layer_index - индекс слоя в массиве _GpStaplerPriv::layers.
  static GtkTreeIter gp_stapler_get_iter_on_layer( GpStapler *stapler, guint layer_index );

  /// Позволяет получить итератор, указывающий на тэг.
  ///
  /// \param stapler - указатель на объект GpStapler;
  /// \param tag_index - индекс тэга в массиве _GpStaplerPriv::tags.
  static GtkTreeIter gp_stapler_get_iter_on_tag( GpStapler *stapler, guint tag_index );

  /// Функция получения параметров.
  ///
  /// \param object - указатель на объект GpStapler;
  /// \param prop_id - идентификатор параметра;
  /// \param value - значение параметра;
  /// \param pspec -  описание параметра.
  static void gp_stapler_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec );

  /// Функция установки параметров.
  ///
  /// \param object - указатель на объект GpStapler;
  /// \param prop_id - идентификатор параметра;
  /// \param value - новое значение параметра;
  /// \param pspec -  описание параметра.
  static void gp_stapler_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec );

  /// Предварительное освобождение памяти, занятой объектом.
  ///
  /// \param object - указатель на объект GpStapler.
  static void gp_stapler_dispose( GObject *object );

  /// Окончательное освобождение памяти, занятой объектом.
  ///
  /// \param object - указатель на объект GpStapler.
  static void gp_stapler_finalize( GObject *object );
/// @}



G_DEFINE_TYPE_EXTENDED( GpStapler, gp_stapler, G_TYPE_INITIALLY_UNOWNED, 0,
                        G_IMPLEMENT_INTERFACE( GTK_TYPE_TREE_DRAG_DEST, gp_stapler_drag_dest_init )
                        G_IMPLEMENT_INTERFACE( GTK_TYPE_TREE_DRAG_SOURCE, gp_stapler_drag_source_init )
                        G_IMPLEMENT_INTERFACE( G_TYPE_GP_ICA_RENDERER, gp_stapler_ica_renderer_init )
                        G_IMPLEMENT_INTERFACE( GTK_TYPE_TREE_MODEL, gp_stapler_tree_model_init )
);



// Интерфейс GpIcaRenderer -->
  gboolean gp_stapler_button_press_event( GpIcaRenderer *self, GdkEvent *event, GtkWidget *widget )
  {
    GpStapler *stapler = GP_STAPLER(self);

    g_return_val_if_fail(stapler, FALSE);

    if(event->type == GDK_2BUTTON_PRESS)
    {
      GdkEventButton *ev = (GdkEventButton*)event;
      gdouble x_val, y_val;
      gp_ica_state_renderer_area_point_to_value( stapler->priv->state_renderer, ev->x, ev->y, &x_val, &y_val );

      g_debug("2click. Pix: [%d, %d]. Val: [%.2f, %.2f].", (gint)ev->x, (gint)ev->y, x_val, y_val);
    }

    return FALSE;
  }


  gint gp_stapler_get_completion( GpIcaRenderer *renderer, gint renderer_id )
  {
    g_return_val_if_fail( GP_STAPLER_IS(renderer), -1);
    GPtrArray *layers = GP_STAPLER(renderer)->priv->layers;

    if(layers->len == 0)
      return -1;

    guint i;
    guint64 total_finished = 0;

    for(i = 0; i < layers->len; i++)
    {
      Layer *layer = g_ptr_array_index(layers, i);

      if(layer_get_visible(layer))
        total_finished += layer_get_finished(layer);
      else
        total_finished += 1000;
    }

    return total_finished / layers->len;
  }


  const gchar *gp_stapler_get_name( GpIcaRenderer *stapler, gint renderer_id )
  {
    switch(renderer_id)
    {
      case REN_DATA:      return "Data renderer.";
    }

    return "Unknown renderer.";
  }



  GpIcaRendererType gp_stapler_get_renderer_type( GpIcaRenderer *stapler, gint renderer_id )
  {
    switch( renderer_id )
    {
      case REN_DATA:      return GP_ICA_RENDERER_VISIBLE;
    }

    return GP_ICA_RENDERER_UNKNOWN;
  }


  gint gp_stapler_get_renderers_num( GpIcaRenderer *stapler )
  {
    return N_RENDERERS;
  }


  GpIcaRendererAvailability gp_stapler_render( GpIcaRenderer *stapler, gint renderer_id, gint *x, gint *y, gint *width,
                                               gint *height )
  {
    GpStaplerPriv *priv = GP_STAPLER(stapler)->priv;

    if( priv->state->area_width < 64 || priv->state->area_height < 64 ) return GP_ICA_RENDERER_AVAIL_NONE; // FIXME magic num.

    if( ( renderer_id == REN_DATA ) && ( !priv->data_force_update ) && ( !priv->force_restack_layers ) )
    {
      gboolean finished_all = TRUE;
      guint i;
      for(i = 0; i < priv->layers->len; i++)
        if(layer_get_finished(g_ptr_array_index(priv->layers, i)) != 1000)
        {
          finished_all = FALSE;
          break;
        }

      if(finished_all)
          return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;
    }

    switch( renderer_id )
    {
      case REN_DATA:
      {
  TT_INIT
  TT_POINT
        pixman_image_t *icarenderer_data_pimage = priv->icarenderer_data_pimage;
        g_return_val_if_fail(icarenderer_data_pimage, GP_ICA_RENDERER_AVAIL_NONE);

        pixman_image_t *tmp_data_pimage = priv->tmp_data_pimage;

        guint layer_i;
        for(layer_i = 0; layer_i < priv->layers->len; layer_i++)
        {
          Layer *layer = g_ptr_array_index(priv->layers, layer_i);

          if(priv->data_force_update)
            if(G_UNLIKELY(!layer_force_update(layer, priv->state)))
              layer_set_finished(layer, 1000); //< Ошибка layer_force_update(), не трогаем слой.

          if(layer_get_finished(layer) != 1000 && layer_get_visible(layer))
            if(layer_render_tiles_pimage(layer))
              if(layer_get_finished(layer) != layer_get_prev_finished(layer))
                priv->force_restack_layers = TRUE;
        }
  TT_POINT

        if(priv->force_restack_layers || priv->data_force_update)
        {
          priv->data_force_update = FALSE;
          priv->force_restack_layers = FALSE;

          // Фон.
          pixman_image_composite(PIXMAN_OP_SRC, priv->background_pimage, NULL, icarenderer_data_pimage,
            0, 0, 0, 0, 0, 0, priv->state->visible_width, priv->state->visible_height);

          gint signed_layer_i; //< Знаковый тип, т.к. ниже проверяем на >= 0.
          for(signed_layer_i = priv->layers->len - 1; signed_layer_i >= 0; signed_layer_i--)
          {
            Layer *layer = g_ptr_array_index(priv->layers, signed_layer_i);

            if(layer_get_visible(layer))
            {
              if(layer_get_highlight(layer))
              {
                layer_place_on_icarenderer_data_pimage(layer, priv->state, tmp_data_pimage, PIXMAN_OP_SRC);
                pixman_image_composite(PIXMAN_OP_ATOP, priv->highlight_pimage, NULL, tmp_data_pimage,
                  0, 0, 0, 0, 0, 0, priv->state->visible_width, priv->state->visible_height);
                pixman_image_composite(PIXMAN_OP_OVER, tmp_data_pimage, NULL, icarenderer_data_pimage,
                  0, 0, 0, 0, 0, 0, priv->state->visible_width, priv->state->visible_height);
              }
              else
                layer_place_on_icarenderer_data_pimage(layer, priv->state, icarenderer_data_pimage, PIXMAN_OP_OVER);
            }
          }

          g_signal_emit(stapler, gp_stapler_signals[RESTACKED], 0);
        }
        else
          return GP_ICA_RENDERER_AVAIL_NOT_CHANGED;

  TT_POINT
  TT_PRINT
        if(x) *x = 0;
        if(y) *y = 0;
        if(width) *width = priv->state->visible_width;
        if(height) *height = priv->state->visible_height;
      }
      break;
    }

    return GP_ICA_RENDERER_AVAIL_ALL;
  }


  void gp_stapler_set_visible_size( GpIcaRenderer *stapler, gint width, gint height )
  {
    GP_STAPLER(stapler)->priv->data_force_update = TRUE;
  }


  void gp_stapler_set_shown( GpIcaRenderer *stapler, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y )
  {
    GP_STAPLER(stapler)->priv->data_force_update = TRUE;
  }


  void gp_stapler_set_surface( GpIcaRenderer *stapler, gint renderer_id, guchar *data, gint width, gint height,
                               gint stride )
  {
    GpStaplerPriv *priv = GP_STAPLER(stapler)->priv;

    switch(renderer_id)
    {
      case REN_DATA:
        if(priv->icarenderer_data_pimage)
          pixman_image_unref(priv->icarenderer_data_pimage);

        priv->icarenderer_data_pimage = pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height, (uint32_t*)data, 4 * width);

        g_return_if_fail(priv->icarenderer_data_pimage);

        if(priv->tmp_data_pimage)
          pixman_image_unref(priv->tmp_data_pimage);

        priv->tmp_data_pimage = pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height, NULL, 4 * width);

        g_return_if_fail(priv->tmp_data_pimage);
      break;

      default:
        g_warning("Wrong renderer_id in set_surface call: %d", renderer_id);
      break;
    }
  }
// Интерфейс GpIcaRenderer <--


// Интерфейс GtkTreeDragDest -->
  gboolean gp_stapler_drag_data_received( GtkTreeDragDest *drag_dest, GtkTreePath *dest_path,
                                          GtkSelectionData *selection_data )
  {
    GpStapler *stapler = GP_STAPLER(drag_dest);
    g_return_val_if_fail(stapler, FALSE);

    GtkTreeIter src_iter = { 0, };
    GtkTreeModel *src_model = NULL;
    GtkTreePath *src_path = NULL;
    gboolean rval = FALSE;

    if(G_UNLIKELY(!gtk_tree_get_row_drag_data(selection_data, &src_model, &src_path)))
    {
      g_critical("Failed to get row drag data.");
      goto out;
    }

    if(G_UNLIKELY(src_model != GTK_TREE_MODEL(drag_dest)))
    {
      g_debug("Foreigner model.");
      goto out;
    }

    if(G_UNLIKELY(!gp_stapler_get_iter( GTK_TREE_MODEL( drag_dest ), &src_iter, src_path )))
    {
      g_critical("Failed to get src iter.");
      goto out;
    }

    Layer *layer = g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(src_iter.user_data2));

    gint *indices, depth;
    indices = gtk_tree_path_get_indices(dest_path);
    depth   = gtk_tree_path_get_depth(dest_path);

    if(depth != 1)
    {
      g_debug("Only depth == 1 is supported now in %s", G_STRFUNC);
      goto out;
    }

    if(indices[0] < stapler->priv->layers->len && indices[0] >= 0)
    {
      if(gp_stapler_insert_tiler(GP_STAPLER( drag_dest ), layer_get_tiler( layer ), layer_get_tiles_type( layer ),
                                 dest_path, NULL))
        rval = TRUE;
    }
    else
      if(indices[0] == stapler->priv->layers->len)
      {
        if(gp_stapler_append_tiler(GP_STAPLER( drag_dest ), layer_get_tiler( layer ), layer_get_tiles_type( layer ),
                                   NULL))
          rval = TRUE;
      }

  out:
    if(src_path)
      gtk_tree_path_free(src_path);

    return rval;
  }


  gboolean gp_stapler_row_drop_possible( GtkTreeDragDest *drag_dest, GtkTreePath *dest_path,
                                         GtkSelectionData *selection_data )
  {
    GtkTreeModel *src_model = NULL;
    GtkTreePath *src_path = NULL;
    gboolean rval = FALSE;

    if(!gtk_tree_get_row_drag_data(selection_data, &src_model, &src_path))
      goto out;

    if(src_model != GTK_TREE_MODEL(drag_dest))
    {
      g_debug("Foreigner model.");
      goto out;
    }

    gint *indices, depth;
    indices = gtk_tree_path_get_indices(dest_path);
    depth   = gtk_tree_path_get_depth(dest_path);

    if(depth != 1)
    {
      g_debug("Only depth == 1 is supported now in %s", G_STRFUNC);
      goto out;
    }

    if(indices[0] <= GP_STAPLER(drag_dest)->priv->layers->len && indices[0] >= 0)
      rval = TRUE;

  out:
    if(src_path)
      gtk_tree_path_free(src_path);

    return rval;
  }
// Интерфейс GtkTreeDragDest <--


// Интерфейс GtkTreeDragSource -->
  gboolean gp_stapler_row_draggable( GtkTreeDragSource *drag_source, GtkTreePath *path )
  {
    return TRUE;
  }


  gboolean gp_stapler_drag_data_get( GtkTreeDragSource *drag_source, GtkTreePath *path,
                                     GtkSelectionData *selection_data )
  {
    if(gtk_tree_set_row_drag_data (selection_data, GTK_TREE_MODEL(drag_source), path))
      return TRUE;
    else
      return FALSE;
  }


  gboolean gp_stapler_drag_data_delete( GtkTreeDragSource *drag_source, GtkTreePath *path )
  {
    GtkTreeIter iter;

    if(gp_stapler_get_iter(GTK_TREE_MODEL( drag_source ), &iter, path ))
    {
      gp_stapler_remove_tiler(GP_STAPLER( drag_source ), &iter );
      return TRUE;
    }
    else
      return FALSE;
  }
// Интерфейс GtkTreeDragSource <--



// Интерфейс GtkTreeModel -->
  GtkTreeModelFlags gp_stapler_get_flags( GtkTreeModel *tree_model )
  {
    g_return_val_if_fail( GP_STAPLER_IS(tree_model), (GtkTreeModelFlags)0);

    return GTK_TREE_MODEL_ITERS_PERSIST;
  }


  gint gp_stapler_get_n_columns( GtkTreeModel *tree_model )
  {
    g_return_val_if_fail( GP_STAPLER_IS(tree_model), 0);

    return GP_TILER_TREE_MODEL_COLS_NUM;
  }


  GType gp_stapler_get_column_type( GtkTreeModel *tree_model, gint index )
  {
    g_return_val_if_fail( GP_STAPLER_IS(tree_model), G_TYPE_INVALID);
    g_return_val_if_fail(index < GP_TILER_TREE_MODEL_COLS_NUM && index >= 0, G_TYPE_INVALID);

    return GP_STAPLER_COL_TYPES[index];
  }


  gboolean gp_stapler_get_iter( GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreePath *path )
  {
    GpStapler *stapler = GP_STAPLER(tree_model);
    g_return_val_if_fail(stapler, FALSE);
    g_return_val_if_fail(iter != NULL, FALSE);

    gint *indices, depth;
    indices = gtk_tree_path_get_indices(path);
    depth   = gtk_tree_path_get_depth(path);

    if(depth != 1)
    {
      g_debug("Only depth == 1 is supported now in %s", G_STRFUNC);
      return FALSE;
    }

    // FIXME Пока считаем, что у нас в TreeView только layers.
    if(indices[0] < stapler->priv->layers->len && indices[0] >= 0)
    {
      *iter = gp_stapler_get_iter_on_layer( stapler, indices[0] );
      return TRUE;
    }
    else
    {
      *iter = gp_stapler_get_iter_invalid( stapler );
      return FALSE;
    }
  }


  GtkTreePath *gp_stapler_get_path( GtkTreeModel *tree_model, GtkTreeIter *iter )
  {
    GpStapler *stapler = GP_STAPLER(tree_model);
    g_return_val_if_fail(stapler, NULL);
    g_return_val_if_fail(iter != NULL, NULL);

    if(GPOINTER_TO_UINT(iter->user_data) != ITER_LAYER)
    {
      g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
        G_STRFUNC, GPOINTER_TO_UINT(iter->user_data));
      return NULL;
    }

    GtkTreePath *path = NULL;

    path = gtk_tree_path_new();
    gtk_tree_path_append_index(path, GPOINTER_TO_UINT(iter->user_data2));

    return path;
  }


  void gp_stapler_get_value( GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, GValue *value )
  {
    g_return_if_fail( GP_STAPLER_IS(tree_model));
    g_return_if_fail(iter != NULL);
    g_return_if_fail(column < GP_TILER_TREE_MODEL_COLS_NUM && column >= 0);

    GpStapler *stapler = GP_STAPLER(tree_model);

    g_value_init(value, GP_STAPLER_COL_TYPES[column]);

    switch(GPOINTER_TO_UINT(iter->user_data))
    {
      case ITER_LAYER:
      {
        GPtrArray *layers = stapler->priv->layers;
        guint layer_index = GPOINTER_TO_UINT(iter->user_data2);
        g_return_if_fail(layer_index < layers->len);

        Layer *layer = g_ptr_array_index(layers, layer_index);

        switch(column)
        {
          case GP_TILER_TREE_MODEL_COLS_TILER:
            g_value_set_object(value, layer_get_tiler(layer));
          break;

          case GP_TILER_TREE_MODEL_COLS_NAME:
            g_value_set_string(value, gp_tiler_get_name( layer_get_tiler( layer )));
          break;

          case GP_TILER_TREE_MODEL_COLS_VISIBLE:
            g_value_set_boolean(value, layer_get_visible(layer));
          break;

          case GP_TILER_TREE_MODEL_COLS_HIGHLIGHT:
            g_value_set_boolean(value, layer_get_highlight(layer));
          break;

          case GP_TILER_TREE_MODEL_COLS_UPD_TIMEOUT:
            g_value_set_uint(value, layer_get_update_data_timeout(layer));
          break;

          case GP_TILER_TREE_MODEL_COLS_DELTA_X:
          case GP_TILER_TREE_MODEL_COLS_DELTA_Y:
          {
            gdouble delta;

            if(column == GP_TILER_TREE_MODEL_COLS_DELTA_X)
              layer_get_deltas(layer, &delta, NULL);
            else
              layer_get_deltas(layer, NULL, &delta);

            g_value_set_double(value, delta);
          }
          break;

          case GP_TILER_TREE_MODEL_COLS_USER_OBJ_1:
          case GP_TILER_TREE_MODEL_COLS_USER_OBJ_2:
          case GP_TILER_TREE_MODEL_COLS_USER_OBJ_3:
          case GP_TILER_TREE_MODEL_COLS_USER_OBJ_4:
          case GP_TILER_TREE_MODEL_COLS_USER_OBJ_5:
          case GP_TILER_TREE_MODEL_COLS_USER_OBJ_6:
          case GP_TILER_TREE_MODEL_COLS_USER_OBJ_7:
          case GP_TILER_TREE_MODEL_COLS_USER_OBJ_8:
          case GP_TILER_TREE_MODEL_COLS_USER_OBJ_9:
            g_value_set_object(value, layer_get_user_object(layer, column));
          break;
        }
      }
      break;

      default:
        g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
          G_STRFUNC, GPOINTER_TO_UINT(iter->user_data));
      break;
    }
  }


  gboolean gp_stapler_iter_next( GtkTreeModel *tree_model, GtkTreeIter *iter )
  {
    g_return_val_if_fail( GP_STAPLER_IS(tree_model), FALSE);
    g_return_val_if_fail(iter != NULL, FALSE);

    GpStapler *stapler = GP_STAPLER(tree_model);
    GPtrArray *layers = stapler->priv->layers;

    IterTypes iter_type = GPOINTER_TO_UINT(iter->user_data);

    if(iter_type != ITER_LAYER)
    {
      g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
        G_STRFUNC, iter_type);
      return FALSE;
    }

    guint layer_index = GPOINTER_TO_UINT(iter->user_data2);
    g_return_val_if_fail(layer_index < layers->len, FALSE);

    if(layer_index == layers->len - 1)
      return FALSE;
    else
    {
      iter->user_data2 = GUINT_TO_POINTER(layer_index + 1);
      return TRUE;
    }
  }


  gboolean gp_stapler_iter_children( GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent )
  {
    GpStapler *stapler = GP_STAPLER(tree_model);
    g_return_val_if_fail(stapler, FALSE);
    g_return_val_if_fail(iter, FALSE);

    // FIXME Пока считаем, что у нас в TreeView только layers.
    if(parent == NULL)
    {
      if(stapler->priv->layers->len)
      {
        *iter = gp_stapler_get_iter_on_layer( stapler, 0 );
        return TRUE;
      }
      else
      {
        *iter = gp_stapler_get_iter_invalid( stapler );
        return FALSE;
      }
    }

    *iter = gp_stapler_get_iter_invalid( stapler );
    return FALSE;
  }


  gboolean gp_stapler_iter_has_child( GtkTreeModel *tree_model, GtkTreeIter *iter )
  {
    g_return_val_if_fail( GP_STAPLER_IS(tree_model), FALSE);
    g_return_val_if_fail(iter != NULL, FALSE);

    if(GPOINTER_TO_UINT(iter->user_data) != ITER_LAYER)
      g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
        G_STRFUNC, GPOINTER_TO_UINT(iter->user_data));

    return FALSE;
  }


  gint gp_stapler_iter_n_children( GtkTreeModel *tree_model, GtkTreeIter *iter )
  {
    GpStapler *stapler = GP_STAPLER(tree_model);
    g_return_val_if_fail(stapler, -1);

    // FIXME Пока считаем, что у нас в TreeView только layers.
    if(iter == NULL)
      return stapler->priv->layers->len;

    if(GPOINTER_TO_UINT(iter->user_data) != ITER_LAYER)
      g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
        G_STRFUNC, GPOINTER_TO_UINT(iter->user_data));

    return 0;
  }


  gboolean gp_stapler_iter_nth_child( GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *parent, gint n )
  {
    GpStapler *stapler = GP_STAPLER(tree_model);
    g_return_val_if_fail(stapler, FALSE);
    g_return_val_if_fail(iter, FALSE);

    // FIXME Пока считаем, что у нас в TreeView только layers.
    if(parent == NULL)
    {
      if(n < stapler->priv->layers->len)
      {
        *iter = gp_stapler_get_iter_on_layer( stapler, n );
        return TRUE;
      }
      else
      {
        *iter = gp_stapler_get_iter_invalid( stapler );
        return FALSE;
      }
    }

    *iter = gp_stapler_get_iter_invalid( stapler );
    return FALSE;
  }


  gboolean gp_stapler_iter_parent( GtkTreeModel *tree_model, GtkTreeIter *iter, GtkTreeIter *child )
  {
    GpStapler *stapler = GP_STAPLER(tree_model);
    g_return_val_if_fail(stapler, FALSE);
    g_return_val_if_fail(child, FALSE);
    g_return_val_if_fail(iter, FALSE);

    // FIXME Пока считаем, что у нас в TreeView только layers.
    *iter = gp_stapler_get_iter_invalid( stapler );
    return FALSE;
  }
// Интерфейс GtkTreeModel <--



// Вспомогательные функции для использования GpStapler'а в качестве GtkTreeModel для TreeView -->
  void gp_stapler_cell_renderer_delta_x_edited_cb( GpStapler *stapler, gchar *path_str, gchar *new_text,
                                                   GtkCellRendererText *cell_renderer )
  {
    g_return_if_fail( GP_STAPLER_IS(stapler));

    GtkTreePath *path = NULL;
      path = gtk_tree_path_new_from_string(path_str);

      gdouble delta = strtod(new_text, NULL);

      GtkTreeIter iter;
      if(gtk_tree_model_get_iter(GTK_TREE_MODEL(stapler), &iter, path))
        gp_stapler_set_tiler_deltas( stapler, &iter, &delta, NULL);
      else
        g_critical("Failed to get iter.");
    gtk_tree_path_free(path);
  }


  void gp_stapler_cell_renderer_delta_y_edited_cb( GpStapler *stapler, gchar *path_str, gchar *new_text,
                                                   GtkCellRendererText *cell_renderer )
  {
    g_return_if_fail( GP_STAPLER_IS(stapler));

    GtkTreePath *path = NULL;
      path = gtk_tree_path_new_from_string(path_str);

      gdouble delta = strtod(new_text, NULL);

      GtkTreeIter iter;
      if(gtk_tree_model_get_iter(GTK_TREE_MODEL(stapler), &iter, path))
        gp_stapler_set_tiler_deltas( stapler, &iter, NULL, &delta );
      else
        g_critical("Failed to get iter.");
    gtk_tree_path_free(path);
  }


  void gp_stapler_cell_renderer_highlight_toggled_cb( GpStapler *stapler, gchar *path_str,
                                                      GtkCellRendererToggle *cell_renderer )
  {
    g_return_if_fail( GP_STAPLER_IS(stapler));

    GtkTreePath *path = NULL;
      path = gtk_tree_path_new_from_string(path_str);

      GtkTreeIter iter;
      if(gtk_tree_model_get_iter(GTK_TREE_MODEL(stapler), &iter, path))
        gp_stapler_set_tiler_highlight( stapler, &iter, !gtk_cell_renderer_toggle_get_active( cell_renderer ));
      else
        g_critical("Failed to get iter.");
    gtk_tree_path_free(path);
  }


  void gp_stapler_cell_renderer_update_timeout_edited_cb( GpStapler *stapler, gchar *path_str, gchar *new_text,
                                                          GtkCellRendererText *cell_renderer )
  {
    g_return_if_fail( GP_STAPLER_IS(stapler));

    GtkTreePath *path = NULL;
      path = gtk_tree_path_new_from_string(path_str);

      GtkTreeIter iter;
      if(gtk_tree_model_get_iter(GTK_TREE_MODEL(stapler), &iter, path))
        gp_stapler_set_tiler_update_data_timeout( stapler, &iter, atoi( new_text ));
      else
        g_critical("Failed to get iter.");
    gtk_tree_path_free(path);
  }


  void gp_stapler_cell_renderer_visibility_toggled_cb( GpStapler *stapler, gchar *path_str,
                                                       GtkCellRendererToggle *cell_renderer )
  {
    g_return_if_fail( GP_STAPLER_IS(stapler));

    GtkTreePath *path = NULL;
      path = gtk_tree_path_new_from_string(path_str);

      GtkTreeIter iter;
      if(gtk_tree_model_get_iter(GTK_TREE_MODEL(stapler), &iter, path))
        gp_stapler_set_tiler_visible( stapler, &iter, !gtk_cell_renderer_toggle_get_active( cell_renderer ));
      else
        g_critical("Failed to get iter.");
    gtk_tree_path_free(path);
  }
// Вспомогательные функции для использования GpStapler'а в качестве GtkTreeModel для TreeView <--



gboolean gp_stapler_append_tiler( GpStapler *stapler, GpTiler *tiler, gint tiles_type, GtkTreeIter *out_iter )
{
  g_return_val_if_fail( GP_STAPLER_IS(stapler), FALSE);
  g_return_val_if_fail( GP_IS_TILER(tiler), FALSE);

  Layer *layer = layer_new(tiler, tiles_type);
  g_return_val_if_fail(layer, FALSE);

  g_ptr_array_add(stapler->priv->layers, layer);
  GtkTreeIter iter = gp_stapler_get_iter_on_layer( stapler, stapler->priv->layers->len - 1 );

  stapler->priv->data_force_update = TRUE;

  GtkTreePath *path = gp_stapler_get_path(GTK_TREE_MODEL( stapler ), &iter );
    gtk_tree_model_row_inserted(GTK_TREE_MODEL(stapler), path, &iter);
  gtk_tree_path_free(path);

  if(out_iter)
    *out_iter = iter;

  return TRUE;
}



static void gp_stapler_class_init( GpStaplerClass *klass )
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->get_property = gp_stapler_get_property;
  gobject_class->set_property = gp_stapler_set_property;
  gobject_class->dispose = gp_stapler_dispose;
  gobject_class->finalize = gp_stapler_finalize;

  g_object_class_install_property(gobject_class, PROP_STATE_RENDERER,
    g_param_spec_object(
      "state-renderer", _("State Renderer"),
      _("State Renderer to store information about state of surface"),
      G_TYPE_GP_ICA_STATE_RENDERER, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

  gp_stapler_signals[RESTACKED] =
    g_signal_new(
      "restacked",     //< Имя сигнала.
      G_TYPE_FROM_CLASS(klass), //< Тип класса, которому сигнал принадлежит.
      G_SIGNAL_ACTION, //< Флаг: сигнал -- action.
      0, //< Относительный указатель на функцию сигнала в структуре класса.
      NULL, NULL, //< Аккумулятор и его параметр (нужны например для сбора возвращаемых значений от коллбеков).
      g_cclosure_marshal_VOID__VOID,  //< Конвертер массивов параметров для вызова коллбеков.
      G_TYPE_NONE, 0);  //< Возвращаемое значение и количество входных параметров.

  g_type_class_add_private( klass, sizeof( GpStaplerPriv ) );
}



void gp_stapler_clear_tiler( GpStapler *stapler, GtkTreeIter *iter, gboolean clean_cache,
                             GpTileAccessFunc condition, gpointer user_data )
{
  g_return_if_fail(iter);
  g_return_if_fail( GP_STAPLER_IS(stapler));

  guint layer_index = GPOINTER_TO_UINT(iter->user_data2);
  g_return_if_fail(layer_index < stapler->priv->layers->len);

  // FIXME Пока считаем, что у нас в TreeView только layers.
  Layer *layer = g_ptr_array_index(stapler->priv->layers, layer_index);

  if(clean_cache)
    gp_tiler_cache_clean_by_condition(layer_get_tiler(layer), layer_get_tiles_type(layer), condition, user_data);

  layer_set_status_not_init(layer);
}



GtkTreeIter gp_stapler_get_iter_invalid( GpStapler *stapler )
{
  GtkTreeIter iter;

  iter.user_data = GUINT_TO_POINTER(ITER_INVALID);
  iter.user_data2 = NULL;
  iter.user_data3 = NULL;

  return iter;
}



GtkTreeIter gp_stapler_get_iter_on_layer( GpStapler *stapler, guint layer_index )
{
  GtkTreeIter iter;

  iter.user_data = GUINT_TO_POINTER(ITER_LAYER);
  iter.user_data2 = GUINT_TO_POINTER(layer_index);
  iter.user_data3 = NULL;

  return iter;
}



GtkTreeIter gp_stapler_get_iter_on_tag( GpStapler *stapler, guint tag_index )
{
  GtkTreeIter iter;

  iter.user_data = GUINT_TO_POINTER(ITER_TAG);
  iter.user_data2 = NULL;
  iter.user_data3 = GUINT_TO_POINTER(tag_index);

  return iter;
}



void gp_stapler_get_property( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec )
{
  GpStapler *stapler = GP_STAPLER(object);
  GpStaplerPriv *priv = stapler->priv;

  switch (prop_id)
  {
    case PROP_STATE_RENDERER:
      g_value_set_object(value, priv->state_renderer);
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(stapler, prop_id, pspec);
    break;
  }
}



GpIcaStateRenderer *gp_stapler_get_state_renderer( GpStapler *stapler )
{
  g_return_val_if_fail( GP_STAPLER_IS(stapler), NULL);
  return stapler->priv->state_renderer;
}



gint gp_stapler_get_tiler_tiles_type( GpStapler *stapler, GtkTreeIter *iter )
{
  if(GPOINTER_TO_UINT(iter->user_data) != ITER_LAYER)
  {
    g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
      G_STRFUNC, GPOINTER_TO_UINT(iter->user_data));
    return 0;
  }

  return layer_get_tiles_type(g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2)));
}



uint32_t gp_stapler_get_upper_tiler_in_coord(GpStapler *stapler, GtkTreeIter *upper_tiler_iter,
  gdouble x, gdouble y, gboolean visible_only)
{
  g_return_val_if_fail( GP_STAPLER_IS(stapler), 0);

  GPtrArray *layers = stapler->priv->layers;
  uint32_t pixel = 0;
  Layer *layer;
  gint i;

  for(i = 0; i < layers->len; i++)
  {
    layer = g_ptr_array_index(layers, i);

    if((layer_get_visible(layer) == FALSE) && (visible_only == TRUE))
      continue;

    if(( pixel = layer_get_pixel(g_ptr_array_index(layers, i), x, y) ))
      break;
  }

  if(upper_tiler_iter)
  {
    if(pixel)
      *upper_tiler_iter = gp_stapler_get_iter_on_layer( stapler, i );
    else
      *upper_tiler_iter = gp_stapler_get_iter_invalid( stapler );
  }

  return pixel;
}



guint gp_stapler_get_tilers_num( const GpStapler *stapler )
{
  return stapler->priv->layers->len;
}



static void gp_stapler_init( GpStapler *stapler )
{
  GpStaplerPriv *priv = stapler->priv = G_TYPE_INSTANCE_GET_PRIVATE(stapler, GP_STAPLER_TYPE, GpStaplerPriv);

  priv->layers = g_ptr_array_new_with_free_func((GDestroyNotify)layer_destroy);
  priv->tags = g_ptr_array_new_with_free_func((GDestroyNotify)g_free/*TODO*/);

  priv->data_force_update = TRUE;
  priv->force_restack_layers = TRUE;

  // background_pimage -->
    pixman_color_t background_color = {
      GP_BACKGROUND_RED   * 0xFFFF,
      GP_BACKGROUND_GREEN * 0xFFFF,
      GP_BACKGROUND_BLUE  * 0xFFFF,
      1                   * 0xFFFF
    };
    priv->background_pimage =  pixman_image_create_solid_fill(&background_color);

    if(!priv->background_pimage)
      g_critical("Failed to create pixman image for background.");
  // background_pimage <--

  // highlight_pimage -->
    pixman_color_t highlight_color = { 0x0fff, 0x0fff, 0xffff, 0x00ff };
    priv->highlight_pimage =  pixman_image_create_solid_fill(&highlight_color);

    if(!priv->highlight_pimage)
      g_critical("Failed to create pixman image for highlighting.");
  // highlight_pimage <--
}



void gp_stapler_set_property( GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec )
{
  GpStapler *stapler = GP_STAPLER(object);
  GpStaplerPriv *priv = stapler->priv;

  switch(prop_id)
  {
    case PROP_STATE_RENDERER:
      priv->state_renderer = g_value_get_object(value);

      if( priv->state_renderer != NULL )
      {
        g_object_ref_sink(priv->state_renderer);

        priv->state = gp_ica_state_renderer_get_state( priv->state_renderer );
      }
      else
      {
        g_critical("GpIcaStateRenderer == NULL.");
        g_object_unref(stapler);
      }
    break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(stapler, prop_id, pspec);
    break;
  }
}



void gp_stapler_set_tiler_deltas( GpStapler *stapler, GtkTreeIter *iter, gdouble *delta_x, gdouble *delta_y )
{
  if(delta_x || delta_y)
    stapler->priv->data_force_update = TRUE;
  else
    return;

  g_return_if_fail(iter);
  guint layer_index = GPOINTER_TO_UINT(iter->user_data2);
  g_return_if_fail(layer_index < stapler->priv->layers->len);

  Layer *layer = g_ptr_array_index(stapler->priv->layers, layer_index);
  layer_set_deltas(layer, delta_x, delta_y);

  {
    GtkTreeModel *model = GTK_TREE_MODEL(stapler);

    GtkTreePath *path = gp_stapler_get_path( model, iter );
      gtk_tree_model_row_changed(model, path, iter);
    gtk_tree_path_free(path);
  }
}


void gp_stapler_set_tiler_tiles_type( GpStapler *stapler, GtkTreeIter *iter, gint tiles_type )
{
  if(GPOINTER_TO_UINT(iter->user_data) != ITER_LAYER)
  {
    g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
      G_STRFUNC, GPOINTER_TO_UINT(iter->user_data));
    return;
  }

  Layer *layer = g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2));

  if(layer_set_tiles_type(layer, tiles_type))
    stapler->priv->data_force_update = TRUE;
}



void gp_stapler_set_tiles_type_for_all_tilers(GpStapler *stapler, gint tiles_type)
{
  g_return_if_fail(GP_STAPLER_IS(stapler));

  guint i;
  GPtrArray *layers = stapler->priv->layers;

  for(i = 0; i < layers->len; i++)
    if(layer_set_tiles_type(g_ptr_array_index(layers, i), tiles_type))
      stapler->priv->data_force_update = TRUE;
}



gboolean gp_stapler_get_tiler_highlight( GpStapler *stapler, GtkTreeIter *iter )
{
  g_return_val_if_fail( GP_STAPLER_IS(stapler), FALSE);

  Layer *layer = g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2));

  return layer_get_highlight(layer);
}



void gp_stapler_set_tiler_highlight( GpStapler *stapler, GtkTreeIter *iter, gboolean highlight )
{
  g_return_if_fail( GP_STAPLER_IS(stapler));

  Layer *layer = g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2));
  layer_set_highlight(layer, highlight);

  {
    GtkTreeModel *model = GTK_TREE_MODEL(stapler);

    GtkTreePath *path = gp_stapler_get_path( model, iter );
      gtk_tree_model_row_changed(model, path, iter);
    gtk_tree_path_free(path);
  }

  stapler->priv->force_restack_layers = TRUE;
}



guint gp_stapler_get_tiler_fixed_l(GpStapler *stapler, GtkTreeIter *iter, gint type)
{
  g_return_val_if_fail(GP_STAPLER_IS(stapler), 0);

  Layer *layer = g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2));

  return layer_get_fixed_l(layer, type);
}



void gp_stapler_set_tiler_fixed_l(GpStapler *stapler, GtkTreeIter *iter, gint type, guint l)
{
  g_return_if_fail( GP_STAPLER_IS(stapler));

  Layer *layer = g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2));
  layer_set_fixed_l(layer, type, l);

  if(layer_get_tiles_type(layer) == type)
    stapler->priv->data_force_update = TRUE;
}



void gp_stapler_set_tiler_visible( GpStapler *stapler, GtkTreeIter *iter, gboolean visible )
{
  g_return_if_fail( GP_STAPLER_IS(stapler));

  Layer *layer = g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2));
  layer_set_visible(layer, visible);

  {
    GtkTreeModel *model = GTK_TREE_MODEL(stapler);

    GtkTreePath *path = gp_stapler_get_path( model, iter );
      gtk_tree_model_row_changed(model, path, iter);
    gtk_tree_path_free(path);
  }

  stapler->priv->force_restack_layers = TRUE;
}



static void gp_stapler_dispose( GObject *object )
{
  GpStapler *stapler = GP_STAPLER(object);
  GpStaplerPriv *priv = stapler->priv;

  g_clear_object(&priv->state_renderer);

  if(priv->tags)
  {
    g_ptr_array_free(priv->tags, TRUE);
    priv->tags = NULL;
  }

  if(priv->layers)
  {
    g_ptr_array_free(priv->layers, TRUE);
    priv->layers = NULL;
  }

  G_OBJECT_CLASS(gp_stapler_parent_class)->dispose(object);
}



static void gp_stapler_finalize( GObject *object )
{
  GpStapler *stapler = GP_STAPLER(object);
  GpStaplerPriv *priv = stapler->priv;

  if(priv->icarenderer_data_pimage) pixman_image_unref(priv->icarenderer_data_pimage);
  if(priv->tmp_data_pimage) pixman_image_unref(priv->tmp_data_pimage);

  if(priv->background_pimage) pixman_image_unref(priv->background_pimage);
  if(priv->highlight_pimage) pixman_image_unref(priv->highlight_pimage);

  G_OBJECT_CLASS(gp_stapler_parent_class)->finalize(object);
}



gboolean gp_stapler_insert_tiler( GpStapler *stapler, GpTiler *tiler, gint tiles_type, GtkTreePath *dest_path,
                                  GtkTreeIter *out_iter )
{
  g_return_val_if_fail( GP_STAPLER_IS(stapler), FALSE);
  g_return_val_if_fail( GP_IS_TILER(tiler), FALSE);
  g_return_val_if_fail(dest_path, FALSE);

  GtkTreeModel *model = GTK_TREE_MODEL(stapler);
  GtkTreeIter iter = { 0, };

  if(G_LIKELY( gp_stapler_get_iter( model, &iter, dest_path )))
  {
    if(GPOINTER_TO_UINT(iter.user_data) != ITER_LAYER)
    {
      g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
        G_STRFUNC, GPOINTER_TO_UINT(iter.user_data));
      return FALSE;
    }

    GPtrArray *layers = stapler->priv->layers;

    guint layer_index = GPOINTER_TO_UINT(iter.user_data2);
    g_return_val_if_fail(layer_index < layers->len, FALSE);

    Layer *layer = layer_new(tiler, tiles_type);
    g_return_val_if_fail(layer, FALSE);

    // ~ g_ptr_array_insert -->
      g_ptr_array_set_size(layers, layers->len + 1);
      memmove(layers->pdata + layer_index + 1, layers->pdata + layer_index,
        (layers->len - layer_index) * sizeof(gpointer));
      g_ptr_array_index(layers, layer_index) = layer;
    // ~ g_ptr_array_insert <--

    gtk_tree_model_row_inserted(model, dest_path, &iter);

    stapler->priv->data_force_update = TRUE;

    if(out_iter)
      *out_iter = iter;

    return TRUE;
  }
  else
  {
    g_critical("Failed to get iter from dest_path.");
    return FALSE;
  }
}


gboolean gp_stapler_prepend_tiler( GpStapler *stapler, GpTiler *tiler, gint tiles_type, GtkTreeIter *out_iter )
{
  g_return_val_if_fail( GP_STAPLER_IS(stapler), FALSE);

  gboolean rval = FALSE;

  // FIXME Пока считаем, что у нас в TreeView только layers.
  if(GP_STAPLER(stapler)->priv->layers->len)
  {
    GtkTreePath *path = gtk_tree_path_new_first();
      rval = gp_stapler_insert_tiler( stapler, tiler, tiles_type, path, out_iter );
    gtk_tree_path_free(path);
  }
  else
    rval = gp_stapler_append_tiler( stapler, tiler, tiles_type, out_iter );

  return rval;
}



void gp_stapler_remove_tiler( GpStapler *stapler, GtkTreeIter *iter )
{
  g_return_if_fail( GP_STAPLER_IS(stapler));
  g_return_if_fail(iter != NULL);

  if(GPOINTER_TO_UINT(iter->user_data) != ITER_LAYER)
  {
    g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
      G_STRFUNC, GPOINTER_TO_UINT(iter->user_data));
    return;
  }

  g_ptr_array_remove_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2));

  stapler->priv->force_restack_layers = TRUE;

  {
    GtkTreeModel *model = GTK_TREE_MODEL(stapler);

    GtkTreePath *path = gp_stapler_get_path( model, iter );
      gtk_tree_model_row_deleted(model, path);
    gtk_tree_path_free(path);
  }
}



void gp_stapler_set_tiler_update_data_timeout( GpStapler *stapler, GtkTreeIter *iter, guint interval )
{
  g_return_if_fail( GP_STAPLER_IS(stapler));

  Layer *layer = g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2));
  layer_set_update_data_timeout(layer, interval);

  {
    GtkTreeModel *model = GTK_TREE_MODEL(stapler);

    GtkTreePath *path = gp_stapler_get_path( model, iter );
      gtk_tree_model_row_changed(model, path, iter);
    gtk_tree_path_free(path);
  }
}



void gp_stapler_set_user_object( GpStapler *stapler, GtkTreeIter *iter, GpTilerTreeModelCols col, GObject *user_object )
{
  g_return_if_fail( GP_STAPLER_IS(stapler));
  g_return_if_fail( GP_TILER_TREE_MODEL_COLS_USER_OBJ_1 <= col && col <= GP_TILER_TREE_MODEL_COLS_USER_OBJ_9 );
  g_return_if_fail(G_IS_OBJECT(user_object));

  if(G_UNLIKELY(GPOINTER_TO_UINT(iter->user_data) != ITER_LAYER))
  {
    g_critical("Only ITER_LAYER is supported now in %s, but %u type has been passed",
      G_STRFUNC, GPOINTER_TO_UINT(iter->user_data));
    return;
  }

  Layer *layer = g_ptr_array_index(stapler->priv->layers, GPOINTER_TO_UINT(iter->user_data2));
  layer_set_user_object(layer, col, user_object);
}



GpStapler *gp_stapler_new( GpIcaStateRenderer *state_renderer )
{
  return g_object_new( GP_STAPLER_TYPE,
    "state-renderer", state_renderer,
    NULL);
}



void gp_stapler_drag_dest_init( GtkTreeDragDestIface *iface )
{
  iface->drag_data_received = gp_stapler_drag_data_received;
  iface->row_drop_possible = gp_stapler_row_drop_possible;
}



void gp_stapler_drag_source_init( GtkTreeDragSourceIface *iface )
{
  iface->row_draggable = gp_stapler_row_draggable;
  iface->drag_data_delete = gp_stapler_drag_data_delete;
  iface->drag_data_get = gp_stapler_drag_data_get;
}



void gp_stapler_ica_renderer_init( GpIcaRendererInterface *iface )
{
  iface->get_renderers_num = gp_stapler_get_renderers_num;
  iface->get_renderer_type = gp_stapler_get_renderer_type;
  iface->get_name = gp_stapler_get_name;

  iface->set_surface = gp_stapler_set_surface;
  iface->set_visible_size = gp_stapler_set_visible_size;

  iface->get_completion = gp_stapler_get_completion;
  iface->set_total_completion = NULL;
  iface->render = gp_stapler_render;

  iface->set_swap = NULL;
  iface->set_angle = NULL;
  iface->set_shown = gp_stapler_set_shown;
  iface->set_pointer = NULL;

  iface->button_press_event = gp_stapler_button_press_event;
  iface->button_release_event = NULL;
  iface->key_press_event = NULL;
  iface->key_release_event = NULL;
  iface->motion_notify_event = NULL;
}



void gp_stapler_tree_model_init( GtkTreeModelIface *iface )
{
  iface->get_flags = gp_stapler_get_flags;
  iface->get_n_columns = gp_stapler_get_n_columns;
  iface->get_column_type = gp_stapler_get_column_type;
  iface->get_iter = gp_stapler_get_iter;
  iface->get_path = gp_stapler_get_path;
  iface->get_value = gp_stapler_get_value;
  iface->iter_next = gp_stapler_iter_next;
  iface->iter_children = gp_stapler_iter_children;
  iface->iter_has_child = gp_stapler_iter_has_child;
  iface->iter_n_children = gp_stapler_iter_n_children;
  iface->iter_nth_child = gp_stapler_iter_nth_child;
  iface->iter_parent = gp_stapler_iter_parent;
}

