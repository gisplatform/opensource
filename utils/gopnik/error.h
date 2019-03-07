/*
 * gopnik is a small program for EG20T GPIO.
 *
 * Copyright 2017 Sergey Volkhin.
 *
 * This file is part of gopnik.
 *
 * gopnik is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * gopnik is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with gopnik. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#ifndef _GOPNIK_ERROR_H
#define _GOPNIK_ERROR_H

#include <glib.h>

/**
* GopnikError:
* @GOPNIK_ERROR_FPGA: Ошибка взаимодействия с ПЛИС.
*
* Виды исключений Gopnik.
*/
typedef enum
{
  GOPNIK_ERROR_FPGA
}
GopnikError;

/**
* GOPNIK_ERROR:
*
* Макрос для получения #GError quark для ошибок gopnik.
*/
#define GOPNIK_ERROR (gopnik_error_quark())
static inline GQuark gopnik_error_quark(void) G_GNUC_CONST;

static inline GQuark gopnik_error_quark(void)
{
  return g_quark_from_static_string("gopnik");
}

#endif

