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

public enum TilerTreeModelCols /// Список колонок, которые GpStapler предоставляет через интерфейс GtkTreeModel.
{
  TILER,        ///Указатель на объект Tiler.
  NAME,         ///Имя строки.
  VISIBLE,      ///Флаг видимости.
  HIGHLIGHT,    ///Флаг подсвечивания.
  UPD_TIMEOUT,  ///Интервал автоматического обновления данных (0, если автоматическое обновление не запущено).
  DELTA_X,      ///Сдвиг слоя по оси X в метрах относительно информации из IcaState.
  DELTA_Y,      ///Сдвиг слоя по оси Y в метрах относительно информации из IcaState.

  USER_OBJ_1,   ///Пользовательский GObject #1.
  USER_OBJ_2,   ///Пользовательский GObject #2.
  USER_OBJ_3,   ///Пользовательский GObject #3.
  USER_OBJ_4,   ///Пользовательский GObject #4.
  USER_OBJ_5,   ///Пользовательский GObject #5.
  USER_OBJ_6,   ///Пользовательский GObject #6.
  USER_OBJ_7,   ///Пользовательский GObject #7.
  USER_OBJ_8,   ///Пользовательский GObject #8.
  USER_OBJ_9,   ///Пользовательский GObject #10.

  NUM           ///Количество колонок.
}

/**
 * Реализация абстрактного класса, предназначенного  для формирования
 * квадратных изображений ("плиток" Gp.Tile) в дискретных координатах и масштабе.
 *
 * Координаты верхнего левого угла квадрата кратны длине его стороны.
 * Предполагается, что в реализации класса пользователь сам определит get_tile_from_source.
 * Если нужно реализовать асинхронный (с потоками) Tiler, то есть смысл наследоваться от AsyncTiler.
 *
 * ==== Типы плиток ====
 *
 * При генерации плиток необходимо указывать не только координаты и масштаб
 * каждой плитки, но и тип, к которой относится плитка.
 * Типы нужны, чтобы иметь возможность генерировать плитки разного вида
 * по одним и тем же координатам.
 * Типы -- это int числа, нумеруются по порядку от 0 до (tile_types_num - 1).
 * GpTiler должен генерировать плитки хотя бы одного типа.
 * Количество типов плиток, генерируемых конкретным GpTiler'ом фиксированное,
 * и хранится в construct-only свойстве tile_types_num.
 *
 * //Класс внутри себя регистрирует каждому типу плитки группу в кеше GpSmartCache.//
 * //Деструктор TileTypes сам делает cache.unreg_group() для зарегистрированных типов плиток.//
 *
 *
 * ==== Запрос плиток ====
 *
 * Запросить плитку можно с помощью функции gp_tiler_get_tile(),
 * указав какая конкретно плитка требуется с помощью объекта Gp.Tile.
 * Функция возвратит статус готовности плитки в виде Gp.TileStatus
 * и запишет изображение плитки в буфер, если доступна какая-то информация для отображения.
 * Если плитка еще не готова, следует запросить ее повторно через некоторое время.
 */
public abstract class Tiler : Object
{
    /**
    * Сигнализирует об изменении данных (необходимо перезапросить плитки).
    */
    public signal void data_updated();

    /**
    * Объект-буфер, чисто чтобы забирать данные из кэша.
    * Используется в одном месте, чтобы получить данные из кэша
    * и сразу вернуть их из метода, уже в виде массива.
    * Создается один раз и живет все время жизни Gp.Tiler.
    */
    private MemTile buf_for_cache_get;

    /**
    * Хранилище групп плиток, соответствующих зарегистрированным типам.
    */
    private uint[] groups;

  // Свойства -->
    /**
    * GpSmartCache для кеширования плиток.
    */
    public Gp.SmartCache cache { get; construct; }

    /**
    * Имя Gp.Tiler'а в виде строки.
    *
    * Предполагается, что имя будет использоваться, например, в графическом интерфейсе
    * для описания Gp.Tiler'а.
    */
    public string name { get; construct; }

    /**
    * Количество типов плиток, с которым работает данные Tiler.
    */
    public int tile_types_num
    {
      construct
      {
        this.groups.resize(value);
      }
      get
      {
        return this.groups.length;
      }
    }

    /**
    * Некий тестовый коэффициент.
    * Смысл коэффициента разный для разных реализаций Gp.Tiler'а.
    */
    public double test_coef { get; set; }
  // Свойства <--

  // Абстрактные и виртуальные методы -->
    /**
    * Метод получения плитки из некоего источника.
    *
    * Вызывается в get_tile, тогда и только тогда, когда не удалось найти запрошенную плитку в кеше.
    *
    * Важно: get_tile_from_source имеет право вернуть и другую плитку, отличную от required_tile.
    * В последнем случае get_tile положет ее в кеш и повторно запросит required_tile.
    * get_tile_from_source имеет право вернуть даже плитку типа, отличного от required_tile.
    *
    * Важно: Может быть указан статус плитки, больше котого требуется плитка. Однако это также является
    * только рекомендацией для возможных оптимизаций, функция имеет право вернуть и плитку меньшего статуса.
    *
    * @param required_tile Требуемая плитка.
    * @param status Требуется плитка со статусом отрисовки более указанного.
    *
    * @return Данные плитки (с произвольным статусом), либо null, если данных для плитки пока еще нет.
    */
    protected abstract Gp.MemTile? get_tile_from_source(Gp.Tile required_tile, Gp.TileStatus status = Gp.TileStatus.NOT_INIT);
    /**
    * Метод проверяет плитки данного типа содержат данные для отрисовки (вроде CAIRO_FORMAT_ARGB32)
    * или некие сырые данные.
    * @param type тип плиток.
    * @return true, если данные готовы к отрисовке; false, если данные сырые.
    */
    public virtual bool is_graphical(int type) { return true; }
    /**
    * Проверяет, может ли Tiler потенциально сформировать плитки данного типа.
    * @param type тип плиток.
    * @return true, если может, false, коли нет.
    */
    public virtual bool provides_tile_type(int type) { return false; }
    /**
    * Функция для записи данных в поток.
    *
    * @param user_data Объект с данными пользовательских настроек.
    * @param format Формат данных (в реализациях может совпадать или нет с типом плиток).
    */
    public virtual void write_data_to_stream(Object user_data, int format = 0) { return; }
    /**
    * Проверяет, может ли Tiler записать в поток данные указанного формата.
    * @param format Формат данных (в реализациях может совпадать или нет с типом плиток).
    * @return true, если может, false, коли нет.
    */
    public virtual bool provides_write_data_format(int format) { return false; }
    /**
    * Метод для получения границы области, в рамках которой у Gp.Tiler'а есть данные для отрисовки.
    * @param type тип плиток.
    * @param from_x граница отображения по оси x слева.
    * @param to_x граница отображения по оси x справа.
    * @param from_y граница отображения по оси y снизу.
    * @param to_y граница отображения по оси y сверху.
    * @return true, если границы записаны в переменные, false, если -- нет.
    */
    public virtual bool get_area(int type, out double from_x, out double to_x, out double from_y, out double to_y)
    {
      from_y = from_x = 0;
      to_y = to_x = 1;
      return false;
    }

    // Для Tiler'ов с асинхронными задачами -->
      /**
      * Метод для форсирования обновления данных.
      * Если реализация класса Tiler работает синхронно, то этот метод переопределять не нужно.
      *
      * Функция совместима с типом GSourceFunc.
      *
      * @return В случае успеха Source.CONTINUE, в случае ошибки -- Source.REMOVE.
      */
      public virtual bool update_data() { return Source.CONTINUE; }
      /**
      * Метод удаляет все асинхронные задания на формирования плиток, если такие есть.
      * Если реализация класса Tiler работает синхронно, то этот метод переопределять не нужно.
      */
      public virtual void drop_tasks() { return; }
      /**
      * Метод позволяет получить количество еще не обработанных асинхронных заданий
      * на формирование плиток.
      * Если реализация класса Tiler работает синхронно, то этот метод переопределять не нужно.
      * @return Количество необработанных заданий.
      */
      public virtual uint get_tasks_num() { return 0; }
    // Для Tiler'ов с асинхронными задачами <--
  // Абстрактные и виртуальные методы <--


  construct
  {
    for(int i = 0; i < this.groups.length; i++)
      this.groups[i] = this.cache.reg_group();

    this.buf_for_cache_get = new MemTile();
  }


  ~Tiler()
  {
    // Чистим группы кеша, зарегистрированные для типов плиток.
    foreach(uint group in this.groups)
    {
      this.cache.clean(group);
      this.cache.unreg_group(group);
    }
  }

  /**
   * Очищает кэш с плитками типа "type" объекта Gp.Tiler, опционально -- по условию.
   * @param type Тип плитки;
   * @param condition Функция-условие: для каждой плитки в кеше вызывается данная функция,
   * первым аргументом ей передается указатель на описание плитки в кэше, вторым -- user_data.
   * Удаляются только те плитки, которые удовлетворяют условию (condition вернула TRUE).
   * Если вместо condition передан null, то функция удалит кэш целиком.
   */
  public void cache_clean_by_condition(int type, Gp.TileAccessFunc? condition = null)
    requires(type < this.tile_types_num)
  {
    Gp.SmartCacheAccessFunc? cache_condition = null;

    if(condition != null)
      cache_condition = (malloc_data) =>
      {
        return condition(Gp.MemTile.get_tile_from_malloc_data(malloc_data));
      };

    this.cache.clean_by_condition(this.groups[type], cache_condition);
  }

  /**
   * Помечает плитку в кэше с плитками типа "type" объекта Gp.Tiler как неактуальную, опционально -- по условию.
   * @param type Тип плитки;
   * @param condition Функция-условие: для каждой плитки в кеше вызывается данная функция,
   * первым аргументом ей передается указатель на описание плитки в кэше, вторым -- user_data.
   * Помечаются неактуальными только те плитки, которые удовлетворяют условию (condition вернула TRUE).
   * Если вместо condition передан null, то функция помечает все плитки типа "type".
   */
  public void cache_mark_notactual(int type, Gp.TileAccessFunc? condition = null)
    requires(type < this.tile_types_num)
  {
    Gp.SmartCacheAccessFunc? cache_condition = null;

    if(condition != null)
      cache_condition = (malloc_data) =>
      {
        return condition(Gp.MemTile.get_tile_from_malloc_data(malloc_data));
      };

    this.cache.modify(this.groups[type], cache_condition,
      (malloc_data) =>
      {
        return Gp.MemTile.set_status_to_malloc_data(malloc_data, TileStatus.NOT_ACTUAL);
      });
  }

  /**
  * Метод получения плитки, соответствующей переданным параметрам.
  *
  * @param buf Буфер, в который следует отрисовать плитку (в формате CAIRO_FORMAT_ARGB32);
  * @param tile Требуемая плитка
  * @param status Требуется плитка со статусом отрисовки более указанного.
  *
  * @return Статус отрисовки плитки больше требуемого в случае, если плитка отрисована,
  * TileStatus.NOT_INIT в случае если нет (тогда буфер остается нетронутым).
  */
  public Gp.TileStatus get_tile([CCode(array_length = false)] uint8[] buf, Gp.Tile tile,
    Gp.TileStatus status = Gp.TileStatus.NOT_INIT)
  {
    return_val_if_fail(tile.type < this.tile_types_num, TileStatus.NOT_INIT);

    var rval = TileStatus.NOT_INIT;

    uint index = tile.get_index();

    if(this.cache.get(this.groups[tile.type], index, null, this.buf_for_cache_get.get_malloc_data()))
    {
      if(Gp.Tile.equal_pos(this.buf_for_cache_get.tile, tile))
      {
        // Проверим, что нашли в кэше плитку со статусом больше требуемого.
        if(this.buf_for_cache_get.status > status)
        {
          Memory.copy(buf, this.buf_for_cache_get.get_buf(), TILE_DATA_SIZE);
          rval = this.buf_for_cache_get.status;
        }
      }
      else
        debug("Cache collision, index = %x", index);
    }

    // Достали из кэша уже полностью готовую плитку, нет смысла дальше вызывать from_source.
    if(rval == TileStatus.ACTUAL)
      return rval;

    MemTile? new_memtile;
    while((new_memtile = get_tile_from_source(tile, status)) != null)
      if(Gp.Tile.equal_all(new_memtile.tile, tile))
      {
        // Обновлять кэш есть смысл только, если from_srouce вернул более актуальную плитку, чем уже есть в кэше.
        if(new_memtile.status > rval)
        {
          rval = new_memtile.status;
          Memory.copy(buf, new_memtile.get_buf(), TILE_DATA_SIZE);
          this.cache.set(this.groups[tile.type], index, MemTile.free_to_malloc_data(new_memtile));
        }

        break;
      }
      else
        this.cache.set(this.groups[new_memtile.tile.type], new_memtile.tile.get_index(), MemTile.free_to_malloc_data(new_memtile));

    return rval;
  }
}
}
