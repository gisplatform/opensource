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
 * \file layer.h
 *
 * \author Sergey Volkhin
 * \date 29.10.2013
 *
 * Объект Layer предназначен для формирования одного слоя
 * (с данными от одного Tiler) для объекта Stapler.
 * Используется только внутри реализации объекта Stapler.
 *
*/

#ifndef _layer_h
#define _layer_h

#include <glib.h>
#include <pixman.h>
#include "gp-stapler.h"

G_BEGIN_DECLS



/// Информация о слое.
typedef struct _Layer Layer;



/// Создание объекта.
///
/// Создаст объект для формирования одного слоя для объекта GpStapler.
///
/// \param tiler - указатель на объект GpTiler, предназначенный для формирования плиток;
/// \param tiles_type - тип плиток для слоя.
///
/// \return указатель на объект, либо NULL в случае неудачи.
Layer *layer_new( GpTiler *tiler, gint tiles_type);



/// Помечает все актуальные плитки слоя как неактуальные,
/// слой считается несформированным (_Layer::finished выставляется в 0).
///
/// \param layer - указатель на объект Layer.
void layer_set_status_not_actual(Layer *layer);


/// Очищает слой: все плитки считаются неинициализированными (#GP_TILE_NOT_INIT),
/// слой заполняется нулевыми байтами (слой становится полностью прозрачным)
/// и считается несформированным (_Layer::finished выставляется в 0).
///
/// \param layer - указатель на объект Layer.
void layer_set_status_not_init(Layer *layer);


/// Позволяет получить сдвиги в метрах по осям для плиток в слое GpTiler.
///
/// \param layer - указатель на объект Layer.
/// \param delta_x - указатель на переменную, куда будет помещено значение сдвига по оси X
/// относительно информации из GpIcaState, либо NULL, если сдвиг по оси X не нужен.
/// \param delta_y - указатель на переменную, куда будет помещено значение сдвига по оси Y
/// относительно информации из GpIcaState, либо NULL, если сдвиг по оси Y не нужен.
void layer_get_deltas(Layer *layer, gdouble *delta_x, gdouble *delta_y);


/// Задает сдвиги в метрах по осям для плиток в слое GpTiler.
///
/// \param layer - указатель на объект Layer.
/// \param delta_x - указатель на переменную, содержащую значение сдвига по оси X
/// относительно информации из GpIcaState, либо NULL, если сдвиг оси X должен остаться прежним.
/// \param delta_y - указатель на переменную, содержащую значение сдвига по оси Y
/// относительно информации из GpIcaState, либо NULL, если сдвиг оси Y должен остаться прежним.
void layer_set_deltas(Layer *layer, gdouble *delta_x, gdouble *delta_y);


/// Уничтожает объект layer.
///
/// \note Функция может использоваться как GDestroyNotify.
///
/// \param layer - указатель на слой Layer.
void layer_destroy(Layer *layer);


/// Позволяет получить число (от 0 до 1000), обозначающее статус завершения отрисовки слоя
/// (1000 -- полностью отрисован).
///
/// \param layer - указатель на объект Layer.
///
/// \return - статут отрисовки.
gint layer_get_finished(Layer *layer);


/// Позволяет получить число (от 0 до 1000), обозначающее предыдущий статус завершения отрисовки слоя
/// (1000 -- был уже полностью отрисован).
///
/// \param layer - указатель на объект Layer.
gint layer_get_prev_finished(Layer *layer);


/// Задачет число (от 0 до 1000), обозначающее статус завершения отрисовки слоя
/// (1000 -- полностью отрисован).
///
/// \param layer - указатель на объект Layer.
/// \param finished - новый статут отрисовки.
void layer_set_finished(Layer *layer, gint finished);


/// Функция дает понять слою, что изменились параметры влияющие на отрисовку
/// (масштаб, координаты видимой области, сдвиги слоя по осям и т.п.),
/// из чего следует, что нужно пересчитать внутренние переменные,
/// возможно пересоздать внутренние буферы и поверхности,
/// а также пометить плитки как незавершенные.
///
/// \return TRUE в случае успеха, иначе -- FALSE.
gboolean layer_force_update(Layer *layer, const GpIcaState *state);



/// Функция позволяет получить объект GpTiler,
/// который генерирует плитки для данного слоя.
///
/// \param layer - указатель на объект Layer.
///
/// \return объект GpTiler.
GpTiler *layer_get_tiler(Layer *layer);



/// Позволяет получить тип плиток данного слоя.
///
/// \param layer - указатель на объект Layer.
///
/// \return тип плиток, относящиеся к которому плитки генерируются в слое.
gint layer_get_tiles_type(Layer *layer);



/**
 * layer_set_tiles_type:
 * @layer: Указатель на объект Layer.
 * @tiles_type: Тип плиток слоя.
 *
 * Позволяет установить тип плиток данного слоя.
 *
 * Returns: TRUE, если у нового типа плиток отличное от старого значение ширины плитки (fixed_l).
*/
gboolean layer_set_tiles_type(Layer *layer, gint tiles_type);



/// Функция позволяет получить значение пикселя, который соответствует точке
/// с переданными логическими координатами.
///
/// \param layer - указатель на объект Layer;
/// \param x - координата x в логической системе координат;
/// \param y - координата y в логической системе координат.
///
/// \return значение пикселя в формате CAIRO_FORMAT_ARGB32.
uint32_t layer_get_pixel(Layer *layer, gdouble x, gdouble y);


/// Ресайз и отрисовка плиток (tiles_pimage) слоя \a layer на \a icarenderer_data_pimage.
///
/// \param layer - указатель на объект Layer;
/// \param state - состояние областей отрисовки;
/// \param icarenderer_data_pimage - pixman-изображение, на котором следует отрисовать слой;
/// \param op - способ отрисовки слоя.
void layer_place_on_icarenderer_data_pimage(Layer *layer, const GpIcaState *state, pixman_image_t *icarenderer_data_pimage, pixman_op_t op);


/// Рендерит плитки слоя \a layer (в его внутренний tiles_pimage).
///
/// \return TRUE если отрисовано что-то новое по сравнению с прошлым вызовом, иначе -- FALSE.
gboolean layer_render_tiles_pimage(Layer *layer);


/**
 * layer_get_fixed_l:
 * @layer: указатель на объект Layer.
 * @type: тип плиток.
 *
 * Возвращает значение ширины плитки, если плитки этого типа в этом слое генерируются
 * с фиксированным масштабом.
 *
 * Returns: Ширину плиток в см, либо 0, если масштаб рассчитывается автоматически.
 */
guint layer_get_fixed_l(Layer *layer, gint type);


/**
 * layer_set_fixed_l:
 * @layer: указатель на объект Layer.
 * @type: тип плиток.
 * @l: новая фиксированная ширина плиток в см, либо 0 для автоматического рассчета ширины.
 *
 * Устанавливает значение ширины плитки, если плитки этого типа в этом слое должны генерироваться
 * с фиксированным масштабом. Либо устанавливает режим автоматического рассчета ширины плиток.
 */
void layer_set_fixed_l(Layer *layer, gint type, guint l);


/// Позволяет получить флаг видимости слоя.
///
/// \param layer - указатель на объект Layer
///
/// \return - флаг видимости.
gboolean layer_get_visible(Layer *layer);


/// Задает флаг видимости слоя.
///
/// \param layer - указатель на объект Layer
/// \param visible - новый флаг видимости.
void layer_set_visible(Layer *layer, gboolean visible);


/// Позволяет получить флаг подсвечивания (выделения) слоя.
///
/// \param layer - указатель на объект Layer;
///
/// \return - флаг подсвечивания (выделения) слоя.
gboolean layer_get_highlight(Layer *layer);


/// Задает флаг подсвечивания (выделения) слоя.
///
/// \param layer - указатель на объект Layer;
/// \param highlight - новый флаг подсвечивания (выделения) слоя.
void layer_set_highlight(Layer *layer, gboolean highlight);



/// Получение интервала работы таймера, по которому
/// вызывается алгоритм формирования рисуемых данных (gp_tiler_update_data()).
///
/// \param layer - указатель на объект Layer.
///
/// \return - Интервал обновления в миллисекундах,
/// либо 0, если таймер не запущен.
guint layer_get_update_data_timeout(Layer *layer);



/// Запускает таймер, по которому каждые \a interval миллисекунд будет
/// вызываться алгоритм формирования рисуемых данных (gp_tiler_update_data()).
///
/// \param layer - указатель на объект Layer;
/// \param interval - интервал обновления в миллисекундах,
/// либо 0, если нужно прекратить обновление данных.
void layer_set_update_data_timeout(Layer *layer, guint interval);



/// Позволяет получить назначенный слою \a layer пользовательский объект.
///
/// \param layer - указатель на объект Layer;
/// \param col - колонка GpTilerTreeModelCols (по сути -- какой из девяти объектов назначить).
///
/// \return Пользовательский объект GObject, либо NULL,
/// если объект, соответствующий номеру \a col еще не был назначен.
GObject *layer_get_user_object(Layer *layer, GpTilerTreeModelCols col);



/// Назначает слою \a layer некий пользовательский объект \a object.
///
/// При этом \a layer завладеет ссылкой на \a object.
/// Если объект с номером \a col уже был назначен,
/// значение ссылки старого объекта уменьшится на 1,
/// и на его место будет назначен новый объект.
///
/// \param layer - указатель на объект Layer;
/// \param col - колонка GpTilerTreeModelCols (по сути -- какой из девяти объектов назначить);
/// \param user_object - пользовательский объект.
void layer_set_user_object(Layer *layer, GpTilerTreeModelCols col, GObject *user_object);



G_END_DECLS

#endif // _layer_h
