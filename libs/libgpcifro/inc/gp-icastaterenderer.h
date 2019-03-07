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
 * \file gp-icastaterenderer.h
 *
 * \author Andrei Fadeev
 * \date 5.04.2013
 * \brief Заголовочный файл класса отслеживания состояния CifroArea и CifroImage.
 *
 * \defgroup GpIcaStateRenderer GpIcaStateRenderer - класс отслеживания состояния CifroArea и CifroImage.
 *
 * В процессе использования объектов \link CifroArea \endlink или \link CifroImage \endlink возникает
 * необходимость сохранения состояния области отображения информации. Эта информация передается каждому
 * модулю формирующему изображения. Таким образом каждый модуль имеет свою копию состояния области отображения.
 *
 * Существует ряд вычислений которые требуются для всех объектов формирующих изображения, к ним
 * относятся преобразования координат из одной системы в другую. Класс GpIcaStateRenderer может
 * использоваться одновременно всеми подобными объектами, избавляя их от необходимости
 * самостоятельных вычислений.
 *
 * Класс GpIcaStateRenderer реализует интерфейс \link GpIcaRenderer \endlink с псевдо изображением #GP_ICA_RENDERER_STATE.
 * Он должен быть зарегистрирован первым среди всех модулей формирования изображений. После этого
 * его можно использовать во всех остальных модулях. GpIcaStateRenderer сохраняет всю информацию
 * о состоянии в структуре #GpIcaState указатель на которую можно получить функцией #gp_ica_state_renderer_get_state.
 * Изменять значение полей этой структуры нельзя.
 *
 * Для пересчета значений из прямоугольной системы координат окна в значения и обратно используются
 * функции #gp_ica_state_renderer_area_point_to_value и #gp_ica_state_renderer_area_value_to_point.
 *
 * Для пересчета значений из прямоугольной системы видимой области в значения и обратно используются
 * функции #gp_ica_state_renderer_visible_point_to_value и #gp_ica_state_renderer_visible_value_to_point.
 *
 * Одной из распространенных задач является нанесения координатной сетки на изображение. Для
 * упрощения расчета координат и шага между линиями сетки предназначена функция #gp_ica_state_get_axis_step.
 *
 * Линии толщиной 1 пиксель библиотека cairo рисует без размытия если координата равна
 * ( целое число +- 0,5 ), функция #gp_ica_state_point_to_cairo выравнивает координату по этому правилу.
 *
*/

#ifndef _gp_ica_state_renderer_h
#define _gp_ica_state_renderer_h

#include <gp-icarenderer.h>


G_BEGIN_DECLS


#define G_TYPE_GP_ICA_STATE_RENDERER                     gp_ica_state_renderer_get_type()
#define GP_ICA_STATE_RENDERER( obj )                     ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_GP_ICA_STATE_RENDERER, GpIcaStateRenderer ) )
#define GP_ICA_STATE_RENDERER_CLASS( vtable )            ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_GP_ICA_STATE_RENDERER, GpIcaStateRendererClass ) )
#define GP_ICA_STATE_RENDERER_GET_CLASS( obj )           ( G_TYPE_INSTANCE_GET_INTERFACE ( ( obj ), G_TYPE_GP_ICA_STATE_RENDERER, GpIcaStateRendererClass ) )

GType gp_ica_state_renderer_get_type( void );


typedef struct _GpIcaStateRenderer GpIcaStateRenderer;
typedef struct _GpIcaStateRendererClass GpIcaStateRendererClass;

struct _GpIcaStateRenderer
{
  GInitiallyUnowned parent;
};

struct _GpIcaStateRendererClass
{
  GInitiallyUnownedClass parent_class;
};


/**
 * GpIcaState:
 * Структура состояния области #CifroArea и #CifroImage.
 */
typedef struct GpIcaState {

  gint               completion;                /* Общий прогресс формирования изображения. */

  gint               area_width;                /* Ширина окна объекта. */
  gint               area_height;               /* Высота окна объекта. */

  gint               visible_width;             /* Видимая ширина. */
  gint               visible_height;            /* Видимая высота. */

  gint               border_left;               /* Размер области обрамления с лево. */
  gint               border_right;              /* Размер области обрамления с право. */
  gint               border_top;                /* Размер области обрамления с верху. */
  gint               border_bottom;             /* Размер области обрамления с низу. */

  gboolean           swap_x;                    /* TRUE - ось x направлена в лево, FALSE - в право. */
  gboolean           swap_y;                    /* TRUE - ось y направлена в низ, FALSE - в верх. */

  gdouble            angle;                     /* Угол поворота изображения в радианах. */
  gdouble            angle_cos;                 /* Косинус угла поворота изображения. */
  gdouble            angle_sin;                 /* Синус угла поворота изображения. */

  gdouble            from_x;                    /* Граница отображения оси x с лево. */
  gdouble            to_x;                      /* Граница отображения оси x с право. */
  gdouble            from_y;                    /* Граница отображения оси y с низу. */
  gdouble            to_y;                      /* Граница отображения оси y с верху. */

  gdouble            min_x;                     /* Минимально возможное значение по оси x. */
  gdouble            max_x;                     /* Максимально возможное значение по оси x. */
  gdouble            min_y;                     /* Минимально возможное значение по оси y. */
  gdouble            max_y;                     /* Максимально возможное значение по оси y. */

  gdouble            cur_scale_x;               /* Текущий коэффициент масштаба по оси x. */
  gdouble            cur_scale_y;               /* Текущий коэффициент масштаба по оси y. */

  gint               pointer_x;                 /* Текущие координаты курсора или -1 при нахождении за пределами окна. */
  gint               pointer_y;                 /* Текущие координаты курсора или -1 при нахождении за пределами окна. */
  gdouble            value_x;                   /* Текущее значение в точке нахождения курсора. */
  gdouble            value_y;                   /* Текущее значение в точке нахождения курсора. */

} GpIcaState;


/**
 * gp_ica_state_renderer_new:
 *
 * Создание объекта #GpIcaStateRenderer.
 *
 * Returns: Указатель на объект #GpIcaStateRenderer.
 *
 */
GpIcaStateRenderer *gp_ica_state_renderer_new( void );


/**
 * gp_ica_state_renderer_get_state:
 * @state_renderer: Указатель на объект #GpIcaStateRenderer.
 *
 * Получение указателя на структуру состояния области #CifroArea или #CifroImage.
 * Функция возвращает указатель на внутреннюю структуру, изменение полей которой запрещено.
 *
 * Returns: Указатель на структуру состояния #GpIcaState.
 *
 */
const GpIcaState *gp_ica_state_renderer_get_state( GpIcaStateRenderer *state_renderer );


/**
 * gp_ica_state_renderer_area_point_to_value:
 * @state_renderer: Указатель на объект #GpIcaStateRenderer.
 * @x: Координата x в системе координат окна.
 * @y: Координата y в системе координат окна.
 * @x_val: (out): Координата x в логической системе координат или NULL.
 * @y_val: (out): Координата y в логической системе координат или NULL.
 *
 * Преобразование координат из прямоугольной системы окна в логические координаты,
 * отображаемые в объекте #CifroArea или #CifroImage.
 *
 */
void gp_ica_state_renderer_area_point_to_value( GpIcaStateRenderer *state_renderer, gdouble x, gdouble y, gdouble *x_val,
                                       gdouble *y_val );


/**
 * gp_ica_state_renderer_area_value_to_point:
 * @state_renderer: Указатель на объект #GpIcaStateRenderer.
 * @x: (out): Координата x в системе координат окна или NULL.
 * @y: (out): Координата y в системе координат окна или NULL.
 * @x_val: Координата x в логической системе координат.
 * @y_val: Координата y в логической системе координат.
 *
 * Преобразование координат из логических, отображаемых в объекте #CifroArea или #CifroImage,
 * в прямоугольную систему координат окна.
 *
 */
void gp_ica_state_renderer_area_value_to_point( GpIcaStateRenderer *state_renderer, gdouble *x, gdouble *y, gdouble x_val,
                                       gdouble y_val );


/**
 * gp_ica_state_renderer_visible_point_to_value:
 * @state_renderer: Указатель на объект #GpIcaStateRenderer.
 * @x: Координата x в системе координат видимой области.
 * @y: Координата y в системе координат видимой области.
 * @x_val: (out): Координата x в логической системе координат или NULL.
 * @y_val: (out): Координата y в логической системе координат или NULL.
 *
 * Преобразование координат из прямоугольной системы видимой области в логические координаты,
 * отображаемые в объекте #CifroArea или #CifroImage.
 *
 */
void gp_ica_state_renderer_visible_point_to_value( GpIcaStateRenderer *state_renderer, gdouble x, gdouble y, gdouble *x_val,
                                          gdouble *y_val );


/**
 * gp_ica_state_renderer_visible_value_to_point:
 * @state_renderer: Указатель на объект #GpIcaStateRenderer.
 * @x: (out): Координата x в системе координат видимой области или NULL.
 * @y: (out): Координата y в системе координат видимой области или NULL.
 * @x_val: Координата x в логической системе координат.
 * @y_val: Координата y в логической системе координат.
 *
 * Преобразование координат из логических, отображаемых в объекте #CifroArea или #CifroImage,
 * в прямоугольную систему координат видимой области.
 *
 */
void gp_ica_state_renderer_visible_value_to_point( GpIcaStateRenderer *state_renderer, gdouble *x, gdouble *y, gdouble x_val,
                                          gdouble y_val );


/**
 * gp_ica_state_get_axis_step:
 * @scale: Масштаб - число пикселей в одной логической единице.
 * @step_width: Желаемое расстояние между координатными осями.
 * @from: (out): Логическая координата первой линии сетки.
 * @step: (out): Логический шаг между линиями сетки или NULL.
 * @range: (out): "Цена" деления координатной сетки или NULL.
 * @power: (out): Степень "цены" деления координатной сетки или NULL.
 *
 * Расчет параметров прямоугольной координатной сетки.
 *
 */
void gp_ica_state_get_axis_step( gdouble scale, gdouble step_width, gdouble *from, gdouble *step, gint *range,
                                 gint *power );


/**
 * gp_ica_state_point_to_cairo:
 * @point: Координата для выравнивания.
 *
 * Выравнивание координат для использования в библиотеке cairo.
 *
 * Returns: Координата для использования в библиотеке cairo.
 *
 */
gdouble gp_ica_state_point_to_cairo( gdouble point );


G_END_DECLS


#endif // _gp_ica_state_renderer_h
