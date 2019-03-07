/*
 * GpDispatcher is an async tasks library.
 *
 * Copyright (C) 2015 Sergey Volkhin, Andrey Vodilov,
 * Gennadiy Nefediev, Alexey Pankratov, Maria Pavlova.
 *
 * GpDispatcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpDispatcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpDispatcher. If not, see <http://www.gnu.org/licenses/>.
 *
*/

namespace Gp
{
  #if VALA_0_18
  /**
   * Абстрактный диспетчер, выполняющий задания типа GpWorker.
   */
  public abstract class Dispatcher : Object
  {
    /**
     * Добавление заданий в диспетчер.
     * @param worker задание, которое будет добавлено в диспетчер.
     */
    public abstract void add(Gp.Worker worker) throws ThreadError;
  }

  /**
   * Диспетчер на основе GThreadPool, обрабатывающий задачи типа GpWorker в потоках.
   */
  public class DispatcherThreadPool : Dispatcher
  {
    /**
    * Объект GThreadPool, на основе которого реализован DispatcherThreadPool.
    */
    public ThreadPool<Gp.Worker> pool;

    /**
     * Добавление задания в диспетчер.
     * @param worker задание, которое будет добавлено в диспетчер.
     */
    public override void add(Gp.Worker worker) throws ThreadError
    {
        this.pool.add(worker);
    }

    /**
     * Создание диспетчера GpDispatcherThreadPool.
     * @param max_threads максимальное количество тредов, параметр передается в конструктор pool.
     * @param exclusive экслюзивный ли пул, параметр передается в конструктор pool.
     */
    public DispatcherThreadPool(int max_threads, bool exclusive) throws ThreadError
    {
        this.pool = new ThreadPool<Gp.Worker>.with_owned_data(Worker.run_and_unref, max_threads, exclusive);
    }
  }

  /**
   * Диспетчер обрабатывающий задачи синхронного.
   *
   * Нужен преимущественно для отладки пользователям GpDispatcher.
   */
  public class DispatcherLoop : Dispatcher
  {
    /**
     * Добавление задания в диспетчер.
     * @param worker задание.
     */
    public override void add(Gp.Worker worker)
    {
      worker.run();
    }
  }
  #endif
}
