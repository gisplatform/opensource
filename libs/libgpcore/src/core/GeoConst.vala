/*
 * GpGeoConst is a library with geographical constants.
 *
 * Copyright (C) 2015 Gennadiy Nefediev.
 *
 * GpGeoConst is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpGeoConst is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpGeoConst. If not, see <http://www.gnu.org/licenses/>.
 *
*/

namespace Gp
{
  /**
  * Число угловых секунд в 1 радиане.
  */
  public const double GEO_RAD_TO_SEC = 206264.8062;
  /**
  * Число градусов в 1 радиане.
  */
  public const double GEO_RAD_TO_DEG = 57.29577951308232;
  /**
  * Число радианов в 1 градусе.
  */
  public const double GEO_DEG_TO_RAD = 0.0174532925199432958;
  /**
  * Средний радиус земного шара, м.
  */
  public const double GEO_RE = 6371000;


  // Параметры преобразования координат UTM -->
    /**
    * Северное применение UTM (до 84 гр. с.ш.)
    */
    public const double UTM_NORTH = 84.0;
    /**
    * Южное применение UTM (до 80 гр. ю.ш.)
    */
    public const double UTM_SOUTH = 80.0;
    /**
    * Масштаб на осевом меридиане в проекции UTM.
    */
    public const double UTM_M0 = 0.9996;
    /**
    * Северное положение экватора для ЮП в UTM (False Northing).
    */
    public const double UTM_N0 = 10000000;
    /**
    * Восточное положение осевого меридиана в UTM (False Easting).
    */
    public const double UTM_L0 = 500000;
    /**
    * Параметр для добавления номера зоны в UTM.
    */
    public const double UTM_Z0 = 1000000;
  // Параметры преобразования координат UTM <--
}
