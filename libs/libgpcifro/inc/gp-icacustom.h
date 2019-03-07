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

/*
 * \file gp-icacustom.h
 *
 * \author Andrei Vodilov
 * \date 1.02.2016
 * \brief Заголовочный файл класса для отображения пользовательской информации (точки, метки, линии).
 *
 * \defgroup GpIcaCustom GpIcaCustom - класс для отображения пользовательской информации (точки, метки, линии).
 *
 * Класс реализует объект с интерфейсом \link GpIcaRenderer \endlink и
 * позволяет отображать точки, метки и ломанные.
 *
 * Точка описывается структурой типа \link GpIcaCustomPoints \endlink. Массив
 * точек передаётся через GArray в виде массива структур \link GpIcaCustomPoint \endlink.
 *
 * Точки можно добавлять или удалять. Для добавления точек необъодимо нажать
 * левую кнопку манипулятора при нажатой клавише Ctrl на клавиатуре. Аналогично
 * при нажатой клавише Ctrl можно удалить уже существующую точку. Также можно
 * удалить точку совместив её с одной из соседних.
 *
 * Добавлением и удалением точек можно управлять при помощи функций #gp_ica_custom_clear_points,
 * gp_ica_custom_add_point и gp_ica_custom_set_points. Массив текущих точек можно получить
 * функцией #gp_ica_custom_get_points. Цвет аналитической кривой и точек задаётся функциями
 * #gp_ica_custom_set_line_color и #gp_ica_custom_set_points_color соответственно.
 *
 * Создание объекта производится функцией gp_ica_custom_new.
 *
 *
*/

#ifndef _gp_ica_custom_h
#define _gp_ica_custom_h

#include <gp-icarenderer.h>
#include <gp-icastaterenderer.h>


G_BEGIN_DECLS

/**
 * GpIcaCustomDrawType:
 * @GP_ICA_CUSTOM_DRAW_TYPE_NA:         Не задан.
 * @GP_ICA_CUSTOM_DRAW_TYPE_POINT:      Отдельные точки
 * @GP_ICA_CUSTOM_DRAW_TYPE_LINE:       Линии (в т.ч. ломаные).
 * @GP_ICA_CUSTOM_DRAW_TYPE_AREA:       Произвольные многоугольники.
 * @GP_ICA_CUSTOM_DRAW_TYPE_SQUARE:     Прямоугольники .
 * @GP_ICA_CUSTOM_DRAW_TYPE_CUR_INDEX:  Выделение указанного индекса.
 * @GP_ICA_CUSTOM_DRAW_TYPE_PUNCHER:    Выделение множества областей (для удаления например).
 * @GP_ICA_CUSTOM_DRAW_TYPE_AMOUNT:     Количество типов.
 *
 * Тип данных для рисования.
 */
typedef enum
{
  GP_ICA_CUSTOM_DRAW_TYPE_NA = 0,
  GP_ICA_CUSTOM_DRAW_TYPE_POINT,
  GP_ICA_CUSTOM_DRAW_TYPE_LINE,
  GP_ICA_CUSTOM_DRAW_TYPE_LINE_FREE,
  GP_ICA_CUSTOM_DRAW_TYPE_AREA,
  GP_ICA_CUSTOM_DRAW_TYPE_SQUARE,
  GP_ICA_CUSTOM_DRAW_TYPE_CUR_INDEX,
  GP_ICA_CUSTOM_DRAW_TYPE_PUNCHER,
  GP_ICA_CUSTOM_DRAW_TYPE_AMOUNT
}
GpIcaCustomDrawType;

/**
 * GpIcaCustomLineType:
 * @GP_ICA_CUSTOM_LINE_TYPE_NA:           Не задан.
 * @GP_ICA_CUSTOM_LINE_TYPE_HORIZONTAL:   Горизонтальные линии
 * @GP_ICA_CUSTOM_LINE_TYPE_VERTICAL:     Вертикальные линии
 * @GP_ICA_CUSTOM_LINE_TYPE_AMOUNT:       Количество типов.
 *
 * Тип данных для рисования.
 */
typedef enum
{
  GP_ICA_CUSTOM_LINE_TYPE_NA = 0,
  GP_ICA_CUSTOM_LINE_TYPE_HORIZONTAL,
  GP_ICA_CUSTOM_LINE_TYPE_VERTICAL,
  GP_ICA_CUSTOM_LINE_TYPE_AMOUNT
}
GpIcaCustomLineType;

/**
* GpIcaCustomPoints:
* @x: X координата точки.
* @y: Y координата точки.
*
* Структура с описанием точки.
*/
typedef struct GpIcaCustomPoints
{
  gdouble x;
  gdouble y;
}
GpIcaCustomPoints;


#define G_TYPE_GP_ICA_CUSTOM                    gp_ica_custom_get_type()
#define GP_ICA_CUSTOM( obj )                    ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), G_TYPE_GP_ICA_CUSTOM, GpIcaCustom ) )
#define GP_ICA_CUSTOM_CLASS( vtable )           ( G_TYPE_CHECK_CLASS_CAST( ( vtable ), G_TYPE_GP_ICA_CUSTOM, GpIcaCustomClass ) )
#define GP_ICA_CUSTOM_GET_CLASS( obj )          ( G_TYPE_INSTANCE_GET_INTERFACE( ( obj ), G_TYPE_GP_ICA_CUSTOM, GpIcaCustomClass ) )


typedef struct _GpIcaCustom GpIcaCustom;
typedef struct _GpIcaCustomClass GpIcaCustomClass;

GType gp_ica_custom_get_type( void );


struct _GpIcaCustom
{
  GInitiallyUnowned parent;
};

struct _GpIcaCustomClass
{
  GInitiallyUnownedClass parent_class;
};


/**
 * gp_ica_custom_new:
 * @state_renderer: Объект #GpIcaStateRenderer.
 * @point_data: Пользовательские данные.
 *
 * Создание объекта.
 *
 * Returns: Указатель на созданный объект.
*/
GpIcaCustom *gp_ica_custom_new(GpIcaStateRenderer *state_renderer, gpointer point_data);


/**
 * gp_ica_custom_clear_points:
 * @self: Объект GpIcaCustom.
 *
 * Удалить все контрольные точки.
*/
void gp_ica_custom_clear_points(GpIcaCustom *self);


/**
 * gp_ica_custom_add_point:
 * @self: Объект GpIcaCustom.
 * @x: Координата точки x.
 * @y: Координата точки y.
 *
 * Добавить одну контрольную точку.
*/
void gp_ica_custom_add_point(GpIcaCustom *self, gdouble x, gdouble y);


/**
 * gp_ica_custom_set_points:
 * @self: Объект GpIcaCustom;
 * @selfs: (transfer none) (element-type GpIcaCustomPoints): Массив новых точек.
 *
 * Переопределить все точки.
*/
void gp_ica_custom_set_points(GpIcaCustom *self, GArray *selfs);


/**
 * gp_ica_custom_get_points:
 * @self: Объект GpIcaCustom.
 *
 * Получить массив контрольных точек.
 *
 * Returns: (transfer full) (element-type GpIcaCustomPoints): GArray с контрольными точками.
*/
GArray *gp_ica_custom_get_points(GpIcaCustom *self);

/**
 * gp_ica_custom_get_points_count:
 * @self: Объект GpIcaCustom.
 *
 * Получить количество контрольных точек.
 *
 * Returns: количество контрольных точкек.
*/
gint gp_ica_custom_get_points_count(GpIcaCustom *self);

/**
 * gp_ica_custom_get_num_point_coords:
 * @self: Объект GpIcaCustom.
 * @n: Номер порядковый точки.
 * @x: (out)(transfer full): x координата точки.
 * @y: (out)(transfer full): y координата точки.
 *
 * Получить контрольную точку с номером @n.
 *
 * Returns: TRUE, если точка найдена, FALSE в случае ошибки.
*/
gboolean gp_ica_custom_get_num_point_coords(GpIcaCustom *self, guint n, gdouble *x, gdouble *y);

/**
 * gp_ica_custom_update_num_point_coords:
 * @self: Объект GpIcaCustom.
 * @n: Номер порядковый точки.
 * @x: x координата точки.
 * @y: y координата точки.
 *
 * Обновить контрольную точку с номером @n.
 *
 * Returns: TRUE, если точка найдена, FALSE в случае ошибки.
*/
gboolean gp_ica_custom_update_num_point_coords(GpIcaCustom *self, guint n, gdouble x, gdouble y);

/**
 * gp_ica_custom_get_points_data:
 * @self: Объект GpIcaCustom.
 * @data: (out)(transfer full): Массив данных точек.
 * @len (transfer none): Количество точек.
 *
 * Получить массив данных контрольных точек.
 *
*/
void gp_ica_custom_get_points_data(GpIcaCustom *self, GpIcaCustomPoints *data, guint *len);

/**
 * gp_ica_custom_set_points_data:
 * @self: Объект GpIcaCustom.
 * @data: (transfer full)(array length=len): Массив данных точек.
 * @len: Количество точек.
 *
 * Установить массив данных контрольных точек.
 *
*/
void gp_ica_custom_set_points_data(GpIcaCustom *self, GpIcaCustomPoints *data, guint len);


/**
 * gp_ica_custom_set_line_color:
 * @self: Объект GpIcaCustom;
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета кривой.
*/
void gp_ica_custom_set_line_color(GpIcaCustom *self, gdouble red, gdouble green, gdouble blue);


/**
 * gp_ica_custom_set_points_color:
 * @self: Объект GpIcaCustom.
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета контрольных точек.
*/
void gp_ica_custom_set_points_color(GpIcaCustom *self, gdouble red, gdouble green, gdouble blue);

/**
 * gp_ica_custom_set_draw_type:
 * @self: Объект GpIcaCustom.
 * @new_type: Новый тип для рисования.
 *
 * Задать тип для рисования.
*/
void gp_ica_custom_set_draw_type(GpIcaCustom *self, GpIcaCustomDrawType new_type);

/**
 * gp_ica_custom_get_draw_type:
 * @self: указатель на объект GpIcaCustom.
 *
 * Определить тип рисования.
 *
 * Returns: Тип рисования #GpIcaCustomDrawType.
*/
GpIcaCustomDrawType gp_ica_custom_get_draw_type(GpIcaCustom *self);

/**
 * gp_ica_custom_set_line_type:
 * @self: Объект GpIcaCustom.
 * @new_type: Новый тип для рисования.
 *
 * Задать тип для линии (горизонталяная, вертикальная).
*/
void gp_ica_custom_set_line_type(GpIcaCustom *self, GpIcaCustomLineType new_type);

/**
 * gp_ica_custom_get_line_type:
 * @self: Объект GpIcaCustom.
 *
 * Определить тип линии (горизонталяная, вертикальная).
 *
 * Returns: Тип рисования #GpIcaCustomLineType.
*/
GpIcaCustomLineType gp_ica_custom_get_line_type(GpIcaCustom *self);

/**
 * gp_ica_custom_get_calc_value:
 * @self: Объект GpIcaCustom.
 *
 * Расчет значения измеряемого параметра.
 *
 * Returns: Значения измеряемого параметра в зависимости от типа.
*/
gdouble gp_ica_custom_get_calc_value(GpIcaCustom *self);

G_END_DECLS

#endif // _gp_ica_custom_h
