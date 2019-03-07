/*
 * GpMemTile is a storage object library for GpTile.
 *
 * Copyright (C) 2015 Sergey Volkhin.
 *
 * GpMemTile is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpMemTile is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpMemTile. If not, see <http://www.gnu.org/licenses/>.
 *
*/

namespace Gp
{
  /**
  * Объект для хранения всей информации об отрисованной плитке:
  * описания плитки, статуса отрисовки и отрисованных данных в памяти.
  *
  * Гарантируется, что память со всеми данными о плитке выделена с помощью GLib.malloc единым массивом.
  * Это сделано для возможности взаимодействия со сторонними библиотеками,
  * главным образом Си-шными, которые на вход принимают данные,
  * обязательно выделенные с помощью GLib.malloc (например, библиотека GSmartCache).
  * Если способ хранения данных не принципиален,  можно спокойно пользоваться объектом MemTile
  * для хранения всей информации об отрисованной плитке, не думая о его внутренней структуре.
  *
  * Объект хранит в памяти, выделенной с помощью GLib.malloc, следующие данные, подряд:
  *
  * || ''Данные'' || ''Формат'' || ''Размер в байтах'' ||
  * || Описание плитки || Gp.Tile || sizeof(Gp.Tile) ||
  * || Статус отрисовки || Gp.TileStatus || sizeof(Gp.TileStatus) ||
  * || Отрисованные данные || Cairo.Format.ARGB32 || Gp.TILE_DATA_SIZE ||
  *
  * Для манипуляции данными в вышеописанном формате даже в отсутвии объекта MemTile служит ряд
  * статических методов: "get_status(tile)_from_malloc_data" и "set_status(tile)_to_malloc_data".
  */
  public class MemTile : Object
  {
    /**
    * Данные, выделенные с помощью GLib.malloc памяти.
    */
    private uint8[] ptr = new uint8[MemTile.N_BYTES];

    /**
    * Размер выделенной с помощью GLib.malloc памяти.
    */
    public const ulong N_BYTES = sizeof(Gp.Tile) + sizeof(TileStatus) + Gp.TILE_DATA_SIZE;

    /**
    * Описание содержащейся в объекте плитки.
    */
    public Gp.Tile tile
    {
      get { return Gp.MemTile.get_tile_from_malloc_data(this.ptr);      }
      set {        Gp.MemTile.set_tile_to_malloc_data(this.ptr, value); }
    }

    /**
    * Статус отрисовки содержащейся в объекте плитки.
    */
    public Gp.TileStatus status
    {
      get { return Gp.MemTile.get_status_from_malloc_data(this.ptr); }
      set {        Gp.MemTile.set_status_to_malloc_data(this.ptr, value); }
    }

    /**
    * Метод позволяет получить указатель на данные, выделенные с помощью GLib.malloc.
    */
    public unowned uint8[] get_malloc_data()
    {
      return this.ptr;
    }

    /**
    * Метод позволяет получить указатель на отрисованные данные плитки (например, в формате ARGB32).
    */
    [CCode (array_length = false)]
    public unowned uint8[] get_buf()
    {
      return this.ptr[sizeof(Gp.Tile) + sizeof(Gp.TileStatus)
                     :sizeof(Gp.Tile) + sizeof(Gp.TileStatus) + Gp.TILE_DATA_SIZE];
    }

    /*
    * Метод создания объекта MemTile с инициализацией полей.
    *
    * @param tile Описание плитки.
    * @param status Статус.
    */
    public MemTile.with_tile(Tile tile, TileStatus status = TileStatus.NOT_INIT)
    {
      Object(tile: tile, status: status);
    }

    // Статические функции -->

    /**
    * Функция позволяет взять из объекта Gp.MemTile данные, выделенные с помощью GLib.malloc.
    * При этом в объекте не остается указателя на данные.
    * ''После вызова этой функции, нельзя вызывать другие методы объекта, его можно и нужно удалить.''
    *
    * @param mem_tile Объект типа MemTile
    * @return Данные из объекта
    */
    public static uint8[] free_to_malloc_data(MemTile mem_tile)
    {
      return (owned)mem_tile.ptr;
    }

    /**
    * Функция достает из памяти, выделенной с помощью GLib.malloc, статуса отрисовки плитки.
    *
    * @param malloc_data Память, содержание памяти описано в документации по "Gp.MemTile".
    */
    public static Gp.TileStatus get_status_from_malloc_data(uint8[] malloc_data)
      requires(malloc_data.length >= sizeof(Gp.Tile) + sizeof(Gp.TileStatus))
    {
      return *(Gp.TileStatus*)((uint8*)malloc_data + sizeof(Gp.Tile));
    }

    /**
    * Функция записывает в память, выделенную с помощью GLib.malloc, статус отрисовки плитки.
    *
    * @param malloc_data Память, содержание памяти описано в документации по "Gp.MemTile".
    * @param new_status Статус, который следует записать в память.
    *
    * @return true в случае успеха, false в случае ошибки.
    */
    public static bool set_status_to_malloc_data(uint8[] malloc_data, Gp.TileStatus new_status)
      requires(malloc_data.length >= sizeof(Gp.Tile) + sizeof(Gp.TileStatus))
    {
      *(Gp.TileStatus*)((uint8*)malloc_data + sizeof(Gp.Tile)) = new_status;
      return true;
    }

    /**
    * Функция достает из памяти, выделенной с помощью GLib.malloc, описание плитки.
    *
    * @param malloc_data Память, содержание памяти описано в документации по "Gp.MemTile".
    */
    public static Gp.Tile get_tile_from_malloc_data(uint8[] malloc_data)
      requires(malloc_data.length >= sizeof(Gp.Tile))
    {
      return *(Gp.Tile*)malloc_data;
    }

    /**
    * Функция записывает в память, выделенную с помощью GLib.malloc, описание плитки.
    *
    * @param malloc_data Память, содержание памяти описано в документации по "Gp.MemTile".
    * @param tile Описание плитки, которыое следует записать в память.
    *
    * @return true в случае успеха, false в случае ошибки.
    */
    public static bool set_tile_to_malloc_data(uint8[] malloc_data, Gp.Tile tile)
      requires(malloc_data.length >= sizeof(Gp.Tile))
    {
      *(Gp.Tile*)malloc_data = tile;
      return true;
    }

    // Статические функции <--
  }
}

