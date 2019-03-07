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

/*
 * \file gp-icacurve.h
 *
 * \author Andrei Fadeev
 * \date 2.06.2014
 * \brief Заголовочный файл класса отображения параметрической кривой.
 *
 * \defgroup GpIcaCurve GpIcaCurve - класс отображения параметрической кривой.
 *
 * Класс реализует объект с интерфейсом \link GpIcaRenderer \endlink и
 * позволяет отображать параметрическую кривую.
 *
 * Параметры кривой определяются местоположением контрольных точек.
 * Точки можно перемещать при помощи манипулятора при нажатой левой кнопке.
 * Точка выбранная для перемещения обрамляется окружностью.
 *
 * Точка описывается структурой типа \link GpIcaCurvePoint \endlink. Массив
 * точек передаётся через GArray в виде массива структур \link GpIcaCurvePoint \endlink.
 *
 * Точки можно добавлять или удалять. Для добавления точек необъодимо нажать
 * левую кнопку манипулятора при нажатой клавише Ctrl на клавиатуре. Аналогично
 * при нажатой клавише Ctrl можно удалить уже существующую точку. Также можно
 * удалить точку совместив её с одной из соседних.
 *
 * Аналитический вид кривой расчитывается функцией типа #GpIcaCurveFunc, в неё
 * передаются все точки существующие на данный момент и указатель на пользовательские
 * данные.
 *
 * Добавлением и удалением точек можно управлять при помощи функций #gp_ica_curve_clear_points,
 * gp_ica_curve_add_point и gp_ica_curve_set_points. Массив текущих точек можно получить
 * функцией #gp_ica_curve_get_points. Цвет аналитической кривой и точек задаётся функциями
 * #gp_ica_curve_set_curve_color и #gp_ica_curve_set_point_color соответственно.
 *
 * Создание объекта производится функцией gp_ica_curve_new.
 *
 *
*/

#ifndef _gp_ica_curve_h
#define _gp_ica_curve_h

#include <gp-icarenderer.h>
#include <gp-icastaterenderer.h>


G_BEGIN_DECLS


/**
* GpIcaCurvePoint:
* @x: X координата точки.
* @y: Y координата точки.
* Структура с описанием точки.
*/
typedef struct GpIcaCurvePoint
{
  gdouble x;
  gdouble y;
}
GpIcaCurvePoint;


/**
 * GpIcaCurveFunc:
 * @param: Переменная функции.
 * @points: (transfer none) (element-type GpIcaCurvePoint): Массив точек параметров функции.
 * @user_data: Данные пользователя.
 *
 * Callback-функция для расчета аналитической кривой.
 *
 * Returns: Значение функции.
 */
typedef gdouble (*GpIcaCurveFunc)(gdouble param, GArray *points, gpointer user_data);



#define G_TYPE_GP_ICA_CURVE                    gp_ica_curve_get_type()
#define GP_ICA_CURVE( obj )                    ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), G_TYPE_GP_ICA_CURVE, GpIcaCurve ) )
#define GP_ICA_CURVE_CLASS( vtable )           ( G_TYPE_CHECK_CLASS_CAST( ( vtable ), G_TYPE_GP_ICA_CURVE, GpIcaCurveClass ) )
#define GP_ICA_CURVE_GET_CLASS( obj )          ( G_TYPE_INSTANCE_GET_INTERFACE( ( obj ), G_TYPE_GP_ICA_CURVE, GpIcaCurveClass ) )


typedef struct _GpIcaCurve GpIcaCurve;
typedef struct _GpIcaCurveClass GpIcaCurveClass;

GType gp_ica_curve_get_type( void );


struct _GpIcaCurve
{
  GInitiallyUnowned parent;
};

struct _GpIcaCurveClass
{
  GInitiallyUnownedClass parent_class;
};


/**
 * gp_ica_curve_new:
 * @state_renderer: Указатель на объект #GpIcaStateRenderer.
 * @curve_func: (closure curve_data) (scope notified): Функция расчета кривой по заданным точкам.
 * @curve_data: Пользовательские данные для передачи в curve_func.
 *
 * Создание объекта.
 *
 * Returns: Указатель на созданный объект.
*/
GpIcaCurve *gp_ica_curve_new(GpIcaStateRenderer *state_renderer, GpIcaCurveFunc curve_func, gpointer curve_data);


/**
 * gp_ica_curve_clear_points:
 * @curve: Объект #GpIcaCurve.
 *
 * Удалить все контрольные точки.
*/
void gp_ica_curve_clear_points(GpIcaCurve *curve);


/**
 * gp_ica_curve_add_point:
 * @curve: Объект #GpIcaCurve.
 * @x: Координата точки x.
 * @y: Координата точки y.
 *
 * Добавить одну контрольную точку.
*/
void gp_ica_curve_add_point(GpIcaCurve *curve, gdouble x, gdouble y);


/**
 * gp_ica_curve_set_points:
 * @curve: Объект #GpIcaCurve.
 * @points: (transfer none) (element-type GpIcaCurvePoint): Массив новых точек.
 *
 * Переопределить все точки.
*/
void gp_ica_curve_set_points(GpIcaCurve *curve, GArray *points);


/**
 * gp_ica_curve_get_points:
 * @curve: указатель на объект GpIcaCurve.
 *
 * Получить массив контрольных точек.
 *
 * Returns: (transfer full) (element-type GpIcaCurvePoint): GArray с контрольными точками.
*/
GArray *gp_ica_curve_get_points(GpIcaCurve *curve);


/**
 * gp_ica_curve_set_curve_color:
 * @curve: Объект #GpIcaCurve.
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета кривой.
*/
void gp_ica_curve_set_curve_color(GpIcaCurve *curve, gdouble red, gdouble green, gdouble blue);


/**
 * gp_ica_curve_set_point_color:
 * @curve: Объект #GpIcaCurve.
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета контрольных точек.
*/
void gp_ica_curve_set_point_color(GpIcaCurve *curve, gdouble red, gdouble green, gdouble blue);


G_END_DECLS

#endif // _gp_ica_curve_h
