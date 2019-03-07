/*
 * GpTile is a grapical tiles manipulation library.
 *
 * Copyright (C) 2015 Sergey Volkhin.
 *
 * GpTile is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpTile is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpTile. If not, see <http://www.gnu.org/licenses/>.
 *
*/

namespace Gp
{
  /**
  * Фиксированный размер стороны "плитки" в пикселях.
  */
  public const int TILE_SIDE = 256;

  /**
  * Шаг в байтах между рядами "плитки".
  */
  public const int TILE_STRIDE = 4 * TILE_SIDE;

  /**
  * Фиксированный размер данных плитки в байтах.
  */
  public const int TILE_DATA_SIZE = 4 * TILE_SIDE * TILE_SIDE;

  /**
  * Делитель для получения условных единиц.
  */
  public const double TILE_DIVIDER_INDICES = 10;
  /**
  * Делитель для получения значения в метрах.
  */
  public const double TILE_DIVIDER_METERS = 100;

  /**
  * Статус отрисовки плитки.
  */
  public enum TileStatus
  {
    /**
    * Плитка даже не инициализирована (такую плитку использовать никак нельзя).
    */
    NOT_INIT = -1,
    /**
    * Плитка инициализирована (например, -- изображением-заглушкой),
    * но на ней не отрисовано никаких данных.
    */
    INIT = 0,
    /**
    * Плитка отрисована, но содержит неактуальные данные.
    * Это же справедливо для любых значений от NOT_ACTUAL до ACTUAL.
    */
    NOT_ACTUAL,
    /**
    * Плитка отрисована и содержит актуальные данные.
    */
    ACTUAL = 1000
  }

  /**
  * Структура позволяет кратко и однозначно описать плитку.
  *
  * Например,(но не обязательно) плитку для карты,
  * похожей на GoogleMaps или OpenStreetMap.
  */
  public struct Tile
  {
    /**
    * Координата плитки по оси X (размерность -- сторона плитки).
    */
    public int x;
    /**
    * Координата плитки по оси Y (размерность -- сторона плитки).
    */
    public int y;
    /**
    * Размер (в сантиметрах) стороны плитки.
    */
    public uint l;
    /**
    * Тип, к которому относится плитка.
    *
    * Тип -- это по сути некий идентификатор типа int,
    * по которому можно отличать одинаковые по координатам,
    * но разные по сути плитки (например, которые рисуют разные слои/данные).
    * Обычно типу есть смысл поставить в соответствие некий Enum.
    */
    public int type;

    /**
    * Функция позволяет сравнить два объекта типа GpTile.
    * Плитки сравниваются по координатам, масштабу и типу.
    *
    * Т.е. проверяется полностью ли идентичны два объекта.
    *
    * //Функция совместима с GLib.EqualFunc<Gp.Tile?>.//
    *
    * @param tile1 Первая плитка для сравнения
    * @param tile2 Вторая плитка для сравнения
    *
    * @return true если объекты равны, false - если объекты не равны или если хотя бы один из аргументов == null.
    */
    public static bool equal_all(Tile? tile1, Tile? tile2)
    {
      return (
        tile1 != null &&
        tile2 != null &&
        tile1.x == tile2.x &&
        tile1.y == tile2.y &&
        tile1.l == tile2.l &&
        tile1.type == tile2.type
      ) ? true : false;
    }

    /**
    * Функция позволяет сравнить два объекта типа GpTile.
    * Плитки сравниваются только по координатам и масштабу,
    * тип не учитывается.
    *
    * //Функция совместима с GLib.EqualFunc<Gp.Tile>.//
    *
    * @param tile1 Первая плитка для сравнения
    * @param tile2 Вторая плитка для сравнения
    *
    * @return true если объекты равны, иначе -- false.
    */
    public static bool equal_pos(Tile tile1, Tile tile2)
    {
      return (tile1.x == tile2.x && tile1.y == tile2.y && tile1.l == tile2.l) ? true : false;
    }

    /**
    * Метод получения индекса плитки (что-то вроде MD5-суммы).
    *
    * Предполагается, что индекс соседних плиток, в т.ч. в разных масштабах,
    * скорее всего будут разными. Поэтому его можно использовать в качестве индекса
    * кэша GpSmartCache (конечно, не забывая проверять на возможные коллизии).
    *
    * Индекс генерируется на основе значений координат и масштаба,
    * тип не учитывается.
    *
    * //Функция совместима с GHashFunc.//
    *
    *  === Детали реализации расчета индекса ===
    *
    * * Сначала кладем в младшие (sizeof(uint) / 4) байт index'а младшие (sizeof(uint) / 4) байт x.
    *
    * * Затем кладем в следующие (sizeof(uint) / 4) байт index'а младшие (sizeof(uint) / 4) байт y.
    *
    * * Затем кладем в оставшиеся (sizeof(uint) / 2) байт index'а xor-сумму младших (sizeof(uint) / 2) байт l и старших (sizeof(uint) / 2) байт l.
    *
    * ==== Или, к примеру для теоретической 64-битной системы типа ILP64/SILP64: ====
    *
    * * Сначала кладем в младшие 2 байта index'а младшие 2 байта x.
    *
    * * Затем кладем в следующие 2 байта index'а младшие 2 байта y.
    *
    * * Затем кладем в оставшиеся 4 байта index'а xor-сумму младших 4 байт l и старших 4-х байт l.
    *
    * @return значение индекса плитки.
    */
    public uint get_index()
    {
      return
        (uint)((this.x & ((1 << (8 * (sizeof(uint) / 4))) - 1))) +
        (uint)((this.y & ((1 << (8 * (sizeof(uint) / 4))) - 1)) << (8 * sizeof(uint) / 4)) +
        (uint)((this.l ^ (this.l >> (8 * sizeof(uint) / 2))) << (8 * sizeof(uint) / 2));
    }

    /**
    * Получение границ плитки в условных единицах (например, индексах из БД).
    *
    * @param x_min Минимальное значение по горизонтальной оси
    * @param x_max Максимальное значение по горизонтальной оси
    * @param y_min Минимальное значение по вертикальной оси
    * @param y_max Максимальное значение по вертикальной оси
    */
    public void get_limits_in_indices(out double x_min, out double x_max, out double y_min, out double y_max)
    {
      x_min = (double)(this.x + 0) * (double)this.l / TILE_DIVIDER_INDICES;
      x_max = (double)(this.x + 1) * (double)this.l / TILE_DIVIDER_INDICES;
      y_min = (double)(this.y + 0) * (double)this.l / TILE_DIVIDER_INDICES;
      y_max = (double)(this.y + 1) * (double)this.l / TILE_DIVIDER_INDICES;
    }

    /**
    * Получение границ плитки в метрах.
    *
    * @param x_min Минимальное значение по горизонтальной оси
    * @param x_max Максимальное значение по горизонтальной оси
    * @param y_min Минимальное значение по вертикальной оси или
    * @param y_max Ммаксимальное значение по вертикальной оси
    */
    public void get_limits_in_meters(out double x_min, out double x_max, out double y_min, out double y_max)
    {
      x_min = (double)(this.x + 0) * (double)this.l / TILE_DIVIDER_METERS;
      x_max = (double)(this.x + 1) * (double)this.l / TILE_DIVIDER_METERS;
      y_min = (double)(this.y + 0) * (double)this.l / TILE_DIVIDER_METERS;
      y_max = (double)(this.y + 1) * (double)this.l / TILE_DIVIDER_METERS;
    }

    /**
    * Конвертирует плитку в текст (например, для отладки).
    */
    public string to_string()
    {
      return "[%4d : %4d] x %4u ; type = %d".printf(x, y, l, type);
    }
  }

  /**
  * Тип функции для доступа к плитке.
  *
  * Вторым аргументом "user_data" в функцию передаются пользовательские данные.
  *
  * @param tile Описание плитки.
  */
  public delegate bool TileAccessFunc(Gp.Tile tile);
}

