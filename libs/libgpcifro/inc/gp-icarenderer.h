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
 * \file gp-icarenderer.h
 *
 * \author Andrei Fadeev
 * \date 5.04.2013
 * \brief Заголовочный файл интерфейса формирования изображений.
 *
 * \defgroup GpIcaRenderer GpIcaRenderer - интерфейс формирования изображений.
 *
 * Интерфейс GpIcaRenderer предназначен для создания унифицированных модулей формирования изображений в
 * предоставляемую внешним объектом (#GpCifroArea или #GpCifroImage) область памяти. Эта область памяти
 * имеет параметры соответствующие области рисования, созданной функцией cairo_image_surface_create
 * библиотеки cairo для формата данных CAIRO_FORMAT_ARGB32.
 *
 * Графическая структура объекта #GpCifroArea или #GpCifroImage имеет следующий вид:
 * <br>
 * +-------------------------+<br>
 * | О      Окантовка      О |<br>
 * | к +-----------------+ к |<br>
 * | а |                 | а |<br>
 * | н |                 | н |<br>
 * | т | Видимая область | т |<br>
 * | о |                 | о |<br>
 * | в |                 | в |<br>
 * | к +-----------------+ к |<br>
 * | а      Окантовка      а |<br>
 * +-------------------------+<br>
 * <br>
 * Прямоугольная область вывода по своей границе имеет окантовку, размеры которой задаются как:
 * - высота верхней и нижней частей;
 * - ширина правой и левой частей.
 *
 * Внутри окантовки содержится видимая область в которой отображаются изображения с учётом их
 * зеркального отражения по осям и поворота на необходимый угол. По границе окантовки происходит
 * обрезка изображений видимой области.
 *
 * Модуль формирования может создавать изображения следующих типов:
 *  - #GP_ICA_RENDERER_AREA - создаётся в системе координат прямоугольной области вывода (включая окантовку);
 *  - #GP_ICA_RENDERER_VISIBLE - создаётся в системе координат видимой области;
 *  - #GP_ICA_RENDERER_STATE - псевдо изображение, может использоваться для хранения информации о параметрах области.
 *
 * При выводе изображения объединяются в одно путём последовательного наложения одного на другое в порядке
 * регистрации обработчиков в объектах #CifroArea и #CifroImage с учётом прозрачностей.
 *
 * Каждый модуль может формировать несколько различных изображений. Число формируемых изображений
 * должна возвращать функция #gp_ica_renderer_get_renderers_num. В дальнейшем выбор конкретного изображений
 * осуществляется идентификатором в пределах от 0 и до числа изображений - 1.
 *
 * В процессе работы модуль будет получать уведомления об изменениях параметров области вывода, посредством
 * вызова следующих функций:
 * - #gp_ica_renderer_set_surface - при изменении адреса памяти в который должно выводится изображения;
 * - #gp_ica_renderer_set_area_size - при изменении размера области вывода (размера виджета, размера финального изображения и пр.);
 * - #gp_ica_renderer_set_visible_size - при изменении размера видимой области;
 * - #gp_ica_renderer_set_shown_limits - при изменении границ отображаемых значений по осям;
 * - #gp_ica_renderer_set_border - при изменении размера окантовки области вывода;
 * - #gp_ica_renderer_set_swap - при изменении парметров зеркального отражения по осям;
 * - #gp_ica_renderer_set_angle - при изменении угла поворота видимой области;
 * - #gp_ica_renderer_set_shown - при изменении отображаемых значений по осям;
 * - #gp_ica_renderer_set_pointer - при изменении текущих координаты курсора мыши.
 *
 * Также в процессе работы модуль может получать уведомления о событиях от устройств ввода
 * (вроде мыши или клавиатуры):
 * - #gp_ica_renderer_button_press_event - при нажатии кнопки (обычно: кнопки мыши);
 * - #gp_ica_renderer_button_release_event - при отпускании нажатой кнопки (обычно: кнопки мыши);
 * - #gp_ica_renderer_key_press_event - при нажатии клавиши (клавиатуры);
 * - #gp_ica_renderer_key_release_event - при отпускании нажатой клавиши (клавиатуры);
 * - #gp_ica_renderer_motion_notify_event - при движении указателя (мыши) над виджетом, взаимодействующим с реализацией интерфейса GpIcaRenderer.
 * Чтобы получать уведомления от какого-либо виджета, следует в его реализации подключить обработчик из списка выше
 * к соответствующему сигналу с помощью g_signal_connect_swapped, при этом
 * в качестве указателя на пользовательские данные передать указатель на реализацию GpIcaRenderer'а.
 * Пример: g_signal_connect_swapped( widget, "button-press-event", G_CALLBACK(gp_ica_renderer_button_press_event), gp_ica_renderer_imp ).
 *
 * Функция #gp_ica_renderer_render - записывает в буфер памяти сформированное изображение, одновременно
 * возвращая начало и границу области затронутой отрисовкой. Т.е. можно сформировать изображение
 * размеры которого меньше чем размеры буфера и находящееся в произвольном месте. В ответ модуль долежн
 * вернуть одно из следующих состояний:
 *
 * - #GP_ICA_RENDERER_AVAIL_NONE - изображение не может быть получено в данный момент ( загружается, обрабатывается и т.п.);
 * - #GP_ICA_RENDERER_AVAIL_ALL - изображение готово;
 * - #GP_ICA_RENDERER_AVAIL_NOT_CHANGED - изображение не изменилось с момента последней проверки.
 *
 * Если модуль находится в состоянии формирования изображения (#GP_ICA_RENDERER_AVAIL_NONE) он может
 * сообщить прогрес выполнения этой задачи в ответ на вызов функции #gp_ica_renderer_get_completion.
 * Диапазон возвращаемых значений от 0 до 1000, где 0 - формирование только началось, 1000 - формирование завершено.
 * Если возвращается отрицательное значение, то прогресс выполнения этой операции не учитывается
 * в общем прогрессе. Функция #gp_ica_renderer_set_total_completion вызывается после того как расчитан
 * общий прогресс выолнения задачи формирования изображения всеми модулями и если он отличается от
 * расчитанного в предыдущий момент. Это значение может использоваться для рисования статусного индикатора.
 *
*/

#ifndef _gp_ica_renderer_h
#define _gp_ica_renderer_h

#include <gtk/gtk.h>


G_BEGIN_DECLS


/**
 * GpIcaRendererType:
 * @GP_ICA_RENDERER_UNKNOWN: Неизвестный тип изображения.
 * @GP_ICA_RENDERER_AREA: Изображение в системе координат прямоугольной области вывода.
 * @GP_ICA_RENDERER_VISIBLE: Изображение в системе координат видимой области.
 * @GP_ICA_RENDERER_STATE: Псевдо изображение.
 *
 * Типы изображений.
 */
typedef enum
{
  GP_ICA_RENDERER_UNKNOWN,
  GP_ICA_RENDERER_AREA,
  GP_ICA_RENDERER_VISIBLE,
  GP_ICA_RENDERER_STATE
}
GpIcaRendererType;


/**
 * GpIcaRendererAvailability:
 * @GP_ICA_RENDERER_AVAIL_NONE: Изображение подготавливается.
 * @GP_ICA_RENDERER_AVAIL_ALL: Изображение готово.
 * @GP_ICA_RENDERER_AVAIL_NOT_CHANGED: Изображение не изменилось.
 *
 * Состояние формирования изображения.
 */
typedef enum
{
  GP_ICA_RENDERER_AVAIL_NONE,
  GP_ICA_RENDERER_AVAIL_ALL,
  GP_ICA_RENDERER_AVAIL_NOT_CHANGED
}
GpIcaRendererAvailability;


#define G_TYPE_GP_ICA_RENDERER                gp_ica_renderer_get_type()
#define GP_ICA_RENDERER( obj )                ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_GP_ICA_RENDERER, GpIcaRenderer ) )
#define GP_ICA_RENDERER_CLASS( vtable )       ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_GP_ICA_RENDERER, GpIcaRendererInterface ) )
#define GP_ICA_RENDERER_GET_CLASS( inst )     ( G_TYPE_INSTANCE_GET_INTERFACE ( ( inst ), G_TYPE_GP_ICA_RENDERER, GpIcaRendererInterface ) )

GType gp_ica_renderer_get_type(void);


typedef struct GpIcaRenderer GpIcaRenderer;

typedef struct GpIcaRendererInterface {

  GTypeInterface parent;

  gint (*get_renderers_num)( GpIcaRenderer *renderer );
  GpIcaRendererType (*get_renderer_type)( GpIcaRenderer *renderer, gint renderer_id );
  const gchar *(*get_name)( GpIcaRenderer *renderer, gint renderer_id );

  void      (*set_surface)( GpIcaRenderer *renderer, gint renderer_id, guchar *data, gint width, gint height, gint stride );
  void    (*set_area_size)( GpIcaRenderer *renderer, gint width, gint height );
  void (*set_visible_size)( GpIcaRenderer *renderer, gint width, gint height );

  gint (*get_completion)( GpIcaRenderer *renderer, gint renderer_id );
  void (*set_total_completion)( GpIcaRenderer *renderer, gint completion );
  GpIcaRendererAvailability (*render)( GpIcaRenderer *renderer, gint renderer_id, gint *x, gint *y, gint *width, gint *height );
  void   (*set_shown_limits)( GpIcaRenderer *renderer, gdouble min_x, gdouble max_x, gdouble min_y, gdouble max_y );

  void  (*set_border)( GpIcaRenderer *renderer, gint left, gint right, gint top, gint bottom );
  void    (*set_swap)( GpIcaRenderer *renderer, gboolean swap_x, gboolean swap_y );
  void   (*set_angle)( GpIcaRenderer *renderer, gdouble angle );
  void   (*set_shown)( GpIcaRenderer *renderer, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y );
  void (*set_pointer)( GpIcaRenderer *renderer, gint pointer_x, gint pointer_y, gdouble value_x, gdouble value_y );

  gboolean   (*button_press_event)( GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget );
  gboolean (*button_release_event)( GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget );
  gboolean      (*key_press_event)( GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget );
  gboolean    (*key_release_event)( GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget );
  gboolean  (*motion_notify_event)( GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget );

} GpIcaRendererInterface;


/**
 * gp_ica_renderer_get_renderers_num:
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 *
 * Возвращает число формирователей изображенй реализованных в данном модуле.
 *
 * Returns: Число формирователей изображений.
 *
 */
gint gp_ica_renderer_get_renderers_num(GpIcaRenderer *renderer);


/**
 * gp_ica_renderer_get_name:
 * @renderer: Указатель на интерфейс #GpIcaRenderer.
 * @renderer_id: Идентификатор формирователя изображения.
 *
 * Название формирователя изображения.
 *
 * Возвращает название формирователя изображения с указанным идентификтором.
 * Идентификаторы формирователей изображений находятся в диапазоне от 0
 * до числа формирователей изображений - 1, возвращаемого функцией
 * #gp_ica_renderer_get_renderers_num.
 *
 * Returns: Название формирователя изображения.
 *
 */
const gchar *gp_ica_renderer_get_name(GpIcaRenderer *renderer, gint renderer_id);


/**
 * gp_ica_renderer_get_renderer_type:
 * @renderer: Указатель на интерфейс #GpIcaRenderer.
 * @renderer_id: Идентификатор формирователя изображения.
 *
 * Тип формирователя изображения.
 *
 * Возвращает тип формирователя изображения с указанным идентификтором.
 * Идентификаторы формирователей изображений находятся в диапазоне от 0
 * до числа формирователей изображений - 1, возвращаемого функцией
 * #gp_ica_renderer_get_renderers_num.
 *
 * Returns: Тип формирователя изображения.
 */
GpIcaRendererType gp_ica_renderer_get_renderer_type(GpIcaRenderer *renderer, gint renderer_id);


/**
 * gp_ica_renderer_set_surface:
 * @renderer: Указатель на интерфейс GpIcaRenderer.
 * @renderer_id: Идентификатор формирователя изображения.
 * @data: (array) (element-type guint8): Буфер в памяти.
 * @width: Логическая ширина изображения буфера памяти в пикселях.
 * @height: Логическая высота изображения буфера памяти в пикселях.
 * @stride: Расстояние в байтах между началами дву соседних строк изображения.
 *
 * Задание адреса буфера памяти.
 * Задаёт адреса буфера памяти для формируемых изображений. Запись в буфер разрешена
 * только при вызове функции gp_ica_renderer_render. При увеличении размера объекта
 * CifroArea или CifroImage более размера буфера выделяется новый
 * буфер. При изменении размера объекта до размера менее или равном размеру буфера изменяются
 * логические размеры, в этом случае новые размеры сообщаются вызовом функций
 * gp_ica_renderer_set_area_size и gp_ica_renderer_set_visible_size.
 */
void gp_ica_renderer_set_surface(GpIcaRenderer *renderer, gint renderer_id, guchar *data, gint width, gint height,
                                 gint stride);


/**
 * gp_ica_renderer_set_area_size:
 * @renderer: Указатель на интерфейс #GpIcaRenderer.
 * @width: Ширина области вывода в пикселях.
 * @height: Высота области вывода в пикселях.
 *
 * Размер области вывода.
 *
 * Задаёт новые размеры области вывода объектов #CifroArea или #CifroImage.
 * Область вывода включает в себя окантовку.
 */
void gp_ica_renderer_set_area_size(GpIcaRenderer *renderer, gint width, gint height);


/**
 * gp_ica_renderer_set_visible_size:
 * @renderer: Указатель на интерфейс #GpIcaRenderer.
 * @width: Ширина видимой области в пикселях.
 * @height: Высота видимой области в пикселях.
 *
 * Размер видимой области.
 *
 * Задаёт новые размеры видимой области объектов #CifroArea или #CifroImage.
 * Видимая область ограничена окантовкой, ее размеры в том числе зависят от
 * угла, на который производится поворот изображения.
 */
void gp_ica_renderer_set_visible_size(GpIcaRenderer *renderer, gint width, gint height);


/**
 * gp_ica_renderer_get_completion:
 * @renderer: Указатель на интерфейс #GpIcaRenderer.
 * @renderer_id: Идентификатор формирователя изображения.
 *
 * Прогресс формирования изображения.
 *
 * Возвращает численое значение прогреса формирования изображения 0 - 1000.
 * Если возвращется значение меньше нуля оно не учитывается в общем прогрессе.
 *
 * Returns: Число от 0 до 1000 или отрицательное число.
 */
gint gp_ica_renderer_get_completion(GpIcaRenderer *renderer, gint renderer_id);


/**
 * gp_ica_renderer_set_total_completion:
 * @renderer: Указатель на интерфейс #GpIcaRenderer.
 * @completion: Общий прогресс формирования изображения.
 *
 * Задание общего прогресса формирования изображения.
 *
 * Функция вызывается при изменении общего прогресса формирования изображения.
 *
*/
void gp_ica_renderer_set_total_completion(GpIcaRenderer *renderer, gint completion);


/**
 * gp_ica_renderer_render:
 * @renderer: Указатель на интерфейс GpIcaRenderer.
 * @renderer_id: Идентификатор формирователя изображения.
 * @x: (inout) (transfer none): Координата по оси x начала области изменения.
 * @y: (inout) (transfer none): Координата по оси y начала области изменения.
 * @width: (inout) (transfer none): Ширина области изменения.
 * @height: (inout) (transfer none): Высота области изменения.
 *
 * Копирование изображения в буфер.
 * Производит копирование сформированного изображения в буфер и возвращает координаты
 * области в которой изображение было изменено.
 *
 * Returns: Результат (статус) операции копирования.
 */
GpIcaRendererAvailability gp_ica_renderer_render(GpIcaRenderer *renderer, gint renderer_id, gint *x, gint *y,
                                                 gint *width, gint *height);


/**
 * gp_ica_renderer_set_shown_limits:
 * @renderer: Указатель на интерфейс #GpIcaRenderer.
 * @min_x: Минимально возможное значение по оси x.
 * @max_x: Максимально возможное значение по оси x.
 * @min_y: Минимально возможное значение по оси y.
 * @max_y: Максимально возможное значение по оси y.
 *
 * Задание границ возможных отображаемых значений.
 *
 * Функция вызывается при изменении границ.
 */
void gp_ica_renderer_set_shown_limits(GpIcaRenderer *renderer, gdouble min_x, gdouble max_x, gdouble min_y,
                                      gdouble max_y);


/**
 * gp_ica_renderer_set_border:
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @left: ширина области окантовки от левого края.
 * @right: ширина области окантовки от правого края.
 * @top: высота области окантовки от верхнего края.
 * @bottom: высота области окантовки от нижнего края.
 *
 * Задание размеров окантовки.
 *
 * Функция вызывается при изменении размеров окантовки области вывода.
 */
void gp_ica_renderer_set_border(GpIcaRenderer *renderer, gint left, gint right, gint top, gint bottom);


/**
 * gp_ica_renderer_set_swap:
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @swap_x: TRUE - если при выводе изображение зеркально отражается по оси x, иначе - FALSE.
 * @swap_y: TRUE - если при выводе изображение зеркально отражается по оси y, иначе - FALSE.
 *
 * Задание зеркального отражения по осям видимой области.
 *
 * Функция вызывается при изменении параметров зеркального отражения по осям.
 */
void gp_ica_renderer_set_swap(GpIcaRenderer *renderer, gboolean swap_x, gboolean swap_y);


/**
 * gp_ica_renderer_set_angle:
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @angle: угол в радианах на который производится поворот.
 *
 * Задание угла поворота видимой области.
 *
 * Функция вызывается при изменении угла поворота видимой области. Угол отсчитывается
 * от вертикальной оси и имеет положительное значение при повороте на лево.
 */
void gp_ica_renderer_set_angle(GpIcaRenderer *renderer, gdouble angle);


/**
 * gp_ica_renderer_set_shown:
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @from_x: граница отображения по оси x слева.
 * @to_x: граница отображения по оси x справа.
 * @from_y: граница отображения по оси y снизу.
 * @to_y: граница отображения по оси y сверху.

 * Задание логических координат отображаемых значений.
 *
 * Функция вызывается при изменении логических координат.
 */
void gp_ica_renderer_set_shown(GpIcaRenderer *renderer, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y);


/**
 * gp_ica_renderer_set_pointer:
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @pointer_x: координата по оси x курсора, в пикселях относительно границ окна.
 * @pointer_y: координата по оси y курсора, в пикселях относительно границ окна.
 * @value_x: координата по оси x курсора, в логических координатах.
 * @value_y: координата по оси y курсора, в логических координатах.
 *
 * Задание координат курсора (логических и физических).
 *
 * Функция вызывается при изменении координат (перемещении) курсора мыши.
 */
void gp_ica_renderer_set_pointer(GpIcaRenderer *renderer, gint pointer_x, gint pointer_y, gdouble value_x,
                                 gdouble value_y);


/**
 * gp_ica_renderer_button_press_event:
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @event: событие GdkEventButton.
 * @widget: виджет, взаимодействующий с реализацией интерфейса GpIcaRenderer.
 *
 * Нажатие кнопки (мыши).

 * Функция вызывается при нажатии кнопки (обычно: кнопки мыши)
 * над виджетом, взаимодействующим с реализацией интерфейса GpIcaRenderer.
 *
 * Returns: TRUE, если нужно запретить выполнение всех остальных обработчиков нажатия кнопки.
 *  FALSE, если нужно выполнить остальные обработчики.
 */
gboolean gp_ica_renderer_button_press_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget);


/**
 * gp_ica_renderer_button_release_event:
 * Отпускание кнопки (мыши).
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @event: событие GdkEventButton.
 * @widget: виджет, взаимодействующий с реализацией интерфейса GpIcaRenderer.
 *
 * Функция вызывается при отпускании кнопки (обычно: кнопки мыши)
 * над виджетом, взаимодействующим с реализацией интерфейса GpIcaRenderer.
 *
 * Returns: TRUE, если нужно запретить выполнение всех остальных обработчиков отпускания кнопки.
 *  FALSE, если нужно выполнить остальные обработчики.
 */
gboolean gp_ica_renderer_button_release_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget);


/**
 * gp_ica_renderer_key_press_event:
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @event: событие GdkEventKey.
 * @widget: виджет, взаимодействующий с реализацией интерфейса GpIcaRenderer.
 *
 * Нажатие клавиши (клавиатуры).
 *
 * Функция вызывается при нажатии клавиши (клавиатуры),
 * перехваченном виджетом, взаимодействующим с реализацией интерфейса GpIcaRenderer.
 *
 * Returns: TRUE, если нужно запретить выполнение всех остальных обработчиков нажатия кнопки.
 *  FALSE, если нужно выполнить остальные обработчики.
 *
 */
gboolean gp_ica_renderer_key_press_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget);


/**
 * gp_ica_renderer_key_release_event:
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @event: событие GdkEventKey.
 * @widget: виджет, взаимодействующий с реализацией интерфейса GpIcaRenderer.
 *
 * Отпускание клавиши (клавиатуры).
 *
 * Функция вызывается при отпускании клавиши (клавиатуры),
 * перехваченном виджетом, взаимодействующим с реализацией интерфейса GpIcaRenderer.
 *
 * Returns: TRUE, если нужно запретить выполнение всех остальных обработчиков отпускания кнопки.
 *  FALSE, если нужно выполнить остальные обработчики.
 *
 */
gboolean gp_ica_renderer_key_release_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget);


/**
 * gp_ica_renderer_motion_notify_event:
 * Движение указателя (мыши) над виджетом.
 * @renderer: указатель на интерфейс #GpIcaRenderer.
 * @event: событие GdkEventMotion.
 * @widget: виджет, взаимодействующий с реализацией интерфейса GpIcaRenderer.
 *
 * Функция вызывается при движении указателя мыши
 * над виджетом, реализующим интерфейс GpIcaRenderer.
 *
 * Returns: TRUE, если нужно запретить выполнение всех остальных обработчиков движения указателя.
 *  FALSE, если нужно выполнить остальные обработчики.
 *
 */
gboolean gp_ica_renderer_motion_notify_event(GpIcaRenderer *renderer, GdkEvent *event, GtkWidget *widget);


G_END_DECLS

#endif // _gp_ica_renderer_h
