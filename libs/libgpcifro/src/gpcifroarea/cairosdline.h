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
 * \file cairosdline.h
 *
 * \author Andrei Fadeev
 * \date 23.01.2013
 *
 * \brief Заголовочный файл процедуры рисования линий в image_surface тип cairo.
 *
 */

#ifndef _cairo_sdline_h
#define _cairo_sdline_h

#include <stdint.h>
#include <cairo.h>


typedef struct cairo_sdline_surface {

  cairo_t         *cairo;         // Указатель на объект cairo.
  cairo_surface_t *cairo_surface; // Указатель на поверхность cairo.

  int             width;          // Ширина поверхности.
  int             height;         // Высота поверхности.
  int             stride;         // Размер одной линии поверхности в байтах.

  unsigned char   *data;          // Пиксели поверхности.
  int             self_create;    // 1 - поверхность создавали мы, удаляем сами, иначе - 0.

} cairo_sdline_surface;

cairo_sdline_surface* cairo_sdline_surface_create( int width, int height );
cairo_sdline_surface* cairo_sdline_surface_create_for( cairo_surface_t *cairo_surface );
void cairo_sdline_surface_destroy( cairo_sdline_surface *surface );

uint32_t cairo_sdline_color( double red, double green, double blue, double alpha );
void cairo_sdline_set_cairo_color( cairo_sdline_surface *surface, uint32_t color );

void cairo_sdline_clear( cairo_sdline_surface *surface );
void cairo_sdline_clear_color( cairo_sdline_surface *surface, uint32_t color );

void cairo_sdline_h( cairo_sdline_surface *surface, int x1, int x2, int y1, uint32_t color );
void cairo_sdline_v( cairo_sdline_surface *surface, int x1, int y1, int y2, uint32_t color );
void cairo_sdline( cairo_sdline_surface *surface, int x1, int y1, int x2, int y2, uint32_t color );

void cairo_sdline_bar( cairo_sdline_surface *surface, int x1, int y1, int x2, int y2, uint32_t color );
void cairo_sdline_dot( cairo_sdline_surface *surface, int x, int y, uint32_t color );


#endif /* _cairo_sdline_h */
