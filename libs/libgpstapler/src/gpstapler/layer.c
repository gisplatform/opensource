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
 * \file layer.c
 *
 * \author Sergey Volkhin
 * \date 29.10.2013
 *
 * Реализация объекта Layer, предназначенного для формирования одного слоя
 * (с данными от одного Tiler) для объекта Stapler.
 * Используется только внутри реализации объекта Stapler.
 *
*/


#include "layer.h"
#include <math.h>
#include <string.h>



/// Приводит размер плитки к сантиметрам (изначально размер хранится в виде значения двоичного логарифма стороны).
#define LL_TO_CM(L) (1UL << (L))

/// Приводит размер плитки к метрам (изначально размер хранится в виде значения двоичного логарифма стороны).
#define LL_TO_M(L) (((gdouble)LL_TO_CM(L)) / 100.)


/// Шаг масштабов плитки.
///
/// L_STEP -- коэффициент изменения масштаба для умножения на 2, т.е.
/// L_STEP = 1 => "соседние" масштабы отличаются в 2 раза,
/// L_STEP = 2 => "соседние" масштабы отличаются в 4 раза и т.п.
static const guint L_STEP = 1;



/// Информация, однозначно описывающая какими плитками заполнен слой
/// в данный момент (плитками какого размера и с какими координатами).
///
/// Заполняется с помощью compute_tiles_params().
/// Две такие структуры можно сравнить с помощью TILES_PARAMS_EQUAL().
typedef struct
{
  gint from_xl; /*!< Координата первой плитки по оси X (размерность -- сторона плитки).*/
  gint to_xl;   /*!< Координата последней плитки по оси X (размерность -- сторона плитки).*/
  gint from_yl; /*!< Координата первой плитки по оси Y (размерность -- сторона плитки).*/
  gint to_yl;   /*!< Координата последней плитки по оси Y (размерность -- сторона плитки).*/
  guint ll;     /*!< Автоматически рассчитанный размер стороны "плитки" (размер в сантиметрах = 2 ^ ll, т.е. ll -- двоичный логарифм стороны).*/

  ///\name Вспомогательные поля, вычисляются на основе предыдущих, т.ч. к примеру  не используются при сравнении.
  /// @{
    guint num;  /*!< Общее количество плиток.*/
    guint xnum; /*!< Количество плиток по оси X.*/
    guint ynum; /*!< Количество плиток по оси Y.*/
  /// @}

  /// Макрос сравнения объектов TilesParams.
  #define TILES_PARAMS_EQUAL(TP1, TP2) \
    ((TP1).from_xl == (TP2).from_xl && (TP1).to_xl == (TP2).to_xl && (TP1).from_yl == (TP2).from_yl && (TP1).to_yl == (TP2).to_yl && (TP1).ll == (TP2).ll)
}
TilesParams;



/// Информация о слое.
struct _Layer
{
  GpTiler *tiler;     /*!< Объект GpTiler, предназначенный для формирования квадратных изображений ("плиток").*/
  TilesParams tp;   /*!< Параметры плиток, которыми заполняется область отрисовки в данный момент.*/

  gint cur_type;  /*!< Тип плитки для данного слоя.*/
  gdouble delta_x;  /*!< Сдвиг слоя по оси X относительно информации из GpIcaState.*/
  gdouble delta_y;  /*!< Сдвиг слоя по оси Y относительно информации из GpIcaState.*/

  gint finished;      /*!< Число от 0 до 1000, обозначающее статус завершения отрисовки слоя (1000 -- полностью отрисован).*/
  gint prev_finished; /*!< Предыдущее значение finished (инициализируется в -1 по умолчанию).*/
  gboolean highlight; /*!< Флаг подсвечивания (выделения) слоя.*/
  gboolean visible;   /*!< Флаг видимости слоя.*/

  GpTileStatus *tile_statuses; /*!< Массив размером в tp.num элементов типа #GpTileStatus статусов плиток.*/
  uint32_t *tiles_buf;  /*!< Буфер, в котором раскладываются плитки в размере 1:1.*/
  pixman_image_t *tiles_pimage; /*!< Pixman image для раскладки плиток 1:1.*/

  guint l_max; /*!< Максимальный размер стороны плитки (двоичный логарифм стороны в физических единицах, сантиметрах).*/
  guint *fixed_l;  /*!< Фиксированные размеры сторон плиток (для конкретных типов).*/

  uint32_t *one_tile_buf;           /*!< Буфер под изображение #one_tile_pimage.*/
  pixman_image_t *one_tile_pimage;  /*!< Pixman image для отрисовки одной плитки.*/

  uint32_t *blank_tile_buf;   /*!< Буфер под изображение #blank_tile.*/
  pixman_image_t *blank_tile; /*!< Pixman image с общим для всех blank-плиток фоном.*/

  guint update_data_timeout_id; /*!< ID таймера, по которому вызывается gp_tiler_update_data().*/
  guint update_data_timeout_interval; /*!< Интервал таймера, по которому вызывается gp_tiler_update_data().*/

  GObject *user_object[9];    /*!< Пользовательские объекты.*/

  /*! Идентификатор обработчика сигнала "data-updated", вызывающего layer_set_status_not_actual.*/
  gulong data_updated_layer_set_status_not_actual_handler_id;
};



/// \name Статические функции.
/// @{
  /// Вычисляет параметры плиток, которыми должна быть заполнена область отрисовки в данный момент.
  ///
  /// \param state - состояние областей отрисовки.
  /// \param tp - указатель на структуру, в которую следует записать вычисленные параметры плиток.
  /// \param fixed_l - фиксированный размер плитки в см (0, если нужно рассчитать размер автоматически);
  /// \param l_max - максимальный размер стороны плитки (двоичный логарифм стороны в физических единицах).
  /// \param delta_x - сдвиг слоя по оси X относительно информации из GpIcaState.
  /// \param delta_y - сдвиг слоя по оси Y относительно информации из GpIcaState.
  static void compute_tiles_params(const GpIcaState *const state, TilesParams *tp, guint fixed_l, guint l_max, gdouble delta_x, gdouble delta_y);

  /// Формирует pixman_image_t с blank-плиткой данного размера.
  ///
  /// \note См. пример pixman-0.30.0/demos/radial-test.c, чтобы понять как работает эта функция.
  ///
  /// Вызывается один раз при добавлении нового слоя.
  ///
  /// \param pimage - pixman image (изначально пустой, заполненный нулями),
  /// на которой следует нарисовать фон blank-плитку.
  static void generate_blank_tile(pixman_image_t *pimage);
/// @}



void compute_tiles_params(const GpIcaState *const state, TilesParams *tp, guint fixed_l, guint l_max, gdouble delta_x, gdouble delta_y)
{
  guint ll = 0;
  guint l_cm;

  if(fixed_l == 0)
  { // Вычисляем длину стороны плитки для текущего масштаба -->
    guint cm_per_tile_x = (guint)(state->cur_scale_x * 100 * GP_TILE_SIDE);
    guint cm_per_tile_y = (guint)(state->cur_scale_y * 100 * GP_TILE_SIDE);
    guint cm_per_tile_min = MIN(cm_per_tile_x, cm_per_tile_y); // Сантиметров на сторону плитки.

    for(ll = 0; ll <= l_max; ll += L_STEP) // Перебираем размеры плитки с 1 до log2(l_max) с шагом в 2 * L_STEP раз.
      if(LL_TO_CM(ll) > cm_per_tile_min) // Что данных на пиксель в плитке уже больше, чем на экране.
      {
        if(ll != 0)
          ll -= L_STEP;

        break;
      }

    l_cm = LL_TO_CM(ll);
  } // Вычисляем длину стороны плитки для текущего масштаба <--
  else
    l_cm = fixed_l;

  // Ниже нужен именно floor(), нельзя просто в gint перевести,
  // иначе некорректно будет округляться до целого при отрицательных значениях.
  tp->from_xl = floor((state->from_x + delta_x) / l_cm * 100);
  tp->to_xl   = floor((state->to_x + delta_x) / l_cm * 100);
  tp->from_yl = floor((state->from_y + delta_y) / l_cm * 100);
  tp->to_yl   = floor((state->to_y + delta_y) / l_cm * 100);
  tp->ll = ll;

  tp->xnum = (1 + tp->to_xl - tp->from_xl);
  tp->ynum = (1 + tp->to_yl - tp->from_yl);
  tp->num =  tp->xnum * tp->ynum;
//g_print("{TP}Compute: L = %d, X: [%d;%d], Y:[%d:%d]\n", tp->ll, tp->from_xl, tp->to_xl, tp->from_yl, tp->to_yl);
}


void generate_blank_tile(pixman_image_t *pimage)
{
  pixman_image_t *src_img = NULL;

  gint width = pixman_image_get_width(pimage);
  gint height = pixman_image_get_height(pimage);

#if 0 //< Психоделическая плитка.
  #define BLANK_TILE_DOUBLE_TO_COLOR(x) (((uint32_t)((x)*65536)) - (((uint32_t)((x)*65536)) >> 16))
  #define BLANK_TILE_PIXMAN_STOP(offset,r,g,b,a)\
  {                                             \
    pixman_double_to_fixed (offset),            \
    {                                           \
    BLANK_TILE_DOUBLE_TO_COLOR (r),             \
    BLANK_TILE_DOUBLE_TO_COLOR (g),             \
    BLANK_TILE_DOUBLE_TO_COLOR (b),             \
    BLANK_TILE_DOUBLE_TO_COLOR (a)              \
    }                                           \
  }
    static const pixman_gradient_stop_t stops[] =
    {
      BLANK_TILE_PIXMAN_STOP(0.6, 0,   0,   0,   0.5),
      BLANK_TILE_PIXMAN_STOP(0.8, 0.5, 0.5, 0.5, 0.9),
      BLANK_TILE_PIXMAN_STOP(0.9, 0.5, 0.5, 0.5, 0.5)
    };
  #undef BLANK_TILE_DOUBLE_TO_COLOR
  #undef BLANK_TILE_PIXMAN_STOP

  { // Создаем src_img с радиальным градиентом -->
    pixman_point_fixed_t p0, p1;
    pixman_fixed_t r0, r1;
    double x0, x1, radius0, radius1, left, right, center;

    x0 = 0;
    x1 = 1;
    radius0 = 0;
    radius1 = 1.75;

    // Центрируем градиент.
    left = MIN (x0 - radius0, x1 - radius1);
    right = MAX (x0 + radius0, x1 + radius1);
    center = (left + right) * 0.5;
    x0 -= center;
    x1 -= center;

    // Меняем масштаб, чтобы градиент помещался в квадрат 1x1 с центром в (0,0).
    x0 *= 0.25;
    x1 *= 0.25;
    radius0 *= 0.25;
    radius1 *= 0.25;

    p0.x = pixman_double_to_fixed(x0);
    p0.y = pixman_double_to_fixed(0);

    p1.x = pixman_double_to_fixed(x1);
    p1.y = pixman_double_to_fixed(0);

    r0 = pixman_double_to_fixed(radius0);
    r1 = pixman_double_to_fixed(radius1);

    src_img = pixman_image_create_radial_gradient(&p0, &p1, r0, r1, stops, G_N_ELEMENTS(stops));
  } // Создаем src_img с радиальным градиентом <--

  pixman_transform_t transform;
  pixman_transform_init_identity(&transform);

  pixman_transform_translate(NULL, &transform,
    pixman_double_to_fixed(0.5),
    pixman_double_to_fixed(0.5));

  pixman_transform_scale(NULL, &transform,
    pixman_double_to_fixed(width),
    pixman_double_to_fixed(height));

  pixman_transform_translate(NULL, &transform,
    pixman_double_to_fixed(0.5),
    pixman_double_to_fixed(0.5));

  pixman_image_set_transform(src_img, &transform);
  pixman_image_set_repeat(src_img, PIXMAN_REPEAT_PAD);

#else //< Просто серая плитка.
  pixman_color_t c = {
    0x4000,
    0x4000,
    0x4000,
    0x4000
  };

  src_img = pixman_image_create_solid_fill(&c);
#endif

  static const int border = 6; //< Толщина рамки blank-плитки.

  if(width > (2 * border) && height > (2 * border))
  {
    memset(pixman_image_get_data(pimage), '\0', 4 * width * height);
    pixman_image_composite(PIXMAN_OP_SRC,
      src_img, NULL, pimage,
      0, 0, 0, 0, border, border, width - 2 * border, height - 2 * border);
  }
  else
    pixman_image_composite(PIXMAN_OP_SRC,
      src_img, NULL, pimage,
      0, 0, 0, 0, 0, 0, width, height);

  pixman_image_unref(src_img);
}



void layer_set_status_not_actual(Layer *layer)
{
  guint i;
  layer->finished = 0;
  layer->prev_finished = -1;
  for(i = 0; i < layer->tp.num; i++)
    if(layer->tile_statuses[i] > GP_TILE_STATUS_NOT_ACTUAL)
      layer->tile_statuses[i] = GP_TILE_STATUS_NOT_ACTUAL;
}



void layer_set_status_not_init(Layer *layer)
{
  guint i;
  layer->finished = 0;
  layer->prev_finished = -1;
  for(i = 0; i < layer->tp.num; i++)
    layer->tile_statuses[i] = GP_TILE_STATUS_NOT_INIT;

  memset(layer->tiles_buf, 0, 4 * layer->tp.num * GP_TILE_SIDE * GP_TILE_SIDE);
}



void layer_get_deltas(Layer *layer, gdouble *delta_x, gdouble *delta_y)
{
  if(delta_x) *delta_x = layer->delta_x;
  if(delta_y) *delta_y = layer->delta_y;
}



void layer_set_deltas(Layer *layer, gdouble *delta_x, gdouble *delta_y)
{
  if(delta_x) layer->delta_x = *delta_x;
  if(delta_y) layer->delta_y = *delta_y;
}



// !! Функция должна быть совместима с GDestroyNotify!
void layer_destroy(Layer *layer)
{
  g_return_if_fail(layer);

  g_signal_handler_disconnect(layer->tiler, layer->data_updated_layer_set_status_not_actual_handler_id);

  gp_tiler_drop_tasks(layer->tiler);

  g_object_unref(layer->tiler);

  g_free(layer->tile_statuses);

  if(layer->tiles_pimage) pixman_image_unref(layer->tiles_pimage);
  g_free(layer->tiles_buf);

  g_free(layer->fixed_l);

  if(layer->one_tile_pimage) pixman_image_unref(layer->one_tile_pimage);
  g_free(layer->one_tile_buf);

  if(layer->blank_tile) pixman_image_unref(layer->blank_tile);
  g_free(layer->blank_tile_buf);

  if(layer->update_data_timeout_id)
    g_source_remove(layer->update_data_timeout_id);

  g_free(layer);
}



gint layer_get_finished(Layer *layer)
{
  return layer->finished;
}



gint layer_get_prev_finished(Layer *layer)
{
  return layer->prev_finished;
}



void layer_set_finished(Layer *layer, gint finished)
{
  layer->finished = finished;
}



Layer *layer_new( GpTiler *tiler, gint tiles_type)
{
  G_STATIC_ASSERT(GP_TILE_SIDE != 0);
  G_STATIC_ASSERT(GP_TILE_SIDE <= G_MAXUINT16);

  g_return_val_if_fail(tiler, NULL);
  g_return_val_if_fail( GP_IS_TILER(tiler), NULL);

  // blank_tile -->
    uint32_t *blank_tile_buf = g_malloc(GP_TILE_DATA_SIZE);
    pixman_image_t *blank_tile = pixman_image_create_bits(PIXMAN_a8r8g8b8,
      GP_TILE_SIDE, GP_TILE_SIDE, blank_tile_buf, 4 * GP_TILE_SIDE);

    if(!blank_tile)
    {
      g_warning("Failed to create pixman image for background of blank tile.");
      g_free(blank_tile_buf);
      return NULL;
    }

    generate_blank_tile(blank_tile);
  // blank_tile <--

  // one_tile -->
    uint32_t *one_tile_buf = g_malloc(GP_TILE_DATA_SIZE);
    pixman_image_t *one_tile_pimage = pixman_image_create_bits(PIXMAN_a8r8g8b8,
      GP_TILE_SIDE, GP_TILE_SIDE, one_tile_buf, 4 * GP_TILE_SIDE);

    if(!one_tile_pimage)
    {
      g_warning("Failed to create pixman image for one tile of new layer.");
      pixman_image_unref(blank_tile);
      g_free(blank_tile_buf);
      g_free(one_tile_buf);
      return NULL;
    }
  // one_tile <--

  guint l_max;
  if(GP_TILE_SIDE != 1)
    l_max = 8 * sizeof(guint) + log2(GP_TILE_SIDE) - 1;
  else
    l_max = 8 * sizeof(guint) - 1;

  {
    Layer *layer = g_new0(Layer, 1);

    layer->tiler = tiler;
    g_object_ref_sink(tiler);

    layer->cur_type = tiles_type;

    layer->finished = 0;
    layer->prev_finished = -1;
    layer->highlight = FALSE;
    layer->visible = TRUE;

    layer->delta_x = 0;
    layer->delta_y = 0;

    layer->tile_statuses = NULL;
    layer->tiles_buf = NULL;
    layer->tiles_pimage = NULL;

    layer->l_max = l_max;
    layer->fixed_l = g_new0(guint, gp_tiler_get_tile_types_num(tiler));

    layer->one_tile_buf = one_tile_buf;
    layer->one_tile_pimage = one_tile_pimage;

    layer->blank_tile_buf = blank_tile_buf;
    layer->blank_tile = blank_tile;

    // При обновлении данных пометим уже сгенерированные плитки как неатуальные.
    layer->data_updated_layer_set_status_not_actual_handler_id = g_signal_connect_swapped(tiler,
      "data-updated", G_CALLBACK(layer_set_status_not_actual), layer);

    return layer;
  }
}



gboolean layer_force_update(Layer *layer, const GpIcaState *state)
{
  TilesParams new_tp;
  compute_tiles_params(state, &new_tp, layer->fixed_l[layer->cur_type], layer->l_max, layer->delta_x, layer->delta_y);

  // Пересоздадим layer->tiles_pimage если у нас изменилось кол-во плиток.
  if(layer->tp.xnum != new_tp.xnum || layer->tp.ynum != new_tp.ynum)
  {
    gint width = new_tp.xnum * GP_TILE_SIDE;
    gint height = new_tp.ynum * GP_TILE_SIDE;

    layer->tile_statuses = g_realloc(layer->tile_statuses, sizeof( GpTileStatus ) * new_tp.num);

    layer->tiles_buf = g_realloc(layer->tiles_buf, 4 * width * height);
    layer->tiles_pimage = pixman_image_create_bits(PIXMAN_a8r8g8b8, width, height, layer->tiles_buf, 4 * width);

    g_return_val_if_fail(layer->tiles_pimage, FALSE);

    pixman_image_set_repeat(layer->tiles_pimage, PIXMAN_REPEAT_NONE);
    pixman_image_set_filter(layer->tiles_pimage, PIXMAN_FILTER_GOOD, NULL, 0);
  }

  if(!TILES_PARAMS_EQUAL(layer->tp, new_tp))
  {
    layer->tp = new_tp;

    layer_set_status_not_init(layer);

    if(gp_tiler_get_tasks_num(layer->tiler) > 2 * (gint)new_tp.num || !gp_tiler_get_cache(layer->tiler))
    {
      g_debug("☺ Dropping tasks: tasks num = %d, new_tp.num = %d", gp_tiler_get_tasks_num(layer->tiler), new_tp.num);
      gp_tiler_drop_tasks(layer->tiler);
    }
  }

  return TRUE;
}



gint layer_get_tiles_type(Layer *layer)
{
  return layer->cur_type;
}



gboolean layer_set_tiles_type(Layer *layer, gint tiles_type)
{
  gint old_fixed_l = layer_get_fixed_l(layer, layer->cur_type);
  gint new_fixed_l = layer_get_fixed_l(layer, tiles_type);

  layer->cur_type = tiles_type;
  layer_set_status_not_init(layer);

  return (new_fixed_l != old_fixed_l);
}



uint32_t layer_get_pixel(Layer *layer, gdouble x, gdouble y)
{
  gdouble l_in_meters;

  if(layer->fixed_l[layer->cur_type] == 0)
    l_in_meters = LL_TO_M(layer->tp.ll);
  else
    l_in_meters = (double)layer->fixed_l[layer->cur_type] / 100;

  gdouble from_x = (gdouble)layer->tp.from_xl * l_in_meters - layer->delta_x;
  gdouble to_x = (gdouble)(layer->tp.to_xl + 1) * l_in_meters - layer->delta_x;
  gdouble from_y = (gdouble)layer->tp.from_yl * l_in_meters - layer->delta_y;
  gdouble to_y = (gdouble)(layer->tp.to_yl + 1) * l_in_meters - layer->delta_y;

  if (x < from_x || to_x < x) return 0;
  if (y < from_y || to_y < y) return 0;

  gint width = layer->tp.xnum * GP_TILE_SIDE;
  gint height = layer->tp.ynum * GP_TILE_SIDE;

  gdouble x_len = (gdouble)layer->tp.xnum * l_in_meters;
  gdouble y_len = (gdouble)layer->tp.ynum * l_in_meters;

  gint x_pix = (x - from_x) * width / x_len;
  gint y_pix = height - (y - from_y) * height / y_len;

  g_assert_cmpint(width * y_pix + x_pix, <, layer->tp.num * GP_TILE_SIDE * GP_TILE_SIDE);

  return layer->tiles_buf[width * y_pix + x_pix];
}



void layer_place_on_icarenderer_data_pimage(Layer *layer, const GpIcaState *state, pixman_image_t *icarenderer_data_pimage, pixman_op_t op)
{
  g_return_if_fail(layer->tiles_pimage);

  gdouble l_in_meters;

  if(layer->fixed_l[layer->cur_type] == 0)
    l_in_meters = LL_TO_M(layer->tp.ll);
  else
    l_in_meters = (double)layer->fixed_l[layer->cur_type] / 100;

  gdouble m_in_pix_on_tiles_pimage = (gdouble)l_in_meters / GP_TILE_SIDE;

  struct pixman_f_transform ftransform;
  pixman_f_transform_init_identity (&ftransform);

  pixman_f_transform_translate( NULL,&ftransform,
      ((gdouble)layer->tp.from_xl * l_in_meters - state->from_x - layer->delta_x) / m_in_pix_on_tiles_pimage,
      (-(gdouble)(layer->tp.to_yl + 1) * l_in_meters + state->to_y + layer->delta_y) / m_in_pix_on_tiles_pimage);

  pixman_f_transform_scale(NULL, &ftransform,
    m_in_pix_on_tiles_pimage / state->cur_scale_x,
    m_in_pix_on_tiles_pimage / state->cur_scale_y);

  {
    struct pixman_transform transform;
    pixman_transform_from_pixman_f_transform (&transform, &ftransform);
    pixman_image_set_transform(layer->tiles_pimage, &transform);
  }

  pixman_image_composite(op, layer->tiles_pimage, NULL, icarenderer_data_pimage,
    0, 0, 0, 0, 0, 0, state->visible_width, state->visible_height);
}



gboolean layer_render_tiles_pimage(Layer *layer)
{
  gboolean got_something_new_to_draw = FALSE;
  guint64 total_finished = 0;
  guint l;

  if(layer->fixed_l[layer->cur_type] == 0)
    l = LL_TO_CM(layer->tp.ll);
  else
    l  = layer->fixed_l[layer->cur_type];

  { // Генерация плиток -->

    GpTile tile = { .l = l, .type = layer->cur_type };

    GpTileStatus rval;
    guint i = 0; //< Номер плитки в массиве tile_statuses.

    for(tile.x = layer->tp.from_xl; tile.x <= layer->tp.to_xl; tile.x++)
      for(tile.y = layer->tp.from_yl; tile.y <= layer->tp.to_yl; tile.y++)
      {
        if(layer->tile_statuses[i] != GP_TILE_STATUS_ACTUAL)
        {
          rval = gp_tiler_get_tile(layer->tiler, (guint8 *) layer->one_tile_buf, &tile, layer->tile_statuses[i]);

          if(rval > 0)
            total_finished += rval;

          pixman_image_t *image_to_draw;

          // Если плитка не инициализирована на tiles_pimage и у нас нет данных,
          // чтобы нарисовать их, инициализируем плитку заглушкой.
          // Иначе -- рисуем ранее полученную плитку.
          if(layer->tile_statuses[i] == GP_TILE_STATUS_NOT_INIT && rval == GP_TILE_STATUS_NOT_INIT)
          {
            image_to_draw = layer->blank_tile;
            rval = GP_TILE_STATUS_INIT; //< Как бы сымитируем, что у нас есть что нарисовать.
          }
          else
            image_to_draw = layer->one_tile_pimage;

          // Рисуем, если данные изменились и есть хоть что-нибудь, что можно нарисовать.
          if(rval != layer->tile_statuses[i] && rval != GP_TILE_STATUS_NOT_INIT)
          {
            got_something_new_to_draw = TRUE;
            pixman_image_composite(PIXMAN_OP_SRC, image_to_draw, NULL, layer->tiles_pimage,
              0, 0, 0, 0,
              (tile.x - layer->tp.from_xl) * GP_TILE_SIDE,
              (layer->tp.to_yl - tile.y) * GP_TILE_SIDE,
              GP_TILE_SIDE, GP_TILE_SIDE);

            layer->tile_statuses[i] = rval;
          }
        }
        else
          total_finished += 1000;

        i++;
      }
  } // Генерация плиток <--

  layer->prev_finished = layer->finished;
  layer->finished = total_finished / layer->tp.num;

  return got_something_new_to_draw;
}



GpTiler *layer_get_tiler(Layer *layer)
{
  return layer->tiler;
}



gboolean layer_get_highlight(Layer *layer)
{
  return layer->highlight;
}



void layer_set_highlight(Layer *layer, gboolean highlight)
{
  layer->highlight = highlight;
}




guint layer_get_fixed_l(Layer *layer, gint type)
{
  g_return_val_if_fail(type >= 0 && type < gp_tiler_get_tile_types_num(layer->tiler), 0);
  return layer->fixed_l[type];
}



void layer_set_fixed_l(Layer *layer, gint type, guint l)
{
  g_return_if_fail(type >= 0 && type < gp_tiler_get_tile_types_num(layer->tiler));
  layer->fixed_l[type] = l;
}




gboolean layer_get_visible(Layer *layer)
{
  return layer->visible;
}



void layer_set_visible(Layer *layer, gboolean visible)
{
  layer->visible = visible;
}



guint layer_get_update_data_timeout(Layer *layer)
{
  return layer->update_data_timeout_interval;
}



void layer_set_update_data_timeout(Layer *layer, guint interval)
{
  if(layer->update_data_timeout_id)
  {
    g_source_remove(layer->update_data_timeout_id);
    layer->update_data_timeout_id = 0;
  }

  if(interval)
    layer->update_data_timeout_id = g_timeout_add( interval, (GSourceFunc) gp_tiler_update_data, layer->tiler );

  layer->update_data_timeout_interval = interval;
}



GObject *layer_get_user_object(Layer *layer, GpTilerTreeModelCols col)
{
  return layer->user_object[col - GP_TILER_TREE_MODEL_COLS_USER_OBJ_1];
}



void layer_set_user_object(Layer *layer, GpTilerTreeModelCols col, GObject *user_object)
{
  g_clear_object(&layer->user_object[col - GP_TILER_TREE_MODEL_COLS_USER_OBJ_1]);
  layer->user_object[col - GP_TILER_TREE_MODEL_COLS_USER_OBJ_1] = g_object_ref_sink(user_object);
}

