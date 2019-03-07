/*
 * GpLandle is a async library (includes server) to send packets through TCP (with a little help of GpChunk).
 *
 * Copyright (C) 2018 Sergey Volkhin.
 *
 * GpLandle is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpLandle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpLandle. If not, see <http://www.gnu.org/licenses/>.
 *
*/



namespace Gp
{
/*
* Объект для передачи пакетов данных с заголовками по TCP.
*
* Все ошибки (даже ошибки отправки данных) передаются через результат выполнения receiver_async.
*/
public class Landle : Object
{
  // Сигналы -->
    /**
    * Пришли новые данные (целое сообщение).
    * @param landle Объект типа Landle.
    * @param data Данные сообщения (без заголовка).
    */
    public signal void received(Landle landle, uint8[] data);
  // Сигналы <--

  // Свойства -->
    /**
    * Приоритет ввода-вывода Landle'а.
    */
    public int priority { public get; public set; default = Priority.DEFAULT; }

    /**
    * Адрес противоположной стороны.
    */
    public string address { public get; construct; }

    public DataInputStream dis { private get; construct; }
    public DataOutputStream dos { private get; construct; }
    public SocketConnection con { private get; construct; }
    public Cancellable? cancellable { public get; construct; }
  // Свойства <--

  Chunk to_receive = new Chunk.sized(0);
  Queue<Chunk> to_send = new Queue<Chunk>();

  bool sending = false; //< Мини конечный автомат отправки с двумя состояниями.
  bool receiving_header = true; //< Мини конечный автомат приема с двумя состояниями.

  Error? send_error = null;

  public Landle(SocketConnection connection, Cancellable? cancellable = null)
  {
    string address = "";

    try
    {
      var remote_address = connection.get_remote_address();

      if(remote_address is InetSocketAddress)
        address = (remote_address as InetSocketAddress).get_address().to_string();
      else
        throw new IOError.INVALID_ARGUMENT("Connection is not a network connection.");
    }
    catch(Error e)
    {
      warning("Failed to determine Landle remote address: %s", e.message);
    }

    Object(
      address: address,
      con: connection,
      dis: new DataInputStream(connection.input_stream),
      dos: new DataOutputStream(connection.output_stream),
      cancellable: cancellable);
  }

  /**
  * @param user_buffer Сообщение для отправки.
  * @param compress Сжимать ли данные.
  */
  public void send(uint8[] user_buffer, bool compress = false) throws Error
    requires(user_buffer.length < int32.MAX)
  {
    var chunk = new Chunk.with_data(user_buffer);

    chunk.deflate(compress);

    to_send.push_tail((owned)chunk);

    if(!sending)
      send_async.begin();
  }

  /**
  * @param str Сообщение для отправки в виде null-terminated строки.
  * @param compress Сжимать ли данные.
  */
  public void send_string(string str, bool compress = false) throws Error
    requires((str.length + 1) < int32.MAX)
  {
    var chunk = new Chunk.with_string(str);

    chunk.deflate(compress);

    to_send.push_tail((owned)chunk);

    if(!sending)
      send_async.begin();
  }

  private async void send_async()
  {
    Chunk? chunk;

    sending = true;

    try
    {
      while((chunk = to_send.pop_head()) != null)
        while((chunk.bytes_processed != chunk.data.length) && con.is_connected())
          chunk.bytes_processed += yield dos.write_async(chunk.data[chunk.bytes_processed:chunk.data.length], priority, cancellable);
    }
    catch(Error e)
    {
      send_error = e;
    }

    sending = false;
  }

  /**
  * Запуск работы Landle.
  */
  public async void receiver_async() throws Error
  {
    while(con.is_connected())
    {
      var read_rval = yield dis.read_async(to_receive.data[to_receive.bytes_processed:to_receive.data.length], priority, cancellable);

      if(unlikely(cancellable != null && cancellable.is_cancelled()))
        return;

      if(unlikely(send_error != null))
        throw send_error;

      if(unlikely(read_rval == 0))
        #if VALA_0_28
          throw new IOError.NOT_CONNECTED(_("No data in conncetion."));
        #else
          throw new IOError.FAILED(_("No data in conncetion."));
        #endif

      to_receive.bytes_processed += read_rval;

      if(receiving_header) //< Мини конечный автомат, состояние: чтение заголовка.
      {
        if(to_receive.bytes_processed == Chunk.HEADER_SIZE) //< Собрали весь заголовок.
        {
          to_receive.data.resize((int)(Chunk.HEADER_SIZE + to_receive.get_user_data_size_from_header()));

          receiving_header = false; //< Переключаем на мини конечный автомат на чтение данных.
        }
      }

      if(receiving_header == false) //< Мини конечный автомат, состояние: чтение данных.
      {
        if(to_receive.bytes_processed == to_receive.data.length) //< Собрали все данные.
        {
          to_receive.deflate(false);

          // Сигнализируем пользователю.
          received(this, to_receive.data[Chunk.HEADER_SIZE:to_receive.data.length]);

          // Взводим мини конечный автомат в изначальное состояние.
          to_receive.data.resize((int)Chunk.HEADER_SIZE);
          to_receive.bytes_processed = 0;
          receiving_header = true;
        }
      }
    }
  }


  // --------------- Сервер для Landle --------------- -->
  /**
   * Важно! Сервер работает в рамках MainLoop дефолтного контекста треда.
   * Для обращения из других тредов следует использовать, например, IdleSource.
   */
  public class Server : Object, GLib.Initable
  {
    // Сигналы -->
      /**
      * Подсоединился новый клиент.
      * @param client Клиент.
      */
      public signal void connected(Landle client);

      /**
      * Отсоединился клиент.
      * @param client Клиент.
      */
      public signal void disconnected(Landle client, Error? error);
    // Сигналы <--

    // Свойства -->
      /**
      * Порт, на котором слушать соединения.
      */
      public uint port { public get; construct; }

      /**
      * Объект для остановки сервера.
      */
      public Cancellable? cancellable { public get; construct; }
    // Свойства <--

    private SocketService? service;

    /**
    * Создает новый TCP-сервер.
    * @param port Номер порта для входящих соединений.
    */
    public Server(uint16 port, Cancellable? cancellable = null) throws Error
    {
      Object(port: port, cancellable: cancellable);
      init(cancellable);
    }

    public virtual bool init(Cancellable? cancellable = null) throws Error
    {
      if(service != null)
        return true;

      service = new SocketService();

      service.incoming.connect((connection) =>
      {
        var new_client = new Landle(connection, cancellable);

        new_client.receiver_async.begin((obj, res) =>
        {
          try
          {
            new_client.receiver_async.end(res);
            disconnected(new_client, null);
          }
          catch(Error e)
          {
            disconnected(new_client, e);
          }
        });

        connected(new_client);
        return false;
      });

      service.add_inet_port((uint16)port, null);
      service.start();

      if(cancellable != null)
        cancellable.cancelled.connect(() =>
          {
            service.stop();
            service = null;
          });

      return true;
    }
  } //< public class Server
  // --------------- Сервер для Landle --------------- <--
} //< public class Landle
} //< namespace Gp

