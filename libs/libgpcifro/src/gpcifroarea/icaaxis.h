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

/*!
 * \file icaaxis.h
 *
 * \author Andrei Fadeev
 * \date 16.07.2014
 * \brief Заголовочный файл класса отображения осей.
 *
*/

#ifndef _gp_ica_axis_h
#define _gp_ica_axis_h

#include <glib.h>

#include "cairosdline.h"
#include "gp-icastaterenderer.h"


/*! \brief Структура с параметрами отображения осей.  */
typedef struct GpIcaAxis {

  guint32                ground_color;     /*!< Цвет подложки. */
  guint32                border_color;     /*!< Цвет обрамления области отображения данных. */
  guint32                axis_color;       /*!< Цвет осей. */
  guint32                zero_axis_color;  /*!< Цвет осей для нулевых значений. */
  guint32                text_color;       /*!< Цвет подписей. */

  gchar                 *x_axis_name;      /*!< Подпись оси абсцисс. */
  gchar                 *y_axis_name;      /*!< Подпись оси ординат. */

} GpIcaAxis;


/*! Рисование координатных линий в видимой области.
 *
 * Функция рисует координатные линии в поверхности связанной с видимой
 * областью объекта \link GpIcaRenderer \endlink.
 *
 * \param surface указатель на структуру с описанием поверхности;
 * \param state_renderer укзатель на объект \link GpIcaStateRenderer \endlink;
 * \param axis_info параметры рисования осей.
 *
 * \return Нет.
 *
*/
void gp_ica_axis_draw_axis( cairo_sdline_surface *surface, GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info );


/*! Рисование оцифровки оси абсцисс.
 *
 * Функция рисует линейку оцифровки оси абсцисс в поверхности связанной
 * с областью вывода объекта \link GpIcaRenderer \endlink. Линейка рисуется
 * в области горизонтальной окантовки вверху.
 *
 * \param surface указатель на структуру с описанием поверхности;
 * \param font_layout раскладка шрифта для рисования оцифровки;
 * \param state_renderer укзатель на объект \link GpIcaStateRenderer \endlink;
 * \param axis_info параметры рисования осей.
 *
 * \return Нет.
 *
*/
void gp_ica_axis_draw_hruler( cairo_sdline_surface *surface, PangoLayout *font_layout,
                              GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info );


/*! Рисование оцифровки оси ординат.
 *
 * Функция рисует линейку оцифровки оси ординат в поверхности связанной
 * с областью вывода объекта \link GpIcaRenderer \endlink. Линейка рисуется
 * в области вертикальной окантовки слева.
 *
 * \param surface указатель на структуру с описанием поверхности;
 * \param font_layout раскладка шрифта для рисования оцифровки;
 * \param state_renderer укзатель на объект \link GpIcaStateRenderer \endlink;
 * \param axis_info параметры рисования осей.
 *
 * \return Нет.
 *
*/
void gp_ica_axis_draw_vruler( cairo_sdline_surface *surface, PangoLayout *font_layout,
                              GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info );


/* Рисование "планшета" горизонтального местоположения видимой области.
 *
 * "Планшет" рисуется в области горизонтальной окантовки внизу.
 *
 * \param surface указатель на структуру с описанием поверхности;
 * \param state_renderer укзатель на объект \link GpIcaStateRenderer \endlink;
 * \param axis_info параметры рисования осей.
 *
 * \return Нет.
 *
*/
void gp_ica_axis_draw_x_pos( cairo_sdline_surface *surface, GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info );


/* Рисование "планшета" вертикального местоположения видимой области.
 *
 * "Планшет" рисуется в области вертикальной окантовки справа.
 *
 * \param surface указатель на структуру с описанием поверхности;
 * \param state_renderer укзатель на объект \link GpIcaStateRenderer \endlink;
 * \param axis_info параметры рисования осей.
 *
 * \return Нет.
 *
*/
void gp_ica_axis_draw_y_pos( cairo_sdline_surface *surface, GpIcaStateRenderer *state_renderer, GpIcaAxis *axis_info );


#endif // _gp_ica_axis.h
