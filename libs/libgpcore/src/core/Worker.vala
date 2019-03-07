/*
 * GpWorker is a async tasks library.
 *
 * Copyright (C) 2015 Sergey Volkhin.
 *
 * GpWorker is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpWorker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpWorker. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using GLib;

namespace Gp
{
  #if VALA_0_18
  /**
   * Интерфейс для создания на его базе объектов (как главный пример, заданий для GpDispatcher или
   * GThreadPool), выполняющих некую работу в абстрактном методе run.
   *
   * Почему интерфейс удобен для создания заданий для GThreadPool:
   * Задание можно запустить и сразу уменьшить счетчик ссылок одной функцией run_and_unref()
   * (имеет смысл эту функцию устанавливать в качестве коллбека func при создании GThreadPool).
   * Пример:
   * {{{
   * var pool = new ThreadPool<Worker>.with_owned_data(Worker.run_and_unref, 3, false);
   * }}}
   * ''Вышеописанный подход реализован в классе GpDispathcerThreadPool''.
   * Если все задания в программе будут наследоваться от Gp.Worker,
   * то можно вообще говоря обойтись одним пулом (типа GpDispatcher или GThreadPool).
   */
  public interface Worker : Object
  {
    /**
     * Абстрактный метод, выполняющий некое задание.
     */
    public abstract void run();

    /**
     * Метод получения приоритета объекта Worker.
     * По умолчанию -- GLib.Priority.DEFAULT.
     * Returns: значение приоритета, на базе GLib.Priority.
     */
    public virtual int get_priority()
    {
      return GLib.Priority.DEFAULT;
    }


    /**
     * Функция вызывает метод run() объекта worker,
     * после чего уменьшает счетчик ссылок объекта worker на единицу.
     * Эту функцию удобно использовать как коллбек func при создании GThreadPool,
     * таким образом можно будет создавать объекты Worker, класть их в GThreadPool
     * и больше не заботится об их освобождении.
     * @param worker объект типа GpWorker, у которого последовательно будут вызваны run() и unref().
     */
    public static void run_and_unref(owned Worker worker)
    {
      worker.run();
    }

    /**
     * Функция сравнения приоритетов двух объектов GpWorker, совместима с GCompareDataFunc.
     * @param a первый объект с интерфейсом GpWorker, для сравнения.
     * @param b второй объект с интерфейсом GpWorker, для сравнения.
     * @return -1, если приоритет a < b ; 0, если приоритеты равны; 1, если приоритет a > b.
     */
    public static int compare_priorities(Worker a, Worker b)
    {
      int pa = a.get_priority();
      int pb = b.get_priority();

      if(pa > pb) //< Здесь именно '>', т.к., например, приоритет 100 выше приоритета 200.
        return -1;
      else
        return (pa == pb) ? 0 : 1;
    }
  }
  #endif
}

