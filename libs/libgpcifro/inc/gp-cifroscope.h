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
 * \file gp-cifroscope.h
 *
 * \author Andrei Fadeev
 * \date 16.07.2014
 *
 * \brief Заголовочный файл GTK+ виджета осциллограф.
 *
 * \defgroup GpCifroScope GpCifroScope - GTK+ виджет осциллограф.
 *
 * GpCifroScope позволяет реализовывать графическое представление данных как
 * в осциллографе. Имеется возможность изменять тип отображения: линиями или точками -
 * #gp_cifro_scope_set_channel_draw_type. При этом возможно отображение данных по
 * нескольким "каналам". Число каналов определяется при создании объекта
 * функцией #gp_cifro_scope_new.
 *
 * Для каждого канала возможно задание индивидуальных параметров "усиления" и
 * смещения сигнала - #gp_cifro_scope_set_channel_time_param, а также параметров его
 * оцифровки - #gp_cifro_scope_set_channel_value_param. Цвет осциллограммы канала
 * задаётся функцией #gp_cifro_scope_set_channel_color, включение и выключение канала
 * для отображения управляется функцией #gp_cifro_scope_set_show_channel.
 * Для каналов может быть задано отличное от основного имя. Это имя будет отображаться
 * в информационной области как подпись к значению. Имя задаётся функцией
 * #gp_cifro_scope_set_channel_axis_name.
 *
 * Пределы в которых осциллограф будет отображать исследуемые значения и подписи
 * к осям задаются функцией #gp_cifro_scope_set_view. Границы масштабов и текущие
 * границы отображения задаются функциями #gp_cifro_scope_set_zoom_scale и
 * #gp_cifro_scope_set_shown соответственно.
 *
 * Данные для отображения задаются функцией #gp_cifro_scope_set_data. После того как
 * данные для всех каналов определены необходимо вызвать функцию #gp_cifro_scope_update
 * для обновления изображения.
 *
 * Виджет использует цвета текущей темы для отображения. Однако пользователь может
 * задать свои цвета. Для этого необходимо установить соответствующий параметр. Цвета
 * задаются как 32-х битное целое цисло: AARRGGBB, где:
 * - AA - значение прозрачности от 0 - FF;
 * - RR - значение красной компоненты цвета от 0 - FF;
 * - GG - значение зелёной компоненты цвета от 0 - FF;
 * - BB - значение синей компоненты цвета от 0 - FF.
 *
 * Для изменения цветов доступны следующие параметры:
 * - ground-color - цвет фона;
 * - border-color - цвет элементов обрамления;
 * - axis-color - цвет осей;
 * - zero-axis-color - цвет оси проходящей через 0;
 * - text-color - цвет текстовых подписей.
 *
 * Если пользователь переопределил один из цветов, цвета из темы использоваться не будут.
 *
*/

#ifndef _cifro_scope_h
#define _cifro_scope_h

#include <gtk/gtk.h>

#include <gp-cifroarea.h>
#include <gp-icastaterenderer.h>


G_BEGIN_DECLS


/**
* GpCifroScopeGravity:
* @GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP:   Ось X - вправо, ось Y - вверх.
* @GP_CIFRO_SCOPE_GRAVITY_LEFT_UP:    Ось X - влево,  ось Y - вверх.
* @GP_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN: Ось X - вправо, ось Y - вниз.
* @GP_CIFRO_SCOPE_GRAVITY_LEFT_DOWN:  Ось X - влево,  ось Y - вниз.
* @GP_CIFRO_SCOPE_GRAVITY_UP_RIGHT:   Ось X - вверх,  ось Y - вправо.
* @GP_CIFRO_SCOPE_GRAVITY_UP_LEFT:    Ось X - вверх,  ось Y - влево.
* @GP_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT: Ось X - вниз,   ось Y - вправо.
* @GP_CIFRO_SCOPE_GRAVITY_DOWN_LEFT:  Ось X - вниз,   ось Y - влево.
*
* Типы ориентации осей осциллографа.
*/
typedef enum
{
  GP_CIFRO_SCOPE_GRAVITY_RIGHT_UP = 1,
  GP_CIFRO_SCOPE_GRAVITY_LEFT_UP,
  GP_CIFRO_SCOPE_GRAVITY_RIGHT_DOWN,
  GP_CIFRO_SCOPE_GRAVITY_LEFT_DOWN,
  GP_CIFRO_SCOPE_GRAVITY_UP_RIGHT,
  GP_CIFRO_SCOPE_GRAVITY_UP_LEFT,
  GP_CIFRO_SCOPE_GRAVITY_DOWN_RIGHT,
  GP_CIFRO_SCOPE_GRAVITY_DOWN_LEFT
}
GpCifroScopeGravity;


/**
* GpCifroScopeDrawType:
* @GP_CIFRO_SCOPE_LINED: Данные соединяются линиями.
* @GP_CIFRO_SCOPE_DOTTED: Данные рисуются отдельными точками.
* @GP_CIFRO_SCOPE_MIX: Данные рисуются и точками и линиями.
*
* Тип изображения осциллографа.
*/
typedef enum
{
  GP_CIFRO_SCOPE_LINED = 1,
  GP_CIFRO_SCOPE_DOTTED,
  GP_CIFRO_SCOPE_MIX
}
GpCifroScopeDrawType;


#define GTK_TYPE_GP_CIFRO_SCOPE                gp_cifro_scope_get_type()
#define GP_CIFRO_SCOPE( obj )                  ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), GTK_TYPE_GP_CIFRO_SCOPE, GpCifroScope ) )
#define GP_CIFRO_SCOPE_CLASS( vtable )         ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), GTK_TYPE_GP_CIFRO_SCOPE, GpCifroScopeClass ) )
#define GP_CIFRO_SCOPE_GET_CLASS( inst )       ( G_TYPE_INSTANCE_GET_INTERFACE (( inst ), GTK_TYPE_GP_CIFRO_SCOPE, GpCifroScopeClass ) )


GType gp_cifro_scope_get_type (void);

typedef struct _GpCifroScope GpCifroScope;
typedef struct _GpCifroScopeClass GpCifroScopeClass;

struct _GpCifroScope
{
  GtkBox parent;
};

struct _GpCifroScopeClass
{
  GtkBoxClass parent_class;
};


/**
 * gp_cifro_scope_new:
 * @gravity: Ориентация осциллографа;
 * @n_channels: Число каналов осциллографа.
 *
 * Создание объекта GpCifroScope. Данная функция создает GTK Widget.
 *
 * Returns: Указатель на созданый объект.
 */
GtkWidget *gp_cifro_scope_new (GpCifroScopeGravity gravity, guint n_channels);


/**
 * gp_cifro_scope_set_view:
 * @cscope: Объект GpCifroScope.
 * @time_axis_name: Подпись оси времени (абсцисса).
 * @time_min: Минимальное время отображения данных.
 * @time_max: Максимальное время отображения данных.
 * @value_axis_name: Подпись оси данных (ордината).
 * @value_min: Минимальное значение данных для отображения.
 * @value_max: Максимальное значение данных для отображения.
 *
 * Задание границ отображаемых значений.
 *
 * Функция задаёт границы в пределах которых осциллограф будет отображать данные.
 * Данные которые находятся вне этих пределов отображаться не будут.
 * Также эта функция задаёт подписи к осям абсцисс и ординат.
*/
void gp_cifro_scope_set_view (GpCifroScope *cscope,
                              const gchar *time_axis_name, gdouble time_min, gdouble time_max,
                              const gchar *value_axis_name, gdouble value_min, gdouble value_max);


/**
 * gp_cifro_scope_set_show_info:
 * @cscope: Указатель на объект GpCifroScope;
 * @show: Показывать - TRUE или нет - FALSE информацию о значениях под курсором.
 *
 * Включение и выключение отображения информации о значениях под курсором.
*/
void gp_cifro_scope_set_show_info (GpCifroScope *cscope, gboolean show);


/**
 * gp_cifro_scope_set_channel_time_param:
 * @cscope: Объект GpCifroScope.
 * @channel: Номер канала осциллографа.
 * @time_shift: Начальный момент времени.
 * @time_step: Шаг смещения по оси времени.
 *
 * Задание параметров оси времени.
 *
 * Функция определяет с какого момента времени следует отображать данные и
 * какой шаг между двумя соседними данными (частота оцифровки). Параметры задаются
 * индивидуально для каждого канала. Если номер канала отрицательный параметры
 * устанавливаются для всех каналов.
*/
void gp_cifro_scope_set_channel_time_param (GpCifroScope *cscope, gint channel, gfloat time_shift, gfloat time_step);


/**
 * gp_cifro_scope_set_channel_value_param:
 * @cscope: Объект GpCifroScope.
 * @channel: Номер канала осциллографа.
 * @value_shift: Коэффициент смещения данных.
 * @value_scale: Коэффициент умножения данных.
 *
 * Задание параметров оси данных.
 *
 * Функция задаёт коэффициенты на которые умножаются и сдвигаются все данные в
 * канале. Это позволяет отображать разнородные данные в одном пространстве и наглядно
 * сравнивать их друг с другом. Коэффициенты задаются индивидуально для каждого канала.
 * Если номер канала отрицательный коэффициенты устанавливаются для всех каналов.
*/
void gp_cifro_scope_set_channel_value_param (GpCifroScope *cscope, gint channel, gfloat value_shift, gfloat value_scale);


/**
 * gp_cifro_scope_set_channel_draw_type:
 * @cscope: Объект GpCifroScope.
 * @channel: Номер канала осциллографа.
 * @draw_type: Тип отображения осциллограмм.
 *
 * Задание типа отображения осциллограмм.
 *
 * Если номер канала отрицательный тип отображения устанавливается для всех каналов.
*/
void gp_cifro_scope_set_channel_draw_type (GpCifroScope *cscope, gint channel, GpCifroScopeDrawType draw_type);


/**
 * gp_cifro_scope_set_channel_color:
 * @cscope: Объект GpCifroScope.
 * @channel: Номер канала осциллографа.
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета которым отображаются данные.
 *
 * Если номер канала отрицательный цвет устанавливается для всех каналов.
*/
void gp_cifro_scope_set_channel_color (GpCifroScope *cscope, gint channel, gdouble red, gdouble green, gdouble blue);


/**
 * gp_cifro_scope_set_show_channel:
 * @cscope: Объект GpCifroScope.
 * @channel: Номер канала осциллографа;
 * @show: Показывать - TRUE или нет - FALSE данные этого канала.
 *
 * Включение и выключение отображения данных канала.
 *
 * Если номер канала отрицательный отображение устанавливается для всех каналов.
*/
void gp_cifro_scope_set_show_channel (GpCifroScope *cscope, gint channel, gboolean show);


/**
 * gp_cifro_scope_set_channel_axis_name:
 * @scope: Объект IcaScope.
 * @channel: Номер канала для задания параметров.
 * @axis_name: Имя оси абсцисс для канала.
 *
 * Задание имени оси абсцисс для канала.
 *
 * Если номер канала отрицательный имя устанавливается для всех каналов.
*/
void gp_cifro_scope_set_channel_axis_name (GpCifroScope *scope, gint channel, const gchar *axis_name);


/**
 * gp_cifro_scope_set_zoom_scale:
 * @cscope: Объект GpCifroScope.
 * @min_scale_time: Минимально возможный коэффициент масштаба по оси времени (приближение).
 * @max_scale_time: Максимально возможный коэффициент масштаба по оси времени (отдаление).
 * @min_scale_value: Минимально возможный коэффициент масштаба по оси значений (приближение).
 * @max_scale_value: Максимально возможный коэффициент масштаба по оси значений (отдаление).
 *
 * Задание границ изменения масштабов.
 *
 * Returns: TRUE если границы изменения масштабов установлены, FALSE в случае ошибки.
*/
gboolean gp_cifro_scope_set_zoom_scale (GpCifroScope *cscope, gdouble min_scale_time, gdouble max_scale_time,
                                        gdouble min_scale_value, gdouble max_scale_value);


/**
 * gp_cifro_scope_set_shown:
 * @cscope: Объект GpCifroScope.
 * @from_time: Начало границы осциллограммы по оси времени.
 * @to_time: Окончание границы осциллограммы по оси времени.
 * @from_value: Начало границы осциллограммы по оси значений с низу.
 * @to_value: Окончание границы осциллограммы по оси с значений верху.
 *
 * Задание границы текущей видимости осциллограммы.
 *
 * Returns: TRUE если границы текущей видимости осциллограммы установлены, FALSE в случае ошибки.
*/
gboolean gp_cifro_scope_set_shown (GpCifroScope *cscope, gdouble from_time, gdouble to_time, gdouble from_value,
                                   gdouble to_value);


/**
 * gp_cifro_scope_set_data:
 * @cscope: Объект GpCifroScope.
 * @channel: Номер канала осциллографа.
 * @num: Число значений для отображения.
 * @values: (transfer none)(array length=num): Массив данных для отображения.
 *
 * Задание данных для отображения.
*/
void gp_cifro_scope_set_data (GpCifroScope *cscope, gint channel, gint num, gfloat *values);


/**
 * gp_cifro_scope_update:
 * @cscope: Объект GpCifroScope.
 *
 * Обновление изображения осциллографа.
*/
void gp_cifro_scope_update (GpCifroScope *cscope);


/**
 * gp_cifro_scope_get_cifro_area:
 * @cscope: Объект GpCifroScope.
 *
 * Получение указателя на объект #GpCifroArea.
 *
 * Функция возвращает указатель на объект #GpCifroArea, используемый для
 * отображения осциллографа. Это даёт возможность формировать дополнительное изображение
 * через собственный #IcaRenderer.
 *
 * Returns: (transfer none): Указатель на объект #GpCifroArea.
*/
GpCifroArea *gp_cifro_scope_get_cifro_area (GpCifroScope *cscope);


/**
 * gp_cifro_scope_get_state_renderer:
 * @cscope: Объект GpCifroScope.
 *
 * Получение указателя на объект #IcaStateRenderer.
 *
 * Функция возвращает указатель на объект #IcaStateRenderer, используемый
 * в #GpCifroArea для отображения осциллографа.
 *
 * Returns: (transfer none): Указатель на объект #IcaStateRenderer.
*/
GpIcaStateRenderer *gp_cifro_scope_get_state_renderer (GpCifroScope *cscope);


G_END_DECLS


#endif /* _cifro_scope_h */
