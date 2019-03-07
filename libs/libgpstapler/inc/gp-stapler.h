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

/*
 * \file stapler.h
 *
 * \author Sergey Volkhin
 * \date 19.09.2013
 *
 * \defgroup GpStapler GpStapler - объект для формирования изображений из плиток, сгенерированных объектами типа Tiler.
 * @{
 *
 * Реализует интерфейсы IcaRenderer, GtkTreeModel, GtkTreeDragSource и GtkTreeDragDest.
 *
 * К одному объекту GpStapler можно подключить множество объектов Tiler.
 * Плитки от разных объектов Tiler будут наложены друг на друга слоями (layers).
 *
 * Слои группируются в тэги. Дерево слоев-тэгов доступно через интерфейс GtkTreeModel.
 *
 * К каждой строке GtkTreeModel (будь то слой, либо тэг) пользователь
 * может назначить до 9 своих GObject-объектов посредством функции gp_stapler_set_user_object().
 * GpStapler при этом завладеет ссылкой на пользовательский объект.
 *
 * При добавлении генератора плиток необходимо указать тип плиток.
 * Тип плиток должен существовать в генераторе.
 * Тип можно поменять в любой момент с помощью #gp_stapler_set_tiler_tiles_type или #gp_stapler_set_tiles_type_for_all_tilers.
 *
 * Умеет работать с дополнительными возможностями, присущими только AsyncTiler'ам.
 *
*/

#ifndef _gp_stapler_h
#define _gp_stapler_h

#include <stdint.h> //< Для uint32_t.
#include "gp-icarenderer.h"
#include "gp-icastaterenderer.h"
#include "gp-tiler.h"

G_BEGIN_DECLS


#define GP_STAPLER_TYPE    gp_stapler_get_type()
#define GP_STAPLER(obj)      (G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), GP_STAPLER_TYPE, GpStapler ) )
#define GP_STAPLER_CLASS(vtable) (G_TYPE_CHECK_CLASS_CAST ( ( vtable ), GP_STAPLER_TYPE, GpStaplerClass ) )
#define GP_STAPLER_IS(obj)     (G_TYPE_CHECK_INSTANCE_TYPE((obj), GP_STAPLER_TYPE))
#define GP_STAPLER_CLASS_IS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GP_STAPLER_TYPE))
#define GP_STAPLER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ( ( obj ), GP_STAPLER_TYPE, GpStaplerClass ) )


typedef struct _GpStapler GpStapler;
typedef struct _GpStaplerClass GpStaplerClass;
typedef struct _GpStaplerPriv GpStaplerPriv;

GType gp_stapler_get_type( void );



struct _GpStapler
{
  GInitiallyUnowned parent;

  /*< private >*/
  GpStaplerPriv *priv;
};

struct _GpStaplerClass
{
  GInitiallyUnownedClass parent_class;

  // Сигналы -->
    /*
     * Сигнал "restacked", сигнализирующий о том, что GpStapler заново наложил слои друг на друга.
     * Это могло произойти по разным причинам: слои поменялись местами,
     * изменилась степень готовности одного из слоев, слой добавлился или удалился,
     * был сделан видимым или не видимым и так далее.
     * Если интересно следить за степенью отрисовки плиток на GpStapler'е,
     * то есть смысл именно по этому сигналу вызывать get_completion.
     */
    void (*restacked) ( GpStapler *stapler);
  // Сигналы <--
};

/**
 * gp_stapler_append_tiler:
 * @stapler: Объект типа #GpStapler.
 * @tiler: Объект типа #GpTiler, предназначенный для формирования плиток.
 * @tiles_type: Тип плиток, которые нужно формировать.
 * @out_iter: (out) (allow-none): переменная, куда будет помещен указатель на строку с вновь добавленным #GpTiler.
 *
 * Добавляет генератор плиток (в конец списка, без тэгов).
 *
 * Returns: TRUE в случае успеха, иначе -- FALSE.
 */
gboolean gp_stapler_append_tiler( GpStapler *stapler, GpTiler *tiler, gint tiles_type, GtkTreeIter *out_iter );



/**
 * gp_stapler_clear_tiler:
 * @stapler: объект #GpStapler.
 * @iter: Указатель на строку.
 * @clean_cache: Очищать ли кэш плиток.
 * @condition: (allow-none) (scope call): Функция-условие, для каждого элемента данных кэша
 * вызывается данная функция, первым аргументом ей передается указатель на данные в кэше,
 * вторым -- размер данных, третьим -- user_data.
 * Удаляются только те элементы данных, которые удовлетворяют условию (@condition вернула TRUE).
 * Если вместо @condition передан NULL и @clean_cache == TRUE, то функция удалит кэш целиком.
 * @user_data: указатель на пользовательские данные.
 *
 * Очищает плитки, отрисованные объектом #GpTiler.
 *
 * Инкрементирует revision, тем самым отбрасывает плитки,
 * генерируемые в данный момент.
 * Опционально очищает кэш плиток данного объекта Tiler.
 */
void gp_stapler_clear_tiler( GpStapler *stapler, GtkTreeIter *iter, gboolean clean_cache,
                             GpTileAccessFunc condition, gpointer user_data );



/**
 * gp_stapler_get_state_renderer:
 * @stapler: объект #GpStapler.
 *
 * Возвращает объект #IcaStateRenderer (псевдоотрисовщик для учета состояния областей), с которым работает данный #GpStapler.
 *
 * Returns: (transfer none): указатель на объект #IcaStateRenderer.
 */
GpIcaStateRenderer *gp_stapler_get_state_renderer( GpStapler *stapler );



/**
 * gp_stapler_get_tiler_tiles_type:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку.
 *
 * Возвращает тип плиток, относящиеся к которому плитки генерирует Tiler.
 *
 * Returns: тип плиток, относящиеся к которому плитки генерирует Tiler.
 */
gint gp_stapler_get_tiler_tiles_type( GpStapler *stapler, GtkTreeIter *iter );



/**
 * gp_stapler_get_upper_tiler_in_coord:
 * @stapler: указатель на объект #GpStapler.
 * @upper_tiler_iter: (out) (allow-none): переменная, куда будет записан указатель на найденный Tiler, либо NULL, если указатель не нужен.
 * @x: координата x в логической системе координат.
 * @y: координата y в логической системе координат.
 * @visible_only: искать только среди видимых слоев (иначе -- поиск идет среди всех слоев).
 *
 * Функция позволяет найти объект Tiler, у которого есть данные для отрисовки в точке
 * с переданными логическими координатами и который рисует свои данные поверх других объектов Tiler
 * (если таковые есть), у которых также есть данные для отрисовки в заданной точке.
 *
 * Т.е. функция находит "верхний" Tiler в заданной точке.
 *
 * Returns: значение пикселя в формате CAIRO_FORMAT_ARGB32, которое отрисовывает в заданной точке
 * найденный объект Tiler, либо 0, если ни один Tiler не рисует данные в заданной точке.
 */
uint32_t gp_stapler_get_upper_tiler_in_coord( GpStapler *stapler, GtkTreeIter *upper_tiler_iter,
  gdouble x, gdouble y, gboolean visible_only);



/**
 * gp_stapler_get_tilers_num:
 * @stapler: указатель на объект #GpStapler.
 *
 * Возвращает текущее количество генераторов плиток типа Tiler.
 *
 * Returns: количество генераторов плиток типа Tiler.
 */
guint gp_stapler_get_tilers_num( const GpStapler *stapler );



/**
 * gp_stapler_insert_tiler:
 * @stapler: указатель на объект #GpStapler.
 * @tiler: указатель на объект Tiler, предназначенный для формирования плиток.
 * @tiles_type: тип плиток, относящиеся к которому плитки нужно формировать.
 * @dest_path: место в GtkTreeView, куда следует поместить Tiler.
 * @out_iter: (out) (allow-none): переменная, куда будет помещен указатель на строку с вновь добавленным
 * объектом Tiler, либо NULL, если указатель не требуется.
 *
 * Добавляет генератор плиток (в @dest_path).
 *
 * Если нужно добавить генератор в самый конец списка,
 * то следует использовать gp_stapler_append_tiler().
 *
 * Returns: TRUE в случае успеха, иначе -- FALSE.
 */
gboolean gp_stapler_insert_tiler( GpStapler *stapler, GpTiler *tiler, gint tiles_type, GtkTreePath *dest_path,
                                  GtkTreeIter *out_iter );



/**
 * gp_stapler_new:
 * @state_renderer: указатель на объект #IcaStateRenderer (псевдоотрисовщик для учета состояния областей).
 *
 * Создание объекта.
 *
 * Создаст объект для выкладки данных на планшет с многопоточной отрисовкой с помощью "плиток".
 * Объект реализует интерфейс #IcaRenderer.
 *
 * Returns: (type GpStapler): указатель на объект.
 */
GpStapler *gp_stapler_new( GpIcaStateRenderer *state_renderer );



/**
 * gp_stapler_prepend_tiler:
 * @stapler: указатель на объект #GpStapler.
 * @tiler: указатель на объект Tiler, предназначенный для формирования плиток.
 * @tiles_type: тип плиток, относящиеся к которому плитки нужно формировать.
 * @out_iter: (out) (allow-none): переменная, куда будет помещен указатель на строку с вновь добавленным
 * объектом Tiler, либо NULL, если указатель не требуется.
 *
 * Добавляет генератор плиток (в начало списка, без тэгов).
 *
 * Returns: TRUE в случае успеха, иначе -- FALSE.
 */
gboolean gp_stapler_prepend_tiler( GpStapler *stapler, GpTiler *tiler, gint tiles_type, GtkTreeIter *out_iter );



/**
 * gp_stapler_remove_tiler:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку.
 *
 * Удаляет генератор плиток Tiler из данного #GpStapler.
 */
void gp_stapler_remove_tiler( GpStapler *stapler, GtkTreeIter *iter );



/**
 * gp_stapler_set_tiler_deltas:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку.
 * @delta_x: указатель на переменную, содержащую значение сдвига по оси X в метрах относительно
 * информации из #IcaState, либо NULL, если сдвиг оси X должен остаться прежним.
 * @delta_y: указатель на переменную, содержащую значение сдвига по оси Y в метрах относительно
 * информации из #IcaState, либо NULL, если сдвиг оси Y должен остаться прежним.
 *
 * Задает сдвиги в сантиметрах по осям для плиток, сгенерированным объектом Tiler.
 */
void gp_stapler_set_tiler_deltas( GpStapler *stapler, GtkTreeIter *iter, gdouble *delta_x, gdouble *delta_y );



/**
 * gp_stapler_set_tiler_tiles_type:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку.
 * @tiles_type: тип плиток, относящиеся к которому плитки нужно формировать.
 *
 * Устанавливает тип плиток, относящиеся к которому плитки нужно формировать.
 */
void gp_stapler_set_tiler_tiles_type( GpStapler *stapler, GtkTreeIter *iter, gint tiles_type );



/**
 * gp_stapler_set_tiles_type_for_all_tilers:
 * @stapler: указатель на объект #GpStapler.
 * @tiles_type: тип плиток, относящиеся к которому плитки нужно формировать.
 *
 * Устанавливает тип плиток, относящиеся к которому плитки нужно формировать всем объектам Tiler.
 */
void gp_stapler_set_tiles_type_for_all_tilers(GpStapler *stapler, gint tiles_type);



/**
 * gp_stapler_get_tiler_highlight:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку.
 *
 * Позволяет узнать подсвечены (выделены) ли данные, сгенерированные Tiler'ом.
 *
 * Returns: TRUE если данные подсвечены, иначе FALSE.
 */
gboolean gp_stapler_get_tiler_highlight( GpStapler *stapler, GtkTreeIter *iter );



/**
 * gp_stapler_set_tiler_highlight:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку.
 * @highlight: нужно ли подсвечивать данные этого Tiler'а.
 *
 * Позволяет указать нужно ли подсветить (выделить) данные, сгенерированные Tiler'ом.
 */
void gp_stapler_set_tiler_highlight( GpStapler *stapler, GtkTreeIter *iter, gboolean highlight );



/**
 * gp_stapler_get_tiler_fixed_l:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку (Tiler).
 * @type: тип плиток.
 *
 * Возвращает значение ширины плитки, если плитки этого типа в этом Tiler'е генерируются
 * с фиксированным масштабом.
 *
 * Returns: Ширину плиток в см, либо 0, если масштаб рассчитывается автоматически.
 */
guint gp_stapler_get_tiler_fixed_l(GpStapler *stapler, GtkTreeIter *iter, gint type);


/**
 * gp_stapler_set_tiler_fixed_l:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку (Tiler).
 * @type: тип плиток.
 * @l: новая фиксированная ширина плиток в см, либо 0 для автоматического рассчета ширины.
 *
 * Устанавливает значение ширины плитки, если плитки этого типа в этом Tiler'е должны генерироваться
 * с фиксированным масштабом. Либо устанавливает режим автоматического рассчета ширины плиток.
 */
void gp_stapler_set_tiler_fixed_l(GpStapler *stapler, GtkTreeIter *iter, gint type, guint l);



/**
 * gp_stapler_set_tiler_visible:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку.
 * @visible: новый флаг видимости.
 *
 * Задает флаг видимости плиток, сгенерированных объектом Tiler.
 */
void gp_stapler_set_tiler_visible( GpStapler *stapler, GtkTreeIter *iter, gboolean visible );



/**
 * gp_stapler_set_tiler_update_data_timeout:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на Tiler.
 * @interval: интервал обновления в миллисекундах,
 * либо 0, если нужно прекратить обновление данных для данного Tiler'а.
 *
 * Имеет смысл лишь для AsyncTiler'а.
 * Запускает таймер, по которому каждые @interval миллисекунд будет
 * вызываться алгоритим формирования рисуемых данных (update_data()) для Tiler'а.
 */
void gp_stapler_set_tiler_update_data_timeout( GpStapler *stapler, GtkTreeIter *iter, guint interval );



/**
 * gp_stapler_set_user_object:
 * @stapler: указатель на объект #GpStapler.
 * @iter: указатель на строку.
 * @col: колонка #GpTilerTreeModelCols (по сути -- какой из девяти объектов назначить).
 * @object: пользовательский объект.
 *
 * Назначает строке @iter некий пользовательский объект @object.
 *
 * При этом @stapler завладеет ссылкой на @object.
 * Если объект с номером @col уже был назначен,
 * значение ссылки старого объекта уменьшится на 1,
 * и на его место будет назначен новый объект.
 */
void gp_stapler_set_user_object( GpStapler *stapler, GtkTreeIter *iter, GpTilerTreeModelCols col, GObject *object );



/*
 * Вспомогательные функции для использования #GpStapler'а в качестве GtkTreeModel для TreeView.
 */

  /**
   * gp_stapler_cell_renderer_delta_x_edited_cb:
   * @stapler: указатель на объект #GpStapler.
   * @path_str: текстовое представление GtkTreePath, указывающее на строку с @cell_renderer.
   * @new_text: (inout): текст для @cell_renderer. Может быть изменен функцией,
   * если введенный пользователем чем-то не годится (например введены не (только) числа).
   * @cell_renderer: виджет, с помощью которого меняется значение сдвига.
   *
   * Позволяет изменить сдвиг слоя по оси X в метрах относительно информации из IcaState.
   *
   * Функция предназначена для использования в качестве обработчика сигнала "edited"
   * виджета @cell_renderer. Предполагается, что GtkTreeView, в котором
   * содержится @cell_renderer, в качестве GtkTreeModel использует @stapler.
   * Обработчик следует устанавливать с помощью g_signal_connect_swapped() и
   * по указателю @user_data передавать @stapler.
   */
  void gp_stapler_cell_renderer_delta_x_edited_cb( GpStapler *stapler, gchar *path_str, gchar *new_text,
                                                   GtkCellRendererText *cell_renderer );

  /**
   * gp_stapler_cell_renderer_delta_y_edited_cb:
   * @stapler: указатель на объект #GpStapler.
   * @path_str: текстовое представление GtkTreePath, указывающее на строку с @cell_renderer.
   * @new_text: (inout): текст для @cell_renderer. Может быть изменен функцией,
   * если введенный пользователем чем-то не годится (например введены не (только) числа).
   * @cell_renderer: виджет, с помощью которого меняется значение сдвига.
   *
   * Позволяет изменить сдвиг слоя по оси Y в метрах относительно информации из #IcaState.
   *
   * Функция предназначена для использования в качестве обработчика сигнала "edited"
   * виджета @cell_renderer. Предполагается, что GtkTreeView, в котором
   * содержится @cell_renderer, в качестве GtkTreeModel использует @stapler.
   * Обработчик следует устанавливать с помощью g_signal_connect_swapped() и
   * по указателю @user_data передавать @stapler.
   */
  void gp_stapler_cell_renderer_delta_y_edited_cb( GpStapler *stapler, gchar *path_str, gchar *new_text,
                                                   GtkCellRendererText *cell_renderer );

  /**
   * gp_stapler_cell_renderer_highlight_toggled_cb:
   * @stapler: указатель на объект #GpStapler.
   * @path_str: текстовое представление GtkTreePath, указывающее на строку с @cell_renderer.
   * @cell_renderer: виджет, с помощью которого меняется флаг подсвечивания.
   *
   * Задает флаг подсвечивания слоя с объектом Tiler.
   *
   * Функция предназначена для использования в качестве обработчика сигнала "toggled"
   * виджета @cell_renderer. Предполагается, что GtkTreeView, в котором
   * содержится @cell_renderer, в качестве GtkTreeModel использует @stapler.
   * Обработчик следует устанавливать с помощью g_signal_connect_swapped() и
   * по указателю @user_data передавать @stapler.
   */
  void gp_stapler_cell_renderer_highlight_toggled_cb( GpStapler *stapler, gchar *path_str,
                                                      GtkCellRendererToggle *cell_renderer );

  /**
   * gp_stapler_cell_renderer_update_timeout_edited_cb:
   * @stapler: указатель на объект #GpStapler.
   * @path_str: текстовое представление GtkTreePath, указывающее на строку с @cell_renderer.
   * @new_text: (inout): текст для @cell_renderer. Может быть изменен функцией,
   * если введенный пользователем чем-то не годится (например введены не (только) числа).
   * @cell_renderer: виджет, с помощью которого меняется значение интервала таймера.
   *
   * Запускает таймер, по которому будет вызываться алгоритим формирования рисуемых данных
   * (tiler_update_data()) для Tiler'а.
   *
   * Функция предназначена для использования в качестве обработчика сигнала "edited"
   * виджета @cell_renderer. Предполагается, что GtkTreeView, в котором
   * содержится @cell_renderer, в качестве GtkTreeModel использует @stapler.
   * Обработчик следует устанавливать с помощью g_signal_connect_swapped() и
   * по указателю @user_data передавать @stapler.
   */
  void gp_stapler_cell_renderer_update_timeout_edited_cb( GpStapler *stapler, gchar *path_str, gchar *new_text,
                                                          GtkCellRendererText *cell_renderer );

  /**
   * gp_stapler_cell_renderer_visibility_toggled_cb:
   * @stapler: указатель на объект #GpStapler.
   * @path_str: текстовое представление GtkTreePath, указывающее на строку с @cell_renderer.
   * @cell_renderer: виджет, с помощью которого меняется флаг видимости.
   *
   * Задает флаг видимости плиток, сгенерированных объектом Tiler.
   *
   * Функция предназначена для использования в качестве обработчика сигнала "toggled"
   * виджета @cell_renderer. Предполагается, что GtkTreeView, в котором
   * содержится @cell_renderer, в качестве GtkTreeModel использует @stapler.
   * Обработчик следует устанавливать с помощью g_signal_connect_swapped() и
   * по указателю @user_data передавать @stapler.
   */
  void gp_stapler_cell_renderer_visibility_toggled_cb( GpStapler *stapler, gchar *path_str,
                                                       GtkCellRendererToggle *cell_renderer );


G_END_DECLS

#endif // _gp_stapler_h


