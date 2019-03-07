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

#ifndef _GOPNIK_CONFIG_H
#define _GOPNIK_CONFIG_H

#include <glib.h>

// Путь к устройствам PCH_GPIO.
static const gchar *PCH_GPIO_DIR_PATH = "/sys/bus/pci/drivers/pch_gpio/";

// Размер области mmap.
static const size_t MMAP_LENGTH = 4096;

// Количество бит в load-тестах.
static const guint LOAD_TEST_BITS = 67110520; //< (8388815 * 8)

// Количество бит в download-тестах.
static const guint DOWNLOAD_TEST_BITS = 6711052; //< 10% от (8388815 * 8)

// Пины GPIO для прошивки ПЛИС.
#define PIN_DIOD      0 //< GPI_0/A54 - диод.
#define PIN_NCONFIG   1 //< GPI_1/A63 - nCONFIG.
#define PIN_DATA      2 //< GPI_2/A67 - DATA.
#define PIN_DCLK      3 //< GPI_3/A85 - DCLK

#define PIN_CONF_DONE 4 //< GPO_0/A93 - CONFDONE (! Можно закомментировать PIN, тогда он не будет использоваться при прошивке).
//#define PIN_NSTATUS   5 //< GPO_1/B54 - nSTATUS (! Можно закомментировать PIN, тогда он не будет использоваться при прошивке).

//#define PIN_TODO      6 //< GPO_2/B57 - зарезервировано.
//#define PIN_TOTOD     7 //< GPO_3/B63 - зарезервировано.

#endif

