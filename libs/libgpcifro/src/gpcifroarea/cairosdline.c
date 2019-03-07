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
 * \file cairosdline.c
 *
 * \author Andrei Fadeev
 * \date 23.01.2013
 *
 * \brief Файл процедуры рисования линий в image_surface тип cairo.
 *
 */

#include "cairosdline.h"

#include <stdlib.h>
#include <string.h>

#define PIXEL_SIZE 4


/* --------- Clipping routines for line */

/* Clipping based heavily on code from                       */
/* http://www.ncsa.uiuc.edu/Vis/Graphics/src/clipCohSuth.c   */

#define CLIP_LEFT_EDGE   0x1
#define CLIP_RIGHT_EDGE  0x2
#define CLIP_BOTTOM_EDGE 0x4
#define CLIP_TOP_EDGE    0x8
#define CLIP_INSIDE(a)   (!a)
#define CLIP_REJECT(a,b) (a&b)
#define CLIP_ACCEPT(a,b) (!(a|b))

/*!
\brief Internal clip-encoding routine.

Calculates a segement-based clipping encoding for a point against a rectangle.

\param x X coordinate of point.
\param y Y coordinate of point.
\param left X coordinate of left edge of the rectangle.
\param top Y coordinate of top edge of the rectangle.
\param right X coordinate of right edge of the rectangle.
\param bottom Y coordinate of bottom edge of the rectangle.
*/
static int _clipEncode( int x, int y, int left, int top, int right, int bottom )
{
  int code = 0;

  if (x < left) {
    code |= CLIP_LEFT_EDGE;
  } else if (x > right) {
    code |= CLIP_RIGHT_EDGE;
  }
  if (y < top) {
    code |= CLIP_TOP_EDGE;
  } else if (y > bottom) {
    code |= CLIP_BOTTOM_EDGE;
  }
  return code;
}

/*!
\brief Clip line to a the clipping rectangle of a surface.

\param surface Target surface to draw on.
\param x1 Pointer to X coordinate of first point of line.
\param y1 Pointer to Y coordinate of first point of line.
\param x2 Pointer to X coordinate of second point of line.
\param y2 Pointer to Y coordinate of second point of line.
*/
static int _clipLine( cairo_sdline_surface *surface, int *x1, int *y1, int *x2, int *y2 )
{
  int left, right, top, bottom;
  int code1, code2;
  int draw = 0;
  int swaptmp;
  float m;

  /*
  * Get clipping boundary
  */
  left = 0;
  right = surface->width - 1;
  top = 0;
  bottom = surface->height - 1;

  while (1) {
    code1 = _clipEncode(*x1, *y1, left, top, right, bottom);
    code2 = _clipEncode(*x2, *y2, left, top, right, bottom);
    if (CLIP_ACCEPT(code1, code2)) {
      draw = 1;
      break;
    } else if (CLIP_REJECT(code1, code2))
      break;
    else {
      if (CLIP_INSIDE(code1)) {
        swaptmp = *x2;
        *x2 = *x1;
        *x1 = swaptmp;
        swaptmp = *y2;
        *y2 = *y1;
        *y1 = swaptmp;
        swaptmp = code2;
        code2 = code1;
        code1 = swaptmp;
      }
      if (*x2 != *x1) {
        m = (float)(*y2 - *y1) / (float)(*x2 - *x1);
      } else {
        m = 1.0f;
      }
      if (code1 & CLIP_LEFT_EDGE) {
        *y1 += (int) ((left - *x1) * m);
        *x1 = left;
      } else if (code1 & CLIP_RIGHT_EDGE) {
        *y1 += (int) ((right - *x1) * m);
        *x1 = right;
      } else if (code1 & CLIP_BOTTOM_EDGE) {
        if (*x2 != *x1) {
          *x1 += (int) ((bottom - *y1) / m);
        }
        *y1 = bottom;
      } else if (code1 & CLIP_TOP_EDGE) {
        if (*x2 != *x1) {
          *x1 += (int) ((top - *y1) / m);
        }
        *y1 = top;
      }
    }
  }

  return draw;
}


cairo_sdline_surface* cairo_sdline_surface_create( int width, int height )
{

  cairo_sdline_surface *surface = cairo_sdline_surface_create_for( cairo_image_surface_create( CAIRO_FORMAT_ARGB32, width, height ) );

  if( surface ) surface->self_create = 1;

  return surface;

}


cairo_sdline_surface* cairo_sdline_surface_create_for( cairo_surface_t *cairo_surface )
{

  cairo_sdline_surface *surface;

  if( cairo_surface_get_type( cairo_surface ) != CAIRO_SURFACE_TYPE_IMAGE ) return NULL;

  if( ( cairo_image_surface_get_format( cairo_surface ) != CAIRO_FORMAT_ARGB32 ) &&
      ( cairo_image_surface_get_format( cairo_surface ) != CAIRO_FORMAT_RGB24 ) ) return NULL;

  surface = malloc( sizeof ( cairo_sdline_surface ) );
  if( !surface ) return NULL;

  surface->cairo = cairo_create( cairo_surface );
  surface->cairo_surface = cairo_surface;

  surface->width = cairo_image_surface_get_width( cairo_surface );
  surface->height = cairo_image_surface_get_height( cairo_surface );
  surface->stride = cairo_image_surface_get_stride( cairo_surface );
  surface->data = cairo_image_surface_get_data( cairo_surface );

  surface->self_create = 0;

  return surface;

}


void cairo_sdline_surface_destroy( cairo_sdline_surface *surface )
{

  if( surface == NULL ) return;
  cairo_destroy( surface->cairo );
  if( surface->self_create ) cairo_surface_destroy( surface->cairo_surface );
  free( surface );

}


uint32_t cairo_sdline_color( double red, double green, double blue, double alpha )
{

  uint32_t color;
  uint32_t ired, igreen, iblue, ialpha;

  if( red < 0.0 )   red = 0.0;
  if( red > 1.0 )   red = 1.0;
  if( green < 0.0 ) green = 0.0;
  if( green > 1.0 ) green = 1.0;
  if( blue < 0.0 )  blue = 0.0;
  if( blue > 1.0 )  blue = 1.0;
  if( alpha < 0.0 ) alpha = 0.0;
  if( alpha > 1.0 ) alpha = 1.0;

  ired = red * 0xFF;
  igreen = green * 0xFF;
  iblue = blue * 0xFF;
  ialpha = alpha * 0xFF;

  color = ( ialpha << 24 ) | ( ired << 16 ) | ( igreen << 8 ) | ( iblue );
  return color;

}


void cairo_sdline_set_cairo_color( cairo_sdline_surface *surface, uint32_t color )
{

  double red, green, blue, alpha;

  if( !surface ) return;

  alpha =  (double)( ( color >> 24 ) & 0xFF ) / 255.0;
  red =    (double)( ( color >> 16 ) & 0xFF ) / 255.0;
  green =  (double)( ( color >> 8 ) & 0xFF ) / 255.0;
  blue =   (double)( color & 0xFF ) / 255.0;

  cairo_set_source_rgba( surface->cairo, red, green, blue, alpha );

}


void cairo_sdline_clear( cairo_sdline_surface *surface )
{

  if( !surface ) return;
  memset( surface->data, 0, surface->height * surface->stride );

}


void cairo_sdline_clear_color( cairo_sdline_surface *surface, uint32_t color )
{

  int i, shift;

  if( !surface ) return;

  for( i = 0, shift = 0; i < surface->width; i++, shift += PIXEL_SIZE )
    *(uint32_t*)( surface->data + shift ) = color;

  for( i = 1, shift = surface->stride; i < surface->height; i++, shift += surface->width * PIXEL_SIZE )
    memcpy( surface->data + shift, surface->data, surface->width * PIXEL_SIZE );

}


void cairo_sdline_h( cairo_sdline_surface *surface, int x1, int x2, int y1, uint32_t color )
{

  int i, shift;
  int swaptmp;

  if( !surface ) return;

  if( ( y1 < 0 ) || ( y1 >= surface->height ) ) return;

  if( x1 < 0 ) x1 = 0;
  if( x1 >= surface->width ) x1 = surface->width - 1;

  if( x2 < 0 ) x2 = 0;
  if( x2 >= surface->width ) x2 = surface->width - 1;

  if( x1 > x2 ) { swaptmp = x1, x1 = x2; x2 = swaptmp; }

  shift = ( y1 * surface->stride ) + PIXEL_SIZE * x1;
  for( i = x1; i <= x2; i++, shift += PIXEL_SIZE )
    *(uint32_t*)( surface->data + shift ) = color;

}


void cairo_sdline_v( cairo_sdline_surface *surface, int x1, int y1, int y2, uint32_t color )
{

  int i, shift;
  int swaptmp;

  if( !surface ) return;

  if( ( x1 < 0 ) || ( x1 >= surface->width ) ) return;

  if( y1 < 0 ) y1 = 0;
  if( y1 >= surface->height ) y1 = surface->height - 1;

  if( y2 < 0 ) y2 = 0;
  if( y2 >= surface->height ) y2 = surface->height - 1;

  if( y1 > y2 ) { swaptmp = y1, y1 = y2; y2 = swaptmp; }

  shift = ( y1 * surface->stride ) + PIXEL_SIZE * x1;
  for( i = y1; i <= y2; i++, shift += surface->stride )
    *(uint32_t*)( surface->data + shift ) = color;

}


void cairo_sdline( cairo_sdline_surface *surface, int x1, int y1, int x2, int y2, uint32_t color )
{

  int pixx, pixy;
  int x, y;
  int dx, dy;
  int sx, sy;
  int swaptmp;
  unsigned char *pixel;

  if( !surface ) return;

  if( !(_clipLine( surface, &x1, &y1, &x2, &y2 ) ) ) return;

  if( x1 == x2 ) { cairo_sdline_v( surface, x1, y1, y2, color ); return; }
  if( y1 == y2 ) { cairo_sdline_h( surface, x1, x2, y1, color ); return; }

  dx = x2 - x1;
  dy = y2 - y1;
  sx = (dx >= 0) ? 1 : -1;
  sy = (dy >= 0) ? 1 : -1;

  dx = sx * dx + 1;
  dy = sy * dy + 1;
  pixx = PIXEL_SIZE;
  pixy = surface->stride;
  pixel = surface->data + pixx * (int) x1 + pixy * (int) y1;
  pixx *= sx;
  pixy *= sy;
  if (dx < dy) {
    swaptmp = dx;
    dx = dy;
    dy = swaptmp;
    swaptmp = pixx;
    pixx = pixy;
    pixy = swaptmp;
  }

  for( x = 0, y = 0; x < dx; x++, pixel += pixx ) {
    *(uint32_t*)pixel = color;
    y += dy;
    if( y >= dx ) {
      y -= dx;
      pixel += pixy;
    }
  }

}


void cairo_sdline_bar( cairo_sdline_surface *surface, int x1, int y1, int x2, int y2, uint32_t color )
{

  int i, j, shift;
  int swaptmp;

  if( x1 < 0 ) x1 = 0;
  if( x1 >= surface->width ) x1 = surface->width - 1;

  if( x2 < 0 ) x2 = 0;
  if( x2 >= surface->width ) x2 = surface->width - 1;

  if( y1 < 0 ) y1 = 0;
  if( y1 >= surface->height ) y1 = surface->height - 1;

  if( y2 < 0 ) y2 = 0;
  if( y2 >= surface->height ) y2 = surface->height - 1;

  if( x1 > x2 ) { swaptmp = x1, x1 = x2; x2 = swaptmp; }
  if( y1 > y2 ) { swaptmp = y1, y1 = y2; y2 = swaptmp; }

  for( j = y1; j <= y2; j++ )
    {
    shift = ( j * surface->stride ) + PIXEL_SIZE * x1;
    for( i = x1; i <= x2; i++, shift += PIXEL_SIZE )
      *(uint32_t*)( surface->data + shift ) = color;
    }

}


void cairo_sdline_dot( cairo_sdline_surface *surface, int x, int y, uint32_t color )
{

  if( x < 0 || y < 0 ) return;
  if( x >= surface->width || y >= surface->height ) return;

  int shift = ( y * surface->stride ) + PIXEL_SIZE * x;
  *(uint32_t*)( surface->data + shift ) = color;

}
