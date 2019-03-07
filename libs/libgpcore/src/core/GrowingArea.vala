/*
 * GpGrowingArea is a thread-safe library for rectangle area representation.
 *
 * Copyright (C) 2015 Sergey Volkhin.
 *
 * GpGrowingArea is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpGrowingArea is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpGrowingArea. If not, see <http://www.gnu.org/licenses/>.
 *
*/

namespace Gp
{
  /**
  * Объект содержит значения границ прямоугольной области и мьютекс для их защиты.
  *
  * Пользователю доступны 4 свойства, все они защищаются одним мьютексом.
  * Метод "get" позволяет получить несколько свойств скопом (метод также защищен мьютексом внутри).
  * Границы умеют только расти: попытка установки значения свойства,
  * которое может уменьшить границы, будет проигнорирована.
  */
  public class GrowingArea : Object
  {
    Mutex mutex = Mutex();

    private double _min_x = double.MAX;
    /**
    * Граница по оси x слева.
    */
    public double min_x
    {
      set
      {
        this.mutex.lock();
          this._min_x = double.min(value, this._min_x);
        this.mutex.unlock();
      }
      get
      {
        this.mutex.lock();
          double rval = this._min_x;
        this.mutex.unlock();
        return rval;
      }
    }

    private double _max_x = double.MIN;
    /**
    * Граница по оси x справа.
    */
    public double max_x
    {
      set
      {
        this.mutex.lock();
          this._max_x = double.max(value, this._max_x);
        this.mutex.unlock();
      }
      get
      {
        this.mutex.lock();
          double rval = this._max_x;
        this.mutex.unlock();
        return rval;
      }
    }

    private double _min_y = double.MAX;
    /**
    * Граница по оси y снизу.
    */
    public double min_y
    {
      set
      {
        this.mutex.lock();
          this._min_y = double.min(value, this._min_y);
        this.mutex.unlock();
      }
      get
      {
        this.mutex.lock();
          double rval = this._min_y;
        this.mutex.unlock();
        return rval;
      }
    }

    private double _max_y = double.MIN;
    /**
    * Граница по оси y сверху.
    */
    public double max_y
    {
      set
      {
        this.mutex.lock();
          this._max_y = double.max(value, this._max_y);
        this.mutex.unlock();
      }
      get
      {
        this.mutex.lock();
          double rval = this._max_y;
        this.mutex.unlock();
        return rval;
      }
    }

    /**
    * Метод для получения границы области.
    * @param min_x граница по оси x слева;
    * @param max_x граница по оси x справа;
    * @param min_y граница по оси y снизу;
    * @param max_y граница по оси y сверху.
    */
    public void get_all(out double min_x, out double max_x, out double min_y, out double max_y)
    {
      this.mutex.lock();
        min_x = this._min_x;
        max_x = this._max_x;
        min_y = this._min_y;
        max_y = this._max_y;
      this.mutex.unlock();
    }
  }
}

