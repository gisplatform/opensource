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
 * \file gp-cifrocustom.h
 *
 * \author Andrei Vodilov
 * \date 01.02.2016
 *
 * \brief Заголовочный файл GTK+ виджета для отображения пользовательской информации (точки, метки, линии).
 *
 * \defgroup GpCifroCustom GpCifroCustom - GTK+ для отображения пользовательской информации (точки, метки, линии).
 *
 * Данный иджет является наследуемым от \link GpCifroScope \endlink и потому к
 * нему могут применяться все функции GpCifroScope. Дополнительно к GpCifroScope
 * виджет обеспечивает отображение параметрической кривой с заданием её
 * параметров через контрольные точки. Управление точками описано в разделе
 * \link IcaPoint \endlink.
 *
 * Функции #gp_cifro_custom_clear_points, #gp_cifro_custom_add_point, #gp_cifro_custom_set_points,
 * #gp_cifro_custom_get_points, #gp_cifro_custom_set_line_color и #gp_cifro_custom_set_points_color
 * аналогичны функциям \link IcaPoint \endlink.
 *
 * Создание объекта производится функцией #gp_cifro_custom_new.
 *
*/

#ifndef _cifro_custom_h
#define _cifro_custom_h

#include <gtk/gtk.h>

#include <gp-cifroscope.h>
#include <gp-icacustom.h>


G_BEGIN_DECLS


/**
* GpCifroCustomGravity:
* @GP_CIFRO_CUSTOM_GRAVITY_RIGHT_UP:  Ось X - вправо, ось Y - вверх.
* @GP_CIFRO_CUSTOM_GRAVITY_LEFT_UP:   Ось X - влево,  ось Y - вверх.
* @GP_CIFRO_CUSTOM_GRAVITY_RIGHT_DOWN:Ось X - вправо, ось Y - вниз.
* @GP_CIFRO_CUSTOM_GRAVITY_LEFT_DOWN: Ось X - влево,  ось Y - вниз.
* @GP_CIFRO_CUSTOM_GRAVITY_UP_RIGHT:  Ось X - вверх,  ось Y - вправо.
* @GP_CIFRO_CUSTOM_GRAVITY_UP_LEFT:   Ось X - вверх,  ось Y - влево.
* @GP_CIFRO_CUSTOM_GRAVITY_DOWN_RIGHT:Ось X - вниз,   ось Y - вправо.
* @GP_CIFRO_CUSTOM_GRAVITY_DOWN_LEFT: Ось X - вниз,   ось Y - влево.
*
* Типы ориентации осей параметрической кривой.
*/
typedef enum
{
  GP_CIFRO_CUSTOM_GRAVITY_RIGHT_UP = GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP,
  GP_CIFRO_CUSTOM_GRAVITY_LEFT_UP = GP_CIFRO_SCOPE_GRAVITY_LEFT_UP,
  GP_CIFRO_CUSTOM_GRAVITY_RIGHT_DOWN = GP_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN,
  GP_CIFRO_CUSTOM_GRAVITY_LEFT_DOWN = GP_CIFRO_SCOPE_GRAVITY_LEFT_DOWN,
  GP_CIFRO_CUSTOM_GRAVITY_UP_RIGHT = GP_CIFRO_SCOPE_GRAVITY_UP_RIGHT,
  GP_CIFRO_CUSTOM_GRAVITY_UP_LEFT = GP_CIFRO_SCOPE_GRAVITY_UP_LEFT,
  GP_CIFRO_CUSTOM_GRAVITY_DOWN_RIGHT = GP_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT,
  GP_CIFRO_CUSTOM_GRAVITY_DOWN_LEFT = GP_CIFRO_SCOPE_GRAVITY_DOWN_LEFT,
}
GpCifroCustomGravity;


#define GTK_TYPE_GP_CIFRO_CUSTOM                gp_cifro_custom_get_type()
#define GP_CIFRO_CUSTOM( obj )                  ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), GTK_TYPE_GP_CIFRO_CUSTOM, GpCifroCustom ) )
#define GP_CIFRO_CUSTOM_CLASS( vtable )         ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), GTK_TYPE_GP_CIFRO_CUSTOM, GpCifroCustomClass ) )
#define GP_CIFRO_CUSTOM_GET_CLASS( inst )       ( G_TYPE_INSTANCE_GET_INTERFACE (( inst ), GTK_TYPE_GP_CIFRO_CUSTOM, GpCifroCustomClass ) )


GType gp_cifro_custom_get_type (void);

typedef struct _GpCifroCustom GpCifroCustom;
typedef struct _GpCifroCustomClass GpCifroCustomClass;

struct _GpCifroCustom
{
  GpCifroScope parent;
};

struct _GpCifroCustomClass
{
  GpCifroScopeClass parent_class;
};


/**
 * gp_cifro_custom_new:
 * @gravity: Ориентация осциллографа (параметр #GpCifroScope).
 * @n_channels: Число каналов осциллографа (параметр #GpCifroScope).
 * @point_data: Пользовательские данные.
 *
 * Создание объекта.
 *
 * Returns: Указатель на созданный объект.
*/
GtkWidget *gp_cifro_custom_new (GpCifroCustomGravity gravity, guint n_channels, gpointer point_data);


/**
 * gp_cifro_custom_clear_points:
 * @cpoint: Объект GpCifroCustom.
 *
 * Удалить все контрольные точки.
*/
void gp_cifro_custom_clear_points (GpCifroCustom *cpoint);


/**
 * gp_cifro_custom_add_point:
 * @cpoint: Объект GpCifroCustom.
 * @x: Координата точки x.
 * @y: Координата точки y.
 *
 * Добавить одну контрольную точку.
*/
void gp_cifro_custom_add_point (GpCifroCustom *cpoint, gdouble x, gdouble y);


/**
 * gp_cifro_custom_set_points:
 * @cpoint: Объект GpCifroCustom;
 * @points: (transfer none) (element-type GpIcaCustomPoints): Массив новых точек.
 *
 * Переопределить все точки.
*/
void gp_cifro_custom_set_points (GpCifroCustom *cpoint, GArray *points);


/**
 * gp_cifro_custom_get_points:
 * @cpoint: Объект GpCifroCustom.
 *
 * Получить массив контрольных точек.
 *
 * Returns: (transfer full) (element-type GpIcaCustomPoints): GArray с контрольными точками.
 */
GArray *gp_cifro_custom_get_points (GpCifroCustom *cpoint);


/**
 * gp_cifro_custom_set_line_color:
 * @cpoint: Объект GpCifroCustom;
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета кривой.
*/
void gp_cifro_custom_set_line_color (GpCifroCustom *cpoint, gdouble red, gdouble green, gdouble blue);


/**
 * gp_cifro_custom_set_points_color:
 * @cpoint: Объект GpCifroCustom;
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета контрольных точек.
 *
*/
void gp_cifro_custom_set_points_color (GpCifroCustom *cpoint, gdouble red, gdouble green, gdouble blue);


void gp_cifro_custom_set_draw_type(GpCifroCustom *self, GpIcaCustomDrawType new_type);

GpIcaCustomDrawType gp_cifro_custom_get_draw_type(GpCifroCustom *self);

G_END_DECLS


#endif /* _cifro_custom_h */
