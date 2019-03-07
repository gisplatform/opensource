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
 * Асинхронная реализация абстрактного класса, предназначенного  для формирования
 * квадратных изображений ("плиток" Gp.Tile) в дискретных координатах и масштабе.
 *
 * ПО СРАВНЕНИЮ С TILER: уже имеет реализации get_tile и write_data_to_stream,
 * но вместо них добавляет три новых абстрактных метода: immut_generate, tile_generate
 * и  write_data_to_stream_imp.
 *
 * ==== Иммут (нужно знать только для разработки AsyncTiler'ов, не для использования) ====
 *
 * Для хранения данных для отрисовки предназначен так называемый "иммут" ("immut"),
 * неизменяемый объект типа GObject (а чаще -- его потомок).
 * Иммут создается в абстрактном методе AsyncTiler.immut_generate().
 *
 * Для хранения данных выбран тип GObject, во вногом ради того,
 * чтобы иметь возможность передавать массив в задания Диспетчера,
 * в разные потоки, и контролировать количество ссылок на массив
 * с помощью встроенного в GObject механизма подсчета ссылок
 * (тип не выбирается с помощью параметра дженерика,
 * т.к. Vala-дженерики не работают с nullable-объектами,
 * + использование Object в API позволяет сделать рельный тип иммута
 * в реализации AsyncTiler'а internal-типом и вообще никак не светить его в API).
 *
 * Соответственно, иммут не может быть изменен после его создания,
 * т.к. он может использоваться одновременно в разных потоках.
 * В случае необходимости изменения иммута в функции immut_generate()
 * должен быть создан новый объект GObject.
 *
 * Функция immut_generate() возвращает либо NULL,
 * если старый иммут актуален, либо функция возвращает новый иммут
 * в виде вновь созданного объекта GObject.
 *
 * Вообще говоря, главная идея иммута -- во множественности данных, по которым генерируются
 * плитки. Чтобы можно было изменять эти данные (создавая новый иммут, с новой информацией),
 * в то же время не останавливая задание, которое работает по старым данным.
 * См. [[https://en.wikipedia.org/wiki/Immutable_object|статью в википедии]]),
 *
 *
 * ==== Online-обновление ====
 *
 * Если поменялись данные, то вот что делается:
 * # Извне вызывается update_data;
 * # По update_data создается задание, а в задании диспетчера будет вызван immut_generate;
 * # Immut_generate мало того, что должен вернуть новый immut,
 * так еще должен пометить с помощью cache_mark_notactual более неактуальные плитки в кеше
 * (все те, и только те плитки, которые затронуло изменение данных).
 * # По окончании генерации immut'а будет брошен сигнал data_updating,
 * по этому сигналу потребитель плиток заново запрашивает все используемые им плитки.
 */
public abstract class AsyncTiler : Tiler
{
  /**
  * Описания нужных ("желаемых") плиток:
  * стоящих в очереди на генерацию (в диспетчере),
  * сгенерированных (лежащих в done_tiles),
  * генерируемых в данный момент.
  */
  private GenericSet<Gp.Tile?> desired_tiles;
  /**
  * Мьютекс для доступа к this.desired_tiles.
  */
  private Mutex mutex_for_desired_tiles;

  /**
  * Флаг, что задание TaskUpdate уже есть в очереди.
  */
  private int update_processing;

  /**
  * Очередь готовых сгенерированных плиток.
  */
  private AsyncQueue<Gp.MemTile> done_tiles;

  /**
  * Данные для отрисовки.
  */
  private Object? immut;
  /**
  * Идентификатор ("ревизия", т.е. порядковый номер) состояния иммута this.immut.]
  */
  private uint immut_revision;
  /**
  * Мьютекс для доступа к this.immut_revision и указателю this.immut.
  */
  private Mutex immut_mutex;

  // Свойства -->
    /**
    * Диспетчер, обрабатывающий задания AsyncTiler'а (типа AsyncTilerTask).
    */
    public Gp.Dispatcher dispatcher { get; construct; }
  // Свойства <--

  // Сигналы -->
    /**
    * Сигнализирует о ходе процесса обновления данных (генерации нового immut'а).
    * @param fraction Доля законченности обновления от 0 до 1.
    */
    public signal void data_updating(double fraction);
    /**
    * Сигнализирует о завершение записи данных в поток (IOStream).
    */
    public signal void data_written();
    /**
    * Сигнализирует о ходе процесса записи данных в поток (IOStream).
    * @param fraction Доля законченности записи от 0 до 1.
    */
    public signal void data_writing(double fraction);
  // Сигналы <--

  // Абстрактные и виртуальные методы -->
    /**
    * Метод, в котором происходит (пере)создание иммута (неизменяемого объекта с данными для отрисовки).
    *
    * Инициировать вызов метода в потоке можно вызовом метода Gp.AsyncTiler.update_data().
    *
    * @param old_immut текущие (старые) данные для отрисовки, либо null
    *
    * @return новый объект с данными для отрисовки, либо null
    */
    protected abstract Object? immut_generate(Object? old_immut);
    /**
    * Метод, в котором происходит непосредственная генерация плитки.
    *
    * @param immut указатель на данные для отрисовки.
    *
    * @param tile Объект с описанием плитки, которую нужно сгенерировать.
    * @param tile Буфер, куда будут записаны сгенерированные ARGB32-данные.
    */
    protected abstract void tile_generate(Object immut, Gp.Tile tile, [CCode (array_length = false)] uint8[] buf);
    /**
    * Метод, в котором происходит запись данных в поток.
    *
    * @param immut Данные для записи
    * @param user_data Объект с данными пользовательских настроек.
    * @param format Формат данных, определенный в реализации GpAsyncTiler.
    */
    protected virtual bool write_data_to_stream_imp(Object? immut, Object user_data, int format) { return true; }
  // Абстрактные и виртуальные методы <--

  construct
  {
    this.done_tiles = new AsyncQueue<Gp.MemTile>();

    this.desired_tiles = new GenericSet<Gp.Tile?>(Gp.Tile.get_index, Gp.Tile.equal_all);
    this.mutex_for_desired_tiles = Mutex();

    this.immut_mutex = Mutex();
  }


  ~AsyncTiler()
  {
    this.drop_tasks();
    this.drop_done_tiles_from_queue(); //< На старых версия Vala (как минимум, на Wheezy) очереди приходится чистить вручную, иначе получим утечку памяти.
  }


  /**
  * Метод очищает очередь сформированных плиток.
  */
  private void drop_done_tiles_from_queue()
  {
    while(this.done_tiles.try_pop() != null)
      ;
  }


  /**
  * Инициирует сигнал с информацией о ходе процесса обновления данных.
  * Метод thread-safe. Предназначен для использования в AsyncTiler.immut_generate().
  * @param fraction Доля законченности обновления от 0 до 1.
  */
  protected void data_updating_emit(double fraction)
  {
    Idle.add(() =>
    {
      this.data_updating(fraction);
      return Source.REMOVE;
    });
  }


  /**
  * Инициирует сигнал с информацией о ходе процесса записи данных.
  * Метод thread-safe. Предназначен для использования в AsyncTiler.write_data_to_stream_imp().
  * @param fraction Доля законченности записи от 0 до 1.
  */
  protected void data_writing_emit(double fraction)
  {
    Idle.add(() =>
    {
      this.data_writing(fraction);
      return Source.REMOVE;
    });
  }


  /**
  * Метод очищает очередь заданий на формирование плиток.
  */
  public override void drop_tasks()
  {
    this.mutex_for_desired_tiles.lock();
      this.desired_tiles.remove_all();
    this.mutex_for_desired_tiles.unlock();
  }



  /**
  * Метод позволяет получить последний сгенерированный иммут (если таковой вообще есть).
  */
  public Object? get_current_immut()
  {
    this.immut_mutex.lock();
      var immut = this.immut;
    this.immut_mutex.unlock();

    return immut;
  }


  /**
  * Метод позволяет получить количество еще не обработанных заданий на формирование плиток.
  * @return количество заданий в очереди.
  */
  public override uint get_tasks_num()
  {
    uint rval;

    this.mutex_for_desired_tiles.lock();
      rval = this.desired_tiles.length;
    this.mutex_for_desired_tiles.unlock();

    return rval;
  }


  /**
  * Метод получения плитки из очереди this.done_tiles.
  * Может вернуть и плитку, отличную от required_tile.
  *
  * @param required_tile требуемая плитка.
  * @param status Требуется плитка со статусом отрисовки более указанного.
  *
  * @return Данные плитки (актуальные или нет), либо null, если данных для плитки пока еще нет.
  */
  protected override Gp.MemTile? get_tile_from_source(Gp.Tile required_tile, Gp.TileStatus status)
  {
    Gp.MemTile? mem_tile_from_queue;

    while(( mem_tile_from_queue = this.done_tiles.try_pop() ) != null)
    {
      this.mutex_for_desired_tiles.lock();
        this.desired_tiles.remove(mem_tile_from_queue.tile);
      this.mutex_for_desired_tiles.unlock();

      return_val_if_fail(mem_tile_from_queue.tile.type < this.tile_types_num, null);

      return mem_tile_from_queue;
    }

    this.mutex_for_desired_tiles.lock();
    if(this.desired_tiles.contains(required_tile) == false)
    {
      this.desired_tiles.add(required_tile);

      this.mutex_for_desired_tiles.unlock();

      try
      {
        this.dispatcher.add(new AsyncTilerTaskGetTile(this, required_tile));
      }
      catch(ThreadError e)
      {
        critical("failed to add AsyncTilerTaskGetTile to dispatcher");
      }
    }
    else
      this.mutex_for_desired_tiles.unlock();

    return null;
  }


  /**
  * Метод для форсирования обновления данных.
  *
  * Например, в БД (возможно) обновились данные и, соответственно,
  * нужно проверить на сколько актуальнен иммут, и, соответственно,
  * лежащие в кеше плитки, возможно, некоторые стоит перегенерировать.
  *
  * Функция совместима с типом GSourceFunc.
  *
  * @return В случае успеха Source.CONTINUE, в случае ошибки -- Source.REMOVE.
  */
  public override bool update_data()
  {
    try
    {
      if(AtomicInt.compare_and_exchange(ref this.update_processing, 0, 1))
        this.dispatcher.add(new AsyncTilerTaskUpdate(this, false));
      return Source.CONTINUE;
    }
    catch(ThreadError e)
    {
      critical("failed to add AsyncTilerTaskUpdate to dispatcher");
      return Source.REMOVE;
    }
  }


  /**
  * Метод для форсирования обновления всех данных из БД.
  *
  * Не проверяет появились ли новые данные (как AsyncTiler.update_data()),
  * исчезли ли старые, обновляет тупо все заново.
  *
  * Функция совместима с типом GSourceFunc.
  *
  * @return GLib.Source.CONTINUE.
  */
  public bool update_data_all()
  {
    try
    {
      if(AtomicInt.compare_and_exchange(ref this.update_processing, 0, 1))
        this.dispatcher.add(new AsyncTilerTaskUpdate(this, true));
      return Source.CONTINUE;
    }
    catch(ThreadError e)
    {
      critical("failed to add AsyncTilerTaskUpdate (all) to dispatcher");
      return Source.REMOVE;
    }
  }


  /**
  * Функция для записи данных в поток
  *
  * @param user_data Объект с данными пользовательских настроек.
  * @param format Формат данных.
  */
  public override void write_data_to_stream(Object user_data, int format = 0)
  {
    try
    {
      this.dispatcher.add(new AsyncTilerTaskWrite(this, user_data, format));
    }
    catch(ThreadError e)
    {
      critical("failed to add AsyncTilerTaskWrite to dispatcher");
    }
  }


  internal void sync_get_tile(AsyncTilerTaskGetTile task)
  {
    {
      bool desire;

      this.mutex_for_desired_tiles.lock();
        desire = this.desired_tiles.contains(task.tile);
      this.mutex_for_desired_tiles.unlock();

      if(!desire)
        return;
    }

    Gp.MemTile mem_tile = new Gp.MemTile.with_tile(task.tile, TileStatus.ACTUAL);

    uint revision_before_generate = 0;

    this.immut_mutex.lock();
      do
      {
        revision_before_generate = this.immut_revision;

        {
          Object? immut = this.immut;

        this.immut_mutex.unlock();
          if(immut != null)
            this.tile_generate(immut, mem_tile.tile, mem_tile.get_buf());
        }

        this.immut_mutex.lock();
      }
      while(unlikely(revision_before_generate != this.immut_revision));
    this.immut_mutex.unlock();

    this.done_tiles.push(mem_tile);
  }


  internal void sync_update_data(AsyncTilerTaskUpdate task)
  {
    AtomicInt.set(ref this.update_processing, 0);

    this.immut_mutex.lock();
    {
      Object? old_immut = this.immut;
      Object? new_immut = this.immut_generate(task.update_all ? null : old_immut);
      // <-- FIXME разве под локом должен иммут генериться?

      if(new_immut != null)
      {
        this.immut = new_immut;
        this.immut_revision++;

        this.immut_mutex.unlock();
      }
      else
        this.immut_mutex.unlock();
    }

    Idle.add(() =>
    {
      this.data_updated();
      return Source.REMOVE;
    });
  }


  internal void sync_write_data_to_stream(AsyncTilerTaskWrite task, Object? immut)
  {
    if(this.write_data_to_stream_imp(immut, task.user_data, task.format) == true)
    {
      Idle.add(() =>
      {
        this.data_written();
        return Source.REMOVE;
      });
    }
    else
    {
      Timeout.add(100, () =>
      {
        try
        {
          this.dispatcher.add(task);
        }
        catch(ThreadError e)
        {
          critical("failed to re-add AsyncTilerTaskWrite to dispatcher");
        }
        return false;
      });
    }
  }

} //< AsyncTiler
} //< Gp
