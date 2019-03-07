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
 * \file gp-cifrocurve.h
 *
 * \author Andrei Fadeev
 * \date 22.07.2014
 *
 * \brief Заголовочный файл GTK+ виджета параметрической кривой.
 *
 * \defgroup GpCifroCurve GpCifroCurve - GTK+ виджет параметрической кривой.
 *
 * Данный иджет является наследуемым от \link GpCifroScope \endlink и потому к
 * нему могут применяться все функции GpCifroScope. Дополнительно к GpCifroScope
 * виджет обеспечивает отображение параметрической кривой с заданием её
 * параметров через контрольные точки. Управление точками описано в разделе
 * \link IcaCurve \endlink.
 *
 * Функции #gp_cifro_curve_clear_points, #gp_cifro_curve_add_point, #gp_cifro_curve_set_points,
 * #gp_cifro_curve_get_points, #gp_cifro_curve_set_curve_color и #gp_cifro_curve_set_point_color
 * аналогичны функциям \link IcaCurve \endlink.
 *
 * Создание объекта производится функцией #gp_cifro_curve_new.
 *
*/

#ifndef _cifro_curve_h
#define _cifro_curve_h

#include <gtk/gtk.h>

#include <gp-cifroscope.h>
#include <gp-icacurve.h>


G_BEGIN_DECLS


/**
* GpCifroCurveGravity:
* @GP_CIFRO_CURVE_GRAVITY_RIGHT_UP:   Ось X - вправо, ось Y - вверх.
* @GP_CIFRO_CURVE_GRAVITY_LEFT_UP:    Ось X - влево,  ось Y - вверх.
* @GP_CIFRO_CURVE_GRAVITY_RIGHT_DOWN: Ось X - вправо, ось Y - вниз.
* @GP_CIFRO_CURVE_GRAVITY_LEFT_DOWN:  Ось X - влево,  ось Y - вниз.
* @GP_CIFRO_CURVE_GRAVITY_UP_RIGHT:   Ось X - вверх,  ось Y - вправо.
* @GP_CIFRO_CURVE_GRAVITY_UP_LEFT:    Ось X - вверх,  ось Y - влево.
* @GP_CIFRO_CURVE_GRAVITY_DOWN_RIGHT: Ось X - вниз,   ось Y - вправо.
* @GP_CIFRO_CURVE_GRAVITY_DOWN_LEFT:  Ось X - вниз,   ось Y - влево.
*
* Типы ориентации осей параметрической кривой.
*/
typedef enum
{
  GP_CIFRO_CURVE_GRAVITY_RIGHT_UP = GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP,
  GP_CIFRO_CURVE_GRAVITY_LEFT_UP = GP_CIFRO_SCOPE_GRAVITY_LEFT_UP,
  GP_CIFRO_CURVE_GRAVITY_RIGHT_DOWN = GP_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN,
  GP_CIFRO_CURVE_GRAVITY_LEFT_DOWN = GP_CIFRO_SCOPE_GRAVITY_LEFT_DOWN,
  GP_CIFRO_CURVE_GRAVITY_UP_RIGHT = GP_CIFRO_SCOPE_GRAVITY_UP_RIGHT,
  GP_CIFRO_CURVE_GRAVITY_UP_LEFT = GP_CIFRO_SCOPE_GRAVITY_UP_LEFT,
  GP_CIFRO_CURVE_GRAVITY_DOWN_RIGHT = GP_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT,
  GP_CIFRO_CURVE_GRAVITY_DOWN_LEFT = GP_CIFRO_SCOPE_GRAVITY_DOWN_LEFT,
} GpCifroCurveGravity;


#define GTK_TYPE_GP_CIFRO_CURVE                gp_cifro_curve_get_type()
#define GP_CIFRO_CURVE( obj )                  ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), GTK_TYPE_GP_CIFRO_CURVE, GpCifroCurve ) )
#define GP_CIFRO_CURVE_CLASS( vtable )         ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), GTK_TYPE_GP_CIFRO_CURVE, GpCifroCurveClass ) )
#define GP_CIFRO_CURVE_GET_CLASS( inst )       ( G_TYPE_INSTANCE_GET_INTERFACE (( inst ), GTK_TYPE_GP_CIFRO_CURVE, GpCifroCurveClass ) )


GType gp_cifro_curve_get_type (void);

typedef struct _GpCifroCurve GpCifroCurve;
typedef struct _GpCifroCurveClass GpCifroCurveClass;

struct _GpCifroCurve
{
  GpCifroScope parent;
};

struct _GpCifroCurveClass
{
  GpCifroScopeClass parent_class;
};


/**
 * gp_cifro_curve_new:
 * @gravity: Ориентация осциллографа (параметр #GpCifroScope).
 * @n_channels: Число каналов осциллографа (параметр #GpCifroScope).
 * @curve_func: (closure curve_data) (scope notified): Функция расчета кривой по заданным точкам.
 * @curve_data: Пользовательские данные для передачи в curve_func.
 *
 * Создание объекта.
 *
 * Returns: Указатель на созданный объект.
*/
GtkWidget *gp_cifro_curve_new (GpCifroCurveGravity gravity, guint n_channels, GpIcaCurveFunc curve_func,
                               gpointer curve_data);


/**
 * gp_cifro_curve_clear_points:
 * @ccurve: Объект #GpCifroCurve.
 *
 * Удалить все контрольные точки.
*/
void gp_cifro_curve_clear_points (GpCifroCurve *ccurve);


/**
 * gp_cifro_curve_add_point:
 * @ccurve: Объект GpCifroCurve.
 * @x: Координата точки x.
 * @y: Координата точки y.
 *
 * Добавить одну контрольную точку.
*/
void gp_cifro_curve_add_point (GpCifroCurve *ccurve, gdouble x, gdouble y);


/**
 * gp_cifro_curve_set_points:
 * @ccurve: Объект GpCifroCurve.
 * @points: (transfer none) (element-type GpIcaCurvePoint): Массив новых точек.
 *
 * Переопределить все точки.
*/
void gp_cifro_curve_set_points (GpCifroCurve *ccurve, GArray *points);


/**
 * gp_cifro_curve_get_points:
 * @ccurve: Указатель на объект GpCifroCurve.
 *
 * Получить массив контрольных точек.
 *
 * Returns: (transfer full) (element-type GpIcaCurvePoint): GArray с контрольными точками.
 */
GArray *gp_cifro_curve_get_points (GpCifroCurve *ccurve);


/**
 * gp_cifro_curve_set_curve_color:
 * @ccurve: Объект GpCifroCurve.
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета кривой.
*/
void gp_cifro_curve_set_curve_color (GpCifroCurve *ccurve, gdouble red, gdouble green, gdouble blue);


/**
 * gp_cifro_curve_set_point_color:
 * @ccurve: Объект GpCifroCurve.
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета контрольных точек.
*/
void gp_cifro_curve_set_point_color (GpCifroCurve *ccurve, gdouble red, gdouble green, gdouble blue);


G_END_DECLS


#endif /* _cifro_curve_h */
