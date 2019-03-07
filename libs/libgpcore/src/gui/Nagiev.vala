/*
 * GpNagiev is a service for switching between (GTK) windows.
 *
 * Copyright (C) 2018 Sergey Volkhin.
 *
 * GpNagiev is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpNagiev is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpNagiev. If not, see <http://www.gnu.org/licenses/>.
 *
*/


using Gtk;


namespace Gp
{
  public class Nagiev : Object
  {
    /**
    * Окно, которое будет показываться и на котором будут ожидаться горячие клавиши.
    */
    public Window? window { get; construct set; }
    /**
    * UDP-порт, он же идентификатор окна.
    */
    public uint port { get; construct set;  }
    /**
    * Объект для остановки работы объекта.
    */
    public Cancellable? cancellable { get; construct set;  }

    private const uint8 output_buffer[] = { 'S', 'W', 'I', 'T', 'C', 'H' };
    private uint8[] input_buffer = new uint8[output_buffer.length + 1];

    private InetAddress loop_address = new InetAddress.loopback(SocketFamily.IPV4);
    private GLib.Socket? socket = null;
    private bool init_done = false;

    /**
    * Создание объекта Nagiev.
    *
    * Можно не передавать порт, тогда Nagiev будет работать только в качестве клиента.
    * Можно не передавать окно, тогда Nagiev также сможет работать только в качестве клиента,
    * и, к тому же, не сможет обрабатывать горячие клавиши.
    *
    * @param window Окно, которое будет показываться и на котором будут ожидаться горячие клавиши.
    * @param port UDP-порт, он же идентификатор окна.
    * @param cancellable Объект для остановки работы объекта.
    */
    public Nagiev(Window? window = null, uint16 port = 0, Cancellable? cancellable = null) throws Error
    {
      Object(window:window, port:port, cancellable:cancellable);
      init(cancellable);
    }

    /**
    * Реализация метода инициализации GLib.Initable.init.
    *
    * @param cancellable Объект отмены.
    * @return true, если объект создался нормально, иначе -- false и бросается исключение.
    */
    public virtual bool init(Cancellable? cancellable = null) throws Error
    {
      if(init_done)
        return true;

      socket = new GLib.Socket(SocketFamily.IPV4, SocketType.DATAGRAM, SocketProtocol.UDP);

      if(port != 0)
      {
        if(window == null)
          throw new IOError.INVALID_ARGUMENT(_("If port set than there must be window too."));

        var sa = new InetSocketAddress(new InetAddress.loopback(SocketFamily.IPV4), (uint16)port);
        socket.bind(sa, true);

        var source = socket.create_source(IOCondition.IN);
        source.set_callback ((s, cond) =>
        {
          try
          {
            bool overflow;
            size_t read = s.receive(input_buffer);

            if(read < input_buffer.length)
            {
              input_buffer[read] = 0;
              overflow = false;
            }
            else
            {
              input_buffer[read - 1] = 0;
              overflow = true;
            }

            message("Nagiev windows service received command: %s%s",
              (string)input_buffer, overflow ? "..." : "");
            window.present();
          }
          catch (Error e)
          {
            dialog_fail(_("Nagiev windows service failed to receive command: ") + e.message, window);
            source.destroy();
          }
          return true;
        });
        source.attach(MainContext.default());

        if(cancellable != null)
          cancellable.cancelled.connect(() => { source.destroy(); socket = null; });
      }

      init_done = true;
      return true;
    }

    /**
    * @param dest_port Идентификатор окна, на которое следует переключиться.
    * @param keyval Gdk-код клавиши, пример: Gdk.Key.F1.
    */
    public void add_hotkey(uint16 dest_port, uint keyval)
      requires(window != null)
    {
      window.key_press_event.connect((widget, event) =>
      {
        if(event.keyval == keyval)
          try
          {
            switch_to_window(dest_port);
          }
          catch(Error e)
          {
            dialog_fail(_("Nagiev windows service failed to send command: ") + e.message, window);
          }

        return false;
      });
    }

    /**
    * @param dest_port Идентификатор окна, на которое следует переключиться.
    */
    public void switch_to_window(uint16 dest_port) throws Error
    {
      socket.send_to(new InetSocketAddress(loop_address, dest_port), output_buffer, cancellable);
    }
  }
}

