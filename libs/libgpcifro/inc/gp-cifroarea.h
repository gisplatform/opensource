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
 * \file gp-cifroarea.h
 *
 * \author Andrei Fadeev
 * \date 5.04.2013
 * \brief Заголовочный файл GTK+ виджета показа изображений сформированных интерфейсом IcaRenderer.
 *
 * \defgroup GpCifroArea GpCifroArea - GTK+ виджет показа изображений сформированных интерфейсом IcaRenderer.
 *
 * Виджет предназначен для показа изображений сформированных внешними модулями с интерфейсом \link IcaRenderer \endlink.
 * Графическая структура изображения описана в разделе \link IcaRenderer \endlink.
 *
 * Виджет реализует следующие возможности:
 * - сведение нескольких изображений в одно и отображение его на экране;
 * - зеркальное отражение изображений типа #ICA_RENDERER_VISIBLE по вертикальной и(или) горизонтальной осям;
 * - поворот изображений типа #ICA_RENDERER_VISIBLE на определенный пользователем угол;
 * - масштабирование изображений, включая совместное и раздельное по обоим осям, а также с фиксированными коэффициентами;
 * - управление отображением с использованием клавиаутры и мышки.
 *
 * Находясь в фокусе виджет воспринимает следующие команды управления от клавиатуры и мышки:
 * - перемещение изображения (в пределах возможных отображаемых значений) клавишами верх, низ, право, лево;
 * - перемещение изображения (в пределах возможных отображаемых значений) перемещением указателя мышки при
 *   нажатой левой клавише;
 * - поворот изображения клавишами право, лево и нажатой клавише shift;
 * - масштабирование изображения клавишами "+" - плюс, "-" - минус, при этом если дополнительно будет нажата
 *   клавиша Ctrl - масштабирование будет производиться только по вертикальной оси, Alt - масштабирование
 *   будет производиться по горизонтальной оси;
 * - масштабирование изображения вращением колесика мышки, с нажатой клавишей Shift - по обеим осям,
 *   Ctrl - по вертикальной оси, Alt - по горизонтальной оси.
 *
 * Создание виджета производится функцией #gp_cifro_area_new, в качестве параметра которой передается интервал
 * в милисекундах через который произодится запрос обновления изображений.
 *
 * После создания виджета необходимо зарегистрировать объекты с интерфейсом \link IcaRenderer \endlink формирующие изображения.
 * Финальное изображение формируется последовательным (по очереди регистрации модулей формирования) наложением
 * изображений друг на друга с учетом прозрачности. Регистрация модулей осуществляется функцией #gp_cifro_area_add_layer.
 *
 * При нахождении виджета в фокусе вокруг него может рисоваться рамка с цветом определяемым функцией
 * #gp_cifro_area_set_focus_color, текущее значение цвета рамки можно узнать функцией #gp_cifro_area_get_focus_color.
 *
 * При нахождении курсора мышки в видимой области его изображение меняется на GdkCursor типа GDK_CROSSHAIR,
 * а при перемещении видимой области на GDK_FLEUR. Это поведение можно изменить функциями #gp_cifro_area_set_point_cursor
 * и #gp_cifro_area_set_move_cursor. При удалении объекта все используемые курсоры будут удалены, соответственно
 * можно просто использовать функции gdk_cursor_new_* без обработки ссылок на курсор. Если в качестве курсора
 * передать NULL будет использовать курсор оконной системы по умолчанию.
 *
 * При изменении размеров окна возможна ситуация, когда текущие его размеры не позволят полноценно сформировать
 * финальное изображение. Функция #gp_cifro_area_set_minimum_visible задает минимальные ширину и высоту видимой
 * области виджета при которых изображение будет показыватся на экране. Функция #gp_cifro_area_get_minimum_visible
 * возвращает их текущие значения.
 *
 * Размер окантовки устанавливается функцией #gp_cifro_area_set_border, текущее его значение возвращает
 * функция #gp_cifro_area_get_border.
 *
 * Зеркальное отражение изображений по осям задается функцией #gp_cifro_area_set_swap, его текущее состояние
 * можно определить функцией #gp_cifro_area_get_swap.
 *
 * Перемещение изображения клавишами верх, низ, право, лево происходит на один экранный пиксель. Если при
 * этом одновременно удерживать нажатой клавишу Ctrl, перемещение происходит на число пикселей указанное
 * функцией #gp_cifro_area_set_move_multiplier. Текущая величина смещения может быть получена
 * функцией #gp_cifro_area_get_move_multiplier.
 *
 * Пределы возможных отображаемых значений до которых можно производить перемещение изображения задаются
 * функцией #gp_cifro_area_set_shown_limits, текущие пределы можно узнать функцией #gp_cifro_area_get_shown_limits.
 *
 * Поворот изображения может быть разрешен или отменен функцией #gp_cifro_area_set_rotation в любой момент
 * времени. Однако запрещение поворота не влияет на уже установленный угол поворота. Если пользователь хочет
 * вернуть прежнее состояние он должен самостоятельно установить угол поворота до запрещения поворота.
 * Текущее состояние разрешения поворота можно получить функцией #gp_cifro_area_get_rotation.
 *
 * Поворот изображения клавишами право, лево происходит на 1 градус, если при этом одновременно удерживать
 * нажатой клавишу Ctrl, поворот происходит на величину угла указанную функцией #gp_cifro_area_set_rotate_multiplier.
 * Текущая величина изменения угла поворота может быть получена функцией #gp_cifro_area_get_rotate_multiplier.
 *
 * При изменении размеров окна может происходить изменение коэффициента масштабирования или изменение
 * видимой области. Поведение виджета в этом случае задается функцией #gp_cifro_area_set_scale_on_resize.
 * Парная ей функция #gp_cifro_area_get_scale_on_resize возвращает текущее заданное поведение виджета.
 * Изменение коэффициента масштабирования производится только при произвольном изменении масштабов
 * без сохранения пропорций.
 *
 * Масштабирование, вращением колесика мышки, может осуществляться относительно центра изображения или
 * относительно текущего положения курсора, при этом точка изображения под курсором всегда остается на
 * своем месте. Поведение виджета в этом случае задается функцией #gp_cifro_area_set_zoom_on_center.
 * Парная ей функция #gp_cifro_area_get_zoom_on_center возвращает текущее заданное поведение виджета.
 *
 * Масштабирование изображения производится на величину, определенную функцией #gp_cifro_area_set_zoom_scale.
 * Величина изменения масштаба задается в процентах. Текущая величина изменения масштаба может быть
 * получена функцией #gp_cifro_area_get_zoom_scale.
 *
 * В случае произвольного изменения масштабов можно поддерживать заданную пропорцию между масштабом
 * по вертикальной и горизонтальной оси. Величина этой пропорции задается функцией #gp_cifro_area_set_scale_aspect.
 * Задаваемая величина должна иметь значение большее нуля, в противном случае поддержание пропорции
 * в масштабах отменяется. Текущая заданная величина пропорции между масштабами может быть получена
 * функцией #gp_cifro_area_get_scale_aspect.
 *
 * Границы изменения масштабов определяются функцией #gp_cifro_area_set_scale_limits, текущие границы
 * можно узнать функцией #gp_cifro_area_get_scale_limits.
 *
 * Помимо произвольного изменения масштаба можно определить фиксированный набор масштабов, которые будут
 * применяться последовательно при выполнении соответствующей операции. Набор масштабов можно задать
 * или получить его текущее значение функциями #gp_cifro_area_set_fixed_zoom_scales и
 * #gp_cifro_area_get_fixed_zoom_scales соответственно. Если набор фиксированных масштабов определен,
 * будут использоваться только они. Для отмены этого поведения необходимо зарегистрировать пустой набор.
 *
 * Текущие границы изображения задаются функцией #gp_cifro_area_set_shown, возвращаются #gp_cifro_area_get_shown.
 *
 * Угол поворота изображения задается функцией #gp_cifro_area_set_angle, возвращается gp_cifro_area_get_angle.
 *
 * Функции #gp_cifro_area_move, #gp_cifro_area_rotate, #gp_cifro_area_zoom, #gp_cifro_area_fixed_zoom используются для
 * программного управления перемещением, поворотом и масштабированием изображения соответственно.
 *
*/

#ifndef _cifro_area_h
#define _cifro_area_h

#include <gtk/gtk.h>
#include <gp-icarenderer.h>


G_BEGIN_DECLS


#define GTK_TYPE_GP_CIFRO_AREA                 gp_cifro_area_get_type()
#define GP_CIFRO_AREA( obj )                   ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), GTK_TYPE_GP_CIFRO_AREA, GpCifroArea ) )
#define GP_CIFRO_AREA_CLASS( vtable )          ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), GTK_TYPE_GP_CIFRO_AREA, GpCifroAreaClass ) )
#define GP_CIFRO_AREA_GET_CLASS( inst )        ( G_TYPE_INSTANCE_GET_INTERFACE ( ( inst ), GTK_TYPE_GP_CIFRO_AREA, GpCifroAreaClass ) )


GType gp_cifro_area_get_type (void);

typedef struct _GpCifroArea GpCifroArea;
typedef struct _GpCifroAreaClass GpCifroAreaClass;

struct _GpCifroArea
{
  GtkEventBox parent;
};

struct _GpCifroAreaClass
{
  GtkEventBoxClass parent_class;
};


/**
 * gp_cifro_area_new:
 * @interval: Интервал опроса необходимости обновления изображений, мс.
 *
 * Создание объекта #GpCifroArea. Данная функция создает GTK Widget.
 *
 * Returns: (type GpCifroArea): Указатель на созданный объект #GtkWidget.
 *
 */
GtkWidget *gp_cifro_area_new (gint interval);


/**
 * gp_cifro_area_add_layer:
 * @carea: Указатель на объект #GpCifroArea.
 * @layer_renderer: Указатель на модуль формирования изображений с интерфейсом #IcaRenderer.
 *
 * Регистрация модуля формирования изображения.
 *
 * Регистрирует в объекте #GpCifroArea модуль формирования изображения. После регистрации
 * объект #GpCifroArea становится владельцем модуля (если модуль создан с типом G_TYPE_INITIALLY_UNOWNED),
 * таким образом при удалении объекта GpCifroArea не требуется удалять модули формирования
 * изображений.
 *
 * Returns: Порядковый номер добавленного слоя, если модуль был успешно зарегистрирован, иначе -1.
 *
 */
gint gp_cifro_area_add_layer (GpCifroArea *carea, GpIcaRenderer *layer_renderer);



/**
 * gp_cifro_area_draw_to_surface:
 * @carea: Указатель на объект #GpCifroArea.
 *
 * Рисует содержимое #GpCifroArea в объект cairo_surface_t формата CAIRO_FORMAT_ARGB32.
 *
 * Returns: (transfer full): Объект cairo_surface_t.
 */
cairo_surface_t* gp_cifro_area_draw_to_surface(GpCifroArea *carea);


/**
 * gp_cifro_area_set_draw_focus:
 * @carea: Указатель на объект #GpCifroArea.
 * @draw_focus: Рисовать - TRUE или нет - FALSE рамку при нахождении в фокусе.
 *
 * Задание необходимости рисования рамки при нахождении объекта в фокусе.
 *
 */
void gp_cifro_area_set_draw_focus (GpCifroArea *carea, gboolean draw_focus);


/**
 * gp_cifro_area_get_draw_focus:
 * @carea: Указатель на объект #GpCifroArea.
 *
 * Определение текущих параметров рисования рамки при нахождении объекта в фокусе.
 *
 * Returns: Рисовать - TRUE или нет - FALSE рамку при нахождении в фокусе.
 *
 */
gboolean gp_cifro_area_get_draw_focus (GpCifroArea *carea);


/**
 * gp_cifro_area_set_focus_color:
 * @carea: Указатель на объект #GpCifroArea.
 * @red: Значение красной компоненты цвета рамки.
 * @green: Значение зеленой компоненты цвета рамки.
 * @blue: Значение синей компоненты цвета рамки.
 * @alpha: Значение прозрачности цвета рамки.
 *
 * Задание цвета рамки при нахождении объекта в фокусе.
 *
 */
void gp_cifro_area_set_focus_color (GpCifroArea *carea, gdouble red, gdouble green, gdouble blue, gdouble alpha);


/**
 * gp_cifro_area_get_focus_color:
 * @carea: Указатель на объект #GpCifroArea.
 * @red: (out): Значение красной компоненты цвета рамки или NULL.
 * @green: (out): Значение зеленой компоненты цвета рамки или NULL.
 * @blue: (out): Значение синей компоненты цвета рамки или NULL.
 * @alpha: (out): Значение прозрачности цвета рамки или NULL.
 *
 * Определение текущего цвета рамки при нахождении объекта в фокусе.
 *
 */
void gp_cifro_area_get_focus_color (GpCifroArea *carea, gdouble *red, gdouble *green, gdouble *blue, gdouble *alpha);


/**
 * gp_cifro_area_set_point_cursor:
 * @carea: Указатель на объект #GpCifroArea.
 * @cursor: Указатель на #GdkCursor или NULL.
 *
 * Задание курсора, используемого при нахождении мышки в видимой области.
 *
 */
void gp_cifro_area_set_point_cursor (GpCifroArea *carea, GdkCursor *cursor);


/**
 * gp_cifro_area_set_move_cursor:
 * @carea: Указатель на объект #GpCifroArea.
 * @cursor: Указатель на #GdkCursor или NULL.
 *
 * Задание курсора, используемого при перемещении видимой области.
 *
 */
void gp_cifro_area_set_move_cursor (GpCifroArea *carea, GdkCursor *cursor);


/**
 * gp_cifro_area_set_minimum_visible:
 * @carea: Указатель на объект #GpCifroArea.
 * @min_width: Минимальная ширина объекта.
 * @min_height: Минимальная высота объекта.
 *
 * Задание минимальных размеров объекта, при которых осуществляется вывод на экран.
 *
 * Минимальные размеры объекта должны находиться в пределах 32 - 1024 точек.
 *
 * Returns: TRUE, если минимальные размеры объекта установлены, FALSE - в случае ошибки.
 *
 */
gboolean gp_cifro_area_set_minimum_visible (GpCifroArea *carea, gint min_width, gint min_height);


/**
 * gp_cifro_area_get_minimum_visible:
 * @carea: Указатель на объект #GpCifroArea.
 * @min_width: (out): Минимальная ширина объекта или NULL.
 * @min_height: (out): Минимальная высота объекта или NULL.
 *
 * Определение текущих минимальных размеров объекта, при которых осуществляется вывод на экран.
 *
 */
void gp_cifro_area_get_minimum_visible (GpCifroArea *carea, gint *min_width, gint *min_height);


/**
 * gp_cifro_area_set_border:
 * @carea: Указатель на объект #GpCifroArea.
 * @left: Отступ с левой стороны;
 * @right: Отступ с правой стороны;
 * @top: Отступ сверху;
 * @bottom: Отступ снизу.
 *
 * Задание размеров области окантовки объекта.
 *
 * Графическая структура объектов #GpCifroArea или #GpCifroImage приведена в описании
 * интерфейса #IcaRenderer. Отступы с каждой стороны должны находитmся в пределах
 * 0 - 1024 точек.
 *
 * Returns: TRUE, если размеры области окантовки установлены, FALSE в - случае ошибки.
 *
 */
gboolean gp_cifro_area_set_border (GpCifroArea *carea, gint left, gint right, gint top, gint bottom);


/**
 * gp_cifro_area_get_border:
 * @carea: #GpCifroArea.
 * @left: (out): Отступ с левой стороны или NULL.
 * @right: (out): Отступ с правой стороны или NULL.
 * @top: (out): Отступ сверху или NULL.
 * @bottom: (out): Отступ снизу или NULL.
 *
 * Определение текущих размеров области окантовки объекта.
 *
 */
void gp_cifro_area_get_border (GpCifroArea *carea, gint *left, gint *right, gint *top, gint *bottom);


/**
 * gp_cifro_area_set_swap:
 * @carea: #GpCifroArea.
 * @swap_x: отражать зеркально изображение относительно вертикальной оси - TRUE или нет - FALSE;
 * @swap_y: отражать зеркально изображение относительно горизонтальной оси - TRUE или нет - FALSE.
 *
 * Задание зеркального отражения изображения по осям.
 *
 */
void gp_cifro_area_set_swap (GpCifroArea *carea, gboolean swap_x, gboolean swap_y);


/**
 * gp_cifro_area_get_swap:
 * @carea: #GpCifroArea.
 * @swap_x: (out): отражать зеркально изображение относительно вертикальной оси - TRUE или нет - FALSE или NULL.
 * @swap_y: (out): отражать зеркально изображение относительно горизонтальной оси - TRUE или нет - FALSE или NULL.
 *
 * Определение текущих параметров зеркального отражения изображения по осям.
 *
 */
void gp_cifro_area_get_swap (GpCifroArea *carea, gboolean *swap_x, gboolean *swap_y);


/**
 * gp_cifro_area_set_move_multiplier:
 * @carea: #GpCifroArea.
 * @move_multiplier: величина перемещения в точках (может быть дробной).
 *
 * Задание величины перемещения изображения при нажатой клавише Ctrl.
 *
 * Величина перемещения должна быть больше нуля.
 *
 * Returns: TRUE, если величина перемещения установлена, FALSE в случае ошибки.
 *
 */
gboolean gp_cifro_area_set_move_multiplier (GpCifroArea *carea, gdouble move_multiplier);


/**
 * gp_cifro_area_get_move_multiplier:
 * @carea: #GpCifroArea.
 *
 * Определение текущих параметров величины перемещения изображения при нажатой клавише Ctrl.
 *
 * Returns: Величина перемещения в точках.
 *
 */
gdouble gp_cifro_area_get_move_multiplier (GpCifroArea *carea);


/**
 * gp_cifro_area_set_shown_limits:
 * @carea: #GpCifroArea.
 * @min_x: минимальное значение по горизонтальной оси.
 * @max_x: максимальное значение по горизонтальной оси.
 * @min_y: минимальное значение по вертикальной оси.
 * @max_y: максимальное значение по вертикальной оси.
 *
 * Задание пределов перемещения/границы изображения.
 *
 * Определяет пределы рисования изображения. Изображение может перемещаться в этих пределах.
 * Формирование изображения также требуется только в этих пределах. Пределы задаются в
 * условных единицах, реальные размеры в точках зависят от коэффициента масштабирования
 * и представления данных модулем формирования изображения.
 *
 * Returns: TRUE, если пределы установлены, FALSE в случае ошибки.
 *
 */
gboolean gp_cifro_area_set_shown_limits (GpCifroArea *carea, gdouble min_x, gdouble max_x, gdouble min_y, gdouble max_y);


/**
 * gp_cifro_area_get_shown_limits:
 * @carea: указатель на объект #GpCifroArea.
 * @min_x: (out): минимальное значение по горизонтальной оси или NULL
 * @max_x: (out): максимальное значение по горизонтальной оси или NULL.
 * @min_y: (out): минимальное значение по вертикальной оси или NULL.
 * @max_y: (out): максимальное значение по вертикальной оси или NULL.
 *
 * Определение текущих значений пределов перемещения/границы изображения.
 *
 * Данная функция определяет значения пределов перемещения/границы изображения.
 *
 */
void gp_cifro_area_get_shown_limits (GpCifroArea *carea, gdouble *min_x, gdouble *max_x, gdouble *min_y, gdouble *max_y);


/**
 * gp_cifro_area_set_rotation:
 * @carea: #GpCifroArea.
 * @rotation: резрешать - TRUE или нет - FALSE изменение угла поворота изображения.
 *
 * Задание разрешения поворота изображения.
 *
 * Изменение параметра разрешения поворота изображения не изменяет текущий угол поворота.
 *
 */
void gp_cifro_area_set_rotation (GpCifroArea *carea, gboolean rotation);


/**
 * gp_cifro_area_get_rotation:
 * @carea: #GpCifroArea.
 *
 * Определение текущих параметров разрешения поворота изображения.
 *
 * Returns: Разрешать - TRUE или нет - FALSE изменение угла поворота изображения.
 *
 */
gboolean gp_cifro_area_get_rotation (GpCifroArea *carea);


/**
 * gp_cifro_area_set_rotate_multiplier:
 * @carea: #GpCifroArea.
 * @rotate_multiplier: величина поворота в радианах.
 *
 * Задание величины поворота изображения при нажатой клавише Ctrl.
 *
 * Величина поворота должна быть больше нуля.
 *
 * Returns: TRUE, если величина поворота установлена, FALSE в случае ошибки.
 *
 */
gboolean gp_cifro_area_set_rotate_multiplier (GpCifroArea *carea, gdouble rotate_multiplier);


/**
 * gp_cifro_area_get_rotate_multiplier:
 * @carea: #GpCifroArea.
 *
 * Определение текущих параметров величины поворота изображения при нажатой клавише Ctrl.
 *
 * Returns: Величина поворота в радианах.
 *
 */
gdouble gp_cifro_area_get_rotate_multiplier (GpCifroArea *carea);


/**
 * gp_cifro_area_set_scale_on_resize:
 * @carea: #GpCifroArea.
 * @scale_on_resize: изменять - TRUE или нет - FALSE масштаб при изменении размеров объекта.
 *
 * Задает поведение масштаба при изменении размеров объекта.
 *
 * При изменении размеров объекта возможно изменение коэффициента масштабирования или
 * изменение видимой области изображения. Если есть определенные функцией #gp_cifro_area_set_fixed_zoom_scales
 * фиксированные масштабы при изменении размеров объекта, всегда будет изменяться видимая область изображения.
 *
 */
void gp_cifro_area_set_scale_on_resize (GpCifroArea *carea, gboolean scale_on_resize);


/**
 * gp_cifro_area_get_scale_on_resize:
 * @carea: #GpCifroArea.
 *
 * Определяет текущее поведение масштаба при изменении размеров объекта.
 *
 * Returns: Изменять - TRUE или нет - FALSE масштаб при изменении размеров объекта.
 *
 */
gboolean gp_cifro_area_get_scale_on_resize (GpCifroArea *carea);


/**
 * gp_cifro_area_set_zoom_on_center:
 * @carea: #GpCifroArea.
 * @zoom_on_center: масштабировать изображение относительно центра - TRUE или относительно текущего местоположения курсора - FALSE.
 *
 * Задает режим масштабирования: относительно центра или текущего положения курсора.
 *
 * Данный параметр влияет только на изменение масштаба с использованием колесика мышки.
 *
 */
void gp_cifro_area_set_zoom_on_center (GpCifroArea *carea, gboolean zoom_on_center);


/**
 * gp_cifro_area_get_zoom_on_center:
 * @carea: #GpCifroArea.
 *
 * Определяет текущий режим масштабирования.
 *
 * Returns: Масштабировать изображение относительно центра - TRUE или относительно текущего местоположения курсора - FALSE.
 *
 */
gboolean gp_cifro_area_get_zoom_on_center (GpCifroArea *carea);


/**
 * gp_cifro_area_set_zoom_scale:
 * @carea: #GpCifroArea.
 * @zoom_scale: величина изменения масштаба.
 *
 * Задает величину изменения масштаба.
 *
 * Величина изменения масштаба определяет на сколько изменится масштаб за один шаг.
 * Величина изменения масштаба задается в процентах и должна быть больше нуля.
 *
 * Returns: TRUE, если величина изменения масштаба установлена, FALSE в случае ошибки.
 *
 */
gboolean gp_cifro_area_set_zoom_scale (GpCifroArea *carea, gdouble zoom_scale);


/**
 * gp_cifro_area_get_zoom_scale:
 * @carea: #GpCifroArea.
 *
 * Определяет текущую величину изменения масштаба.
 *
 * Returns: Величина изменения масштаба.
 *
 */
gdouble gp_cifro_area_get_zoom_scale (GpCifroArea *carea);


/**
 * gp_cifro_area_set_scale_aspect:
 * @carea: #GpCifroArea.
 * @scale_aspect: величина пропорциональности между масштабами.
 *
 * Задает величину пропорциональности между масштабами по разным осям.
 *
 * Если величина пропорциональности определена, соотношение масштабов по разным
 * осям будет оставаться неизменным. Если задана отрицательная величина, поддержание
 * пропорции в масштабах отменяется.
 *
 * Returns: TRUE, если величина пропорциональности между масштабами установлена, FALSE в случае ошибки.
 *
 */
gboolean gp_cifro_area_set_scale_aspect (GpCifroArea *carea, gdouble scale_aspect);


/**
 * gp_cifro_area_get_scale_aspect:
 * @carea: #GpCifroArea.
 *
 * Определяет текущую величину пропорциональности между масштабами.
 *
 * Returns: Величина пропорциональности между масштабами.
 *
 */
gdouble gp_cifro_area_get_scale_aspect (GpCifroArea *carea);


/**
 * gp_cifro_area_set_scale_limits:
 * @carea: #GpCifroArea.
 * @min_scale_x: минимально возможный коэффициент масштаба по горизонтальной оси (приближение).
 * @max_scale_x: максимально возможный коэффициент масштаба по горизонтальной оси (отдаление).
 * @min_scale_y: минимально возможный коэффициент масштаба по вертикальной оси (приближение).
 * @max_scale_y: максимально возможный коэффициент масштаба по вертикальной оси (отдаление).
 *
 * Задает границы изменения масштабов.
 *
 * Returns: TRUE, если границы изменения масштабов установлены, FALSE в случае ошибки.
 *
 */
gboolean gp_cifro_area_set_scale_limits (GpCifroArea *carea, gdouble min_scale_x, gdouble max_scale_x,
                                         gdouble min_scale_y, gdouble max_scale_y);


/**
 * gp_cifro_area_get_scale_limits:
 * @carea: #GpCifroArea.
 * @min_scale_x: (out): минимально возможный коэффициент масштаба по горизонтальной оси (приближение) или NULL.
 * @max_scale_x: (out): максимально возможный коэффициент масштаба по горизонтальной оси (отдаление) или NULL.
 * @min_scale_y: (out): минимально возможный коэффициент масштаба по вертикальной оси (приближение) или NULL.
 * @max_scale_y: (out): максимально возможный коэффициент масштаба по вертикальной оси (отдаление) или NULL.
 *
 * Определяет текущие границы изменения масштабов.
 *
 */
void gp_cifro_area_get_scale_limits (GpCifroArea *carea, gdouble *min_scale_x, gdouble *max_scale_x,
                                     gdouble *min_scale_y, gdouble *max_scale_y);


/**
 * gp_cifro_area_set_fixed_zoom_scales:
 * @carea: #GpCifroArea.
 * @zoom_x_scales: (array length=num_scales): указатель на массив коэффициентов масштабирования по горизонтальной оси.
 * @zoom_y_scales: (array length=num_scales): указатель на массив коэффициентов масштабирования по вертикальной оси.
 * @num_scales: число коэффициентов масштабирования.
 *
 * Задает фиксированный набор масштабов.
 *
 * Если фиксированный набор масштабов определен, используются только коэффициенты масштабирования
 * из этого набора. Если задать пустой набор (num_scales = 0), использование фиксированного набора
 * масштабов отменяется.
 *
 * Returns: TRUE, если фиксированный набор масштабов установлен, FALSE в случае ошибки.
 *
 */
gboolean gp_cifro_area_set_fixed_zoom_scales (GpCifroArea *carea, gdouble *zoom_x_scales, gdouble *zoom_y_scales,
                                              gint num_scales);


/**
 * gp_cifro_area_get_fixed_zoom_scales:
 * @carea: #GpCifroArea.
 * @zoom_x_scales: (out) (array length=num_scales): указатель на массив коэффициентов масштабирования по горизонтальной оси или NULL.
 * @zoom_y_scales: (out) (array length=num_scales): указатель на массив коэффициентов масштабирования по вертикальной оси или NULL.
 * @num_scales: (out): число коэффициентов масштабирования или NULL.
 *
 * Определяет текущий фиксированный набор масштабов.
 *
 * Функция сама выделит память под массивы и вернет указатели на них.
 *
 */
void gp_cifro_area_get_fixed_zoom_scales (GpCifroArea *carea, gdouble **zoom_x_scales, gdouble **zoom_y_scales,
                                          gint *num_scales);


/**
 * gp_cifro_area_set_shown:
 * @carea: #GpCifroArea.
 * @from_x: граница изображения по горизонтальной оси слева.
 * @to_x:   граница изображения по горизонтальной оси справа.
 * @from_y: граница изображения по вертикальной оси снизу.
 * @to_y:   граница изображения по вертикальной оси сверху.
 *
 * Задает границы текущей видимости изображения.
 *
 * Returns: TRUE, если границы текущей видимости изображения установлены, FALSE в случае ошибки.
 *
 */
gboolean gp_cifro_area_set_shown (GpCifroArea *carea, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y);


/**
 * gp_cifro_area_get_shown:
 * @carea: #GpCifroArea.
 * @from_x: (out): граница изображения по горизонтальной оси слева или NULL.
 * @to_x:   (out): граница изображения по горизонтальной оси справа или NULL.
 * @from_y: (out): граница изображения по вертикальной оси снизу или NULL.
 * @to_y:   (out): граница изображения по вертикальной оси сверху или NULL.
 *
 * Определяет границы текущей видимости изображения.
 *
 */
void gp_cifro_area_get_shown (GpCifroArea *carea, gdouble *from_x, gdouble *to_x, gdouble *from_y, gdouble *to_y);


/**
 * gp_cifro_area_set_angle:
 * @carea: #GpCifroArea.
 * @angle: угол текущего поворота изображения в радианах.
 *
 * Задает угол текущего поворота изображения.
 *
 */
void gp_cifro_area_set_angle (GpCifroArea *carea, gdouble angle);


/**
 * gp_cifro_area_get_angle:
 * @carea: #GpCifroArea.
 *
 * Определяет угол текущего поворота изображения.
 *
 * Returns: Угол текущего поворота изображения в радианах.
 *
 */
gdouble gp_cifro_area_get_angle (GpCifroArea *carea);


/**
 * gp_cifro_area_move:
 * @carea: #GpCifroArea.
 * @step_x: шаг смещений по горизонтальной оси;
 * @step_y: шаг смещений по вертикальной оси.
 *
 * Смещает изображение на определенный шаг.
 *
 */
void gp_cifro_area_move (GpCifroArea *carea, gdouble step_x, gdouble step_y);


/**
 * gp_cifro_area_rotate:
 * @carea: #GpCifroArea.
 * @angle: угол поворота изображения.
 *
 * Поворачивает изображение на определенный угол.
 *
 * Угол добавляется к текущему углу, на который повернуто изображение.
 *
 */
void gp_cifro_area_rotate (GpCifroArea *carea, gdouble angle);


/**
 * gp_cifro_area_zoom:
 * @carea: #GpCifroArea.
 * @zoom_x: масштабировать по горизонтальной оси.
 * @zoom_y: масштабировать по вертикальной оси.
 * @center_val_x: центр масштабирования по горизонтальной оси.
 * @center_val_y: центр масштабирования по вертикальной оси.
 * @zoom_in: TRUE - приближение, FALSE - отдаление.
 * @zoom_scale: величина изменения масштаба в процентах.
 *
 * Изменяет текущий масштаб изображения.
 *
 */
void gp_cifro_area_zoom (GpCifroArea *carea, gboolean zoom_x, gboolean zoom_y, gdouble center_val_x,
                         gdouble center_val_y, gboolean zoom_in, gdouble zoom_scale);


/**
 * gp_cifro_area_fixed_zoom:
 * @carea: #GpCifroArea.
 * @center_val_x: центр масштабирования по горизонтальной оси.
 * @center_val_y: центр масштабирования по вертикальной оси.
 * @zoom_in: TRUE - приближение, FALSE - отдаление.
 *
 * Переключение фиксированного масштаба.
 *
 */
void gp_cifro_area_fixed_zoom (GpCifroArea *carea, gdouble center_val_x, gdouble center_val_y, gboolean zoom_in);

/**
 * gp_cifro_area_set_layer_dont_draw_flag:
 * @carea: Указатель на объект #GpCifroArea.
 * @layer_renderer: Указатель на модуль формирования изображений с интерфейсом #IcaRenderer.
 * @flag: Флаг для выключения отрисовки (TRUE - не рисовать, FALSE - рисовать).
 *
 * Устанавливает для слоя с заданным номером признак нерисовть.
 *
 */

void gp_cifro_area_set_layer_dont_draw_flag(GpCifroArea *carea, GpIcaRenderer *layer_renderer, gboolean flag);

/**
 * gp_cifro_area_get_layer_dont_draw_flag:
 * @carea: Указатель на объект #GpCifroArea.
 * @layer_renderer: Указатель на модуль формирования изображений с интерфейсом #IcaRenderer.
 *
 * Получить для слоя с заданным номером признак нерисовть.
 *
 * Returns: Флаг для выключения отрисовки (TRUE - не рисовать, FALSE - рисовать).
 */

gboolean gp_cifro_area_get_layer_dont_draw_flag(GpCifroArea *carea, GpIcaRenderer *layer_renderer);

/**
 * gp_cifro_area_remove_layer:
 * @carea: Указатель на объект #GpCifroArea.
 * @layer_renderer: Указатель на модуль формирования изображений с интерфейсом #IcaRenderer.
 *
 * Удаление модуля формирования изображения.
 *
 * Удаляет в объекте #GpCifroArea модуль формирования изображения.
 *
 */

void gp_cifro_area_remove_layer(GpCifroArea *carea, GpIcaRenderer *layer_renderer);

G_END_DECLS


#endif /* _cifro_area_h */
