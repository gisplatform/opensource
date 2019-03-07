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
 * \file gp-icascope.h
 *
 * \author Andrei Fadeev
 * \date 2.06.2014
 * \brief Заголовочный файл класса отображения осциллографа.
 *
 * \defgroup GpIcaScope GpIcaScope - класс отображения осциллографа.
 *
 * Класс реализует объект с интерфейсом \link GpIcaRenderer \endlink и
 * позволяет отображать данные в виде электронного осциллографа.
 *
 * Возможно отображение данных по нескольким каналам. Число каналов
 * определяется при создании объекта.
 *
 * Для каждого канала задаётся шаг по оси времени, через который
 * отображаются данные и начальное смещение по времени - #gp_ica_scope_set_channel_time_param.
 * Также задаются смещение и коэффициент масштабирования для значений -
 * #gp_ica_scope_set_channel_value_param.
 *
 * Объект создаётся функцией gp_ica_scope_new и имеет плавающую ссылку. При
 * добавлении в \link CifroArea \endlink владельцем объекта становится CifroArea.
 *
 * Данные для отображения задаются функцией #gp_ica_scope_set_data.
 *
 * После того как данные для всех каналов заданы необходимо дать
 * указание о перерисовке вызовом функции #gp_ica_scope_update.
 *
*/

#ifndef _gp_ica_scope_h
#define _gp_ica_scope_h

#include <gp-icarenderer.h>
#include <gp-icastaterenderer.h>


G_BEGIN_DECLS


/**
* GpIcaScopeDrawType:
* @GP_ICA_SCOPE_LINED: Данные соединяются линиями.
* @GP_ICA_SCOPE_DOTTED: Данные рисуются отдельными точками.
* @GP_ICA_SCOPE_MIX: Данные рисуются и точками и линиями.
*
* Тип изображения осциллографа.
*/
typedef enum
{
  GP_ICA_SCOPE_LINED = 1,
  GP_ICA_SCOPE_DOTTED,
  GP_ICA_SCOPE_MIX
}
GpIcaScopeDrawType;


#define G_TYPE_GP_ICA_SCOPE                    gp_ica_scope_get_type()
#define GP_ICA_SCOPE( obj )                    ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), G_TYPE_GP_ICA_SCOPE, GpIcaScope ) )
#define GP_ICA_SCOPE_CLASS( vtable )           ( G_TYPE_CHECK_CLASS_CAST( ( vtable ), G_TYPE_GP_ICA_SCOPE, GpIcaScopeClass ) )
#define GP_ICA_SCOPE_GET_CLASS( obj )          ( G_TYPE_INSTANCE_GET_INTERFACE( ( obj ), G_TYPE_GP_ICA_SCOPE, GpIcaScopeClass ) )


typedef struct _GpIcaScope GpIcaScope;
typedef struct _GpIcaScopeClass GpIcaScopeClass;

GType gp_ica_scope_get_type( void );


struct _GpIcaScope
{
  GInitiallyUnowned parent;
};

struct _GpIcaScopeClass
{
  GInitiallyUnownedClass parent_class;
};


/**
 * gp_ica_scope_new:
 * @state_renderer: Объект #GpIcaStateRenderer.
 * @n_channels: Число каналов осциллографа для отображения.
 *
 * Создание объекта.
 *
 * Returns: Указатель на объект GpIcaScope.
*/
GpIcaScope *gp_ica_scope_new(GpIcaStateRenderer *state_renderer, guint n_channels);


/**
 * gp_ica_scope_set_axis_name:
 * @scope: Oбъект GpIcaScope.
 * @x_axis_name: Подпись оси абсцисс.
 * @y_axis_name: Подпись оси ординат.
 *
 * Задание имени осей.
*/
void gp_ica_scope_set_axis_name( GpIcaScope *scope, const gchar *x_axis_name, const gchar *y_axis_name );


/**
 * gp_ica_scope_set_show_info:
 * @scope: Объект GpIcaScope.
 * @show: Показывать - TRUE или нет - FALSE информацию о значениях под курсором.
 *
 * Включение и выключение отображения информации о значениях под курсором.
*/
void gp_ica_scope_set_show_info( GpIcaScope *scope, gboolean show );


/**
 * gp_ica_scope_set_channel_time_param:
 * @scope: Объект GpIcaScope.
 * @channel: Номер канала для задания параметров.
 * @time_shift: Смещение начала оси времени.
 * @time_step: Шаг по оси времени.
 *
 * Задание параметров оси времени для канала.
*/
void gp_ica_scope_set_channel_time_param( GpIcaScope *scope, gint channel, gfloat time_shift, gfloat time_step );


/**
 * gp_ica_scope_set_channel_value_param:
 * @scope: Объект GpIcaScope.
 * @channel: Номер канала для задания параметров.
 * @value_shift: Смещение данных по оси ординат.
 * @value_scale: Коэффициент масштабирования данных.
 *
 * Задание параметров оси данных для канала.
*/
void gp_ica_scope_set_channel_value_param( GpIcaScope *scope, gint channel, gfloat value_shift, gfloat value_scale );


/**
 * gp_ica_scope_set_channel_axis_name:
 * @scope: Объект GpIcaScope.
 * @channel: Номер канала для задания параметров.
 * @axis_name: Имя оси абсцисс для канала.
 *
 * Задание имени оси абсцисс для канала.
*/
void gp_ica_scope_set_channel_axis_name( GpIcaScope *scope, gint channel, const gchar *axis_name );


/**
 * gp_ica_scope_set_channel_draw_type:
 * @scope: Объект GpIcaScope.
 * @channel: Номер канала для задания параметров.
 * @draw_type: Тип отображения осциллограмм.
 *
 * Задание типа отображения осциллограмм.
*/
void gp_ica_scope_set_channel_draw_type( GpIcaScope *scope, gint channel, GpIcaScopeDrawType draw_type );


/**
 * gp_ica_scope_set_channel_color:
 * @scope: Объект GpIcaScope.
 * @channel: Номер канала осциллографа.
 * @red: Значение красной составляющей цвета от 0 до 1.
 * @green: Значение зелёной составляющей цвета от 0 до 1.
 * @blue: Значение синей составляющей цвета от 0 до 1.
 *
 * Задание цвета которым отображаются данные.
*/
void gp_ica_scope_set_channel_color( GpIcaScope *scope, gint channel, gdouble red, gdouble green, gdouble blue );


/**
 * gp_ica_scope_set_show_channel:
 * @scope: Объект GpIcaScope.
 * @channel: Номер канала осциллографа.
 * @show: Показывать - TRUE или нет - FALSE данные этого канала.
 *
 * Включение и выключение отображения данных канала.
*/
void gp_ica_scope_set_show_channel( GpIcaScope *scope, gint channel, gboolean show );


/**
 * gp_ica_scope_set_data:
 * @scope: Объект GpIcaScope.
 * @channel: Номер канала для отображения данных.
 * @num: Число данных для отображения.
 * @values: (transfer none)(array length=num): Массив данных для отображения.
 *
 * Задание данных для отображения.
*/
void gp_ica_scope_set_data( GpIcaScope *scope, guint channel, guint num, gfloat *values );


/**
 * gp_ica_scope_update:
 * @scope: Объект GpIcaScope.
 *
 * Перерисовка данных осциллографа.
 *
 * Определяет необходимость перерисовать данные после
 * того как были установлены новые значения для каналов
 * осциллографа.
*/
void gp_ica_scope_update( GpIcaScope *scope );


/**
 * gp_ica_scope_configure:
 * @scope: Объект GpIcaScope.
 *
 * Переконфигурация при изменении параметров отображения графической системы,
 * например, при изменении шрифта по умолчанию.
*/
void gp_ica_scope_configure( GpIcaScope *scope );


G_END_DECLS

#endif // _gp_ica_scope_h
