/*
 * GpFileUtils is a file utils library.
 *
 * Copyright (C) 2016 Sergey Volkhin, Andrey Vodilov.
 *
 * GpFileUtils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpFileUtils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpFileUtils. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include <locale.h>
#include <gp-core.h>


int main(int argc, char **argv)
{
  setlocale(LC_CTYPE, "");

  gchar *names[] =
  {
    "21Черника",
    "21Черника",
    "21Черника",
    "3Черника",
    "А_21Черника",
    "А_21Черника",
    NULL,
    "А_21Черника",
    "А_3Черника",
    "Cherry03",
    "Cherry02",
    "Cherry04",
    "Cherry 04",
    NULL,
    "Cherry 2",
    "Cherry 15",
    "Blackberry100and10",
    "Blackberry20and10",
    "Blackberry20and4",
    "Вишня03",
    "Вишня02",
    NULL,
    "",
    "Вишня 04",
    "Вишня 2",
    "Вишня 15",
    "Вишня03",
    "Вишня03",
    "Вишня03",
  };
  names[1] = g_strdup(names[1]); //< Ага, чтобы хоть у одной из одинаковых строк был уникальный адрес.

  int names_length = G_N_ELEMENTS(names);

  g_message("Starting filenames_sort test with array of %d elements:", names_length);

  gp_filenames_sort(names, names_length, TRUE);

  g_message("Sorted array:");
  gint i;
  for(i = 0; i < names_length; i++)
    g_message("%s at [%p]", names[i], names[i]);

  return 0;
}

