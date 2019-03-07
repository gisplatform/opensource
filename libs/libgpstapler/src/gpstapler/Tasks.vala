/*
 * Libgpstapler is a graphical tiles manipulation library.
 *
 * Copyright 2013 Sergey Volkhin.
 *
 * This file is part of Libgpstapler.
 *
 * Libgpstapler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Libgpstapler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Libgpstapler. If not, see <http://www.gnu.org/licenses/>.
 *
*/

namespace Gp
{
  /**
   * Абстрактный класс с заданием AsyncTiler'у.
   */
  internal abstract class AsyncTilerTask : GLib.Object
  {
    /**
    * GpAsyncTiler, который создал задание.
    */
    public Gp.AsyncTiler tiler { construct; get; }

    /**
    * Конструктор AsyncTilerTask.
    * @param tiler объект GpAsyncTiler
    */
    AsyncTilerTask(Gp.AsyncTiler tiler)
    {
      Object(tiler: tiler);
    }
  }


  /**
  * Задание-запрос плитки по ее описанию.
  */
  internal class AsyncTilerTaskGetTile : AsyncTilerTask, Gp.Worker
  {
    /**
    * Плитка, которую необходимо сгенерировать.
    */
    public Gp.Tile tile { construct; get; }

    /**
    * Создает объект AsyncTilerTaskGetTile.
    *
    * @param tiler объект GpAsyncTiler
    * @param tile описание плитки
    */
    public AsyncTilerTaskGetTile(Gp.AsyncTiler tiler, Gp.Tile tile)
    {
      Object(tiler: tiler, tile: tile);
    }

    /**
    * Обработчик метода задания run. Реальную работу сделает sync-метод GpAsyncTiler.
    */
    public void run()
    {
      this.tiler.sync_get_tile(this);
    }
  }


  /**
  * Задание на обновление данных.
  */
  internal class AsyncTilerTaskUpdate : AsyncTilerTask, Gp.Worker
  {
    /**
    * Флаг, что следует достать из БД все данные заново, а не только новые.
    */
    public bool update_all { construct; get; }

    /**
    * Создает объект AsyncTilerTaskUpdate.
    * @param tiler объект GpAsyncTiler
    * @param update_all флаг, что следует достать из БД все данные заново, а не только новые
    */
    public AsyncTilerTaskUpdate(Gp.AsyncTiler *tiler, bool update_all)
    {
      Object(tiler: tiler, update_all: update_all);
    }

    /**
    * Обработчик метода задания run. Реальную работу сделает sync-метод GpAsyncTiler.
    */
    public void run()
    {
      this.tiler.sync_update_data(this);
    }
  }


  /**
  * Задание на запись данных.
  */
  internal class AsyncTilerTaskWrite : AsyncTilerTask, Gp.Worker
  {
    /**
    * Данные пользовательских настроек.
    */
    public Object user_data { construct; get; }

    /**
    * Некий формат данных, определенный в реализации GpAsyncTiler.
    */
    public int format { construct; get; }

    /**
     * Признак уже произведенного запуска задачи sync_write_data_to_stream() --
     * immut не надо обновлять, надо использовать уже сохраненный.
     */
    public bool is_run { private set; public get; default = false; }

    private Object? immut;

    /**
    * Создает объект AsyncTilerTaskWrite.
    * @param tiler объект GpAsyncTiler
    * @param user_data Объект с данными пользовательских настроек.
    * @param format Некий формат данных, определенный в реализации GpAsyncTiler.
    */
    public AsyncTilerTaskWrite(Gp.AsyncTiler tiler, Object user_data, int format)
    {
      Object(tiler: tiler, user_data:user_data, format:format);
    }

    /**
    * Обработчик метода задания run. Реальную работу сделает sync-метод GpAsyncTiler.
    */
    public void run()
    {
      if(is_run == false)
        this.immut = tiler.get_current_immut();

      tiler.sync_write_data_to_stream(this, immut);

      is_run = true;
    }
  }
}
