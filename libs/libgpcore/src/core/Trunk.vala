/*
 * GpTrunk is a thread-safe objects storage library.
 *
 * Copyright (C) 2016 Sergey Volkhin.
 *
 * GpTrunk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpTrunk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpTrunk. If not, see <http://www.gnu.org/licenses/>.
 *
*/

namespace Gp
{
  /**
   * Неупорядоченное thread-safe хранилище объектов (не обязательно GObject).
   * Предназначено для хранения объектов - временных буферов, в том числе в потоках,
   * например, чтобы не создавать временные буферы внутри каждого нового задания Gp.Worker.
   *
   * Объект Trunk не имеет методов типа push/pop для объектов.
   * Вместо этого нужно вызвать метод access и передать ему указатель на пустой объект GpTrunkGuard.
   * По указателю на GpTrunkGuard будет записан новый объект GpTrunkGuard.
   *
   * Метод access возвращает unowned ссылку на объект.
   * Ссылка действительна на время жизни самого GpTrunkGuard!
   *
   * В качестве параметра в access передается "size" -- минимальный размер запрашиваемого объекта.
   * В случае, если объекты все одинакового размера, можно не использовать этот параметр (передавать 0).
   *
   * При вызове access с пустым GpTrunkGuard: если объект с размером не меньше запрашиваемого
   * есть в хранилище, он возьмется из хранилища.  В противном случае новый объект
   * запрашиваемого размера будет создан с помощью фукнции "creator".
   *
   * Объект GpTrunkGuard хранит указатель на вытащенный им из хранилища объект.
   * При вызове access с непустыми GpTrunkGuard:
   * метод повторно вернет unowned ссылку на хранимый в указанном GpTrunkGuard объект.
   *
   * Деструктор GpTrunkGuard автоматически вернет объект в хранилище.
   *
   * Интерфейс GpTrunk/GpTrunkGuard сделан по аналогии с Mutex/MutexGuard в Rust.
   * Также используется паттерн RAII.
   * Но из-за того, что в Vala нет некоторых вещей, которые есть в Rust'е,
   * программисту следует самостоятельно следить за тем,
   * что обращение по unowned ссылке, возвращенной access, не выходит за рамки
   * области существования GpTrunkGuard.
   *
   * Хорошим правилом будет обращаться к объекту в GpTrunkGuard либо только через access,
   * либо объявлять указатель на объект сразу после объявления GpTrunkGuard, вроде:
   * {{{
   * Gp.Trunk.Guard guard = null;
   * unowned SomeObj some_obj = trunk.access(ref guard, size);
   * ...
   * }}}
   */
  public class Trunk<T> : Object
  {
    /**
     * Класс для оперирования объектами из хранилища.
     * Объекты Guard -- не thread-safe!
     * Деструктор класса автоматически возвращает объект в хранилище.
    */
    public class Guard<T> : Object
    {
      private Trunk<T> trunk;
      internal Box<T> box;

      internal Guard(Trunk<T> trunk, uint size)
      {
        this.trunk = trunk;
        this.box = trunk.pop(size);
      }

      /**
       * Деструктор GpTrunkGuard вернет объект в хранилище GpTrunk.
      */
      ~Guard()
      {
        this.trunk.push((owned)this.box);
      }
    }

    /**
    * Внутренний контейнер, чтобы хранить размер объекта вместе с объектом в очереди.
    */
    internal class Box<T> : Object
    {
      public T object;
      public uint size;

      public Box(owned T object, uint size)
      {
        this.object = (owned)object;
        this.size = size;
      }
    }

    /**
    * Метод создания объектов в GpTrunk.
    */
    private TrunkObjectCreateFunc<T> creator;

    /**
    * Контейнер, в котором хранятся объекты.
    */
    private AsyncQueue<Box<T>>queue = new AsyncQueue<Box<T>>();

    /**
     * Конструктор GpTrunk.
     * @param creator Метод создания объектов в GpTrunk.
    */
    public Trunk(owned TrunkObjectCreateFunc<T> creator)
    {
      this.creator = (owned)creator;
    }

    #if !VALA_0_34
    // На старых версия Vala (как минимум, на Wheezy) очереди приходится чистить вручную,
    // иначе получим утечку памяти. В то же время на Stretch с Vala 0.34 (проверено) все ок.
    ~Trunk()
    {
      while(queue.try_pop() != null)
        ;
    }
    #endif

    /**
     * Метод получения объекта из GpTrunk.
     *
     * @param guard Если guard != null, то объект будет взят из guard,
     * иначе будет создан новый Trunk.Guard и записан по указателю guard.
     * @param size Требуемый минимальный размер объекта.
     * Следует особо подчеркнуть, что access может вернуть и объект большего размера.
     * @return Ссылка на объект, которая действительна на время жизни guard!
    */
    public unowned T access(ref Trunk.Guard<T>? guard, uint size = 0)
    {
      if(guard == null)
        guard = new Trunk.Guard<T>(this, size);

      return guard.box.object;
    }

    /**
     * Функция получения количества объектов в хранилище.
     * @return Количество объектов в хранилище.
    */
    public int length()
    {
      return queue.length();
    }

    /**
     * Достать объект из хранилища.
     * @param size Минимальный размер объекта.
     * @return Объект (из хранилища или вновь созданный, в контейнере).
     */
    private Box<T> pop(uint size)
    {
      Box<T>? box = null;

      do
      {
        box = queue.try_pop();
      }
      while(box != null && box.size < size);

      if(box == null)
        box = new Box<T>(creator(size), size);

      return box;
    }

    /**
     * Положить объект в хранилище.
     * @param box Объект (в контейнере), который будет положен в хранилище.
     */
    private void push(owned Box<T> box)
    {
      queue.push((owned)box);
    }
  }

  /**
  * Функция создания объектов, которые предполагается хранить в GpTrunk.
  * @param size Размер объекта, который должен быть создан.
  */
  public delegate G TrunkObjectCreateFunc<G>(uint size);
}
