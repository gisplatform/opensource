/*
 * GpLandle is a async library (includes server) to transfer _packets_ through TCP.
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

using Gp;
using Gtk;

public class Main : Object
{
  private const uint BORDER_WIDTH = 3;

  public enum Client { LANDLE, ADDRESS, TIME, COLOR, COLUMNS_NUM }

  public static int main (string[] args)
  {
    { // Локализация -->
      var prog_bin_dir = GLib.Path.get_dirname(args[0]);
      var prog_root_dir = GLib.Path.build_filename(prog_bin_dir, "..", null);
      var locale_path = GLib.Path.build_filename(prog_root_dir, "share", "locale", null);

      GLib.Intl.bindtextdomain("libgpcore", locale_path);

      #if G_OS_WIN32
        GLib.Intl.bind_textdomain_codeset("libgpcore", "UTF-8" );
        GLib.Intl.textdomain("libgpcore");
      #endif
    } // Локализация <--

    Landle.Server? landle_server = null;
    var cancellable = new Cancellable();

    Gtk.init(ref args);

    var clients_colors = new Queue<string>();
    clients_colors.push_tail("red");
    clients_colors.push_tail("orange");
    clients_colors.push_tail("brown");
    clients_colors.push_tail("green");
    clients_colors.push_tail("blue");
    clients_colors.push_tail("dark blue");
    clients_colors.push_tail("violet");

    var window = new Window();
    window.set_default_size(350, 480);
    window.title = _("Server") + " - Landle";
    window.set_icon_name("network-server");
    window.destroy.connect(Gtk.main_quit);

    var vbox = new Box(Orientation.VERTICAL, 5);
    window.add(vbox);

    // Порт и кнопка запуска сервера -->
      var serve_hbox = new Box(Orientation.HORIZONTAL, 5);
      serve_hbox.border_width = BORDER_WIDTH;
      vbox.pack_start(serve_hbox, false);
      serve_hbox.halign = Align.CENTER;

      serve_hbox.pack_start(new Label(_("Port:")), false);

      var port_spin = new SpinButton.with_range(1, uint16.MAX, 1);
      serve_hbox.pack_start(port_spin, false);
      port_spin.value = 4797;

      var serve_button = new ToggleButton();
      serve_button.add(new Image.from_icon_name("network-server", IconSize.BUTTON));
      serve_button.set_tooltip_text(_("Start/stop server"));
      serve_hbox.pack_start(serve_button, false);

      port_spin.activate.connect(() => { serve_button.active = true; });
    // Порт и кнопка запуска сервера <--

    // Список клиентов -->
      var clients_frame = new Frame(_("Connected clients:"));
      clients_frame.border_width = BORDER_WIDTH;
      vbox.pack_start(clients_frame, true);

      var clients_scroll = new ScrolledWindow(null, null);
      clients_frame.add(clients_scroll);

      var clients = new Gtk.ListStore(Client.COLUMNS_NUM, typeof(Landle), typeof(string), typeof(string), typeof(string));
      var clients_treeview = new TreeView.with_model(clients);
      clients_treeview.get_selection().mode = SelectionMode.MULTIPLE;
      clients_scroll.add(clients_treeview);

      clients_treeview.insert_column_with_attributes(-1, _("Address"), new CellRendererText(),
        "text", Client.ADDRESS, "foreground", Client.COLOR);
      clients_treeview.insert_column_with_attributes(-1, _("Connection time"), new CellRendererText(),
        "text", Client.TIME);
    // Список клиентов <--

    // Сообщение на отправку -->
      var send_frame = new Frame(null);
      send_frame.border_width = BORDER_WIDTH;
      vbox.pack_start(send_frame, true);

      // Заголов фрейма -->
        var send_hbox = new Box(Orientation.HORIZONTAL, 0);
        send_frame.label_widget = send_hbox;

        send_hbox.pack_start(new Label(_("Message to send") + " ("), false);

        var compress_check = new CheckButton.with_label(_("compress it"));
        send_hbox.pack_start(compress_check, false);

        send_hbox.pack_start(new Label("):"), false);
      // Заголов фрейма <--

      // Содержимое фрейма -->
        var send_vbox = new Box(Orientation.VERTICAL, 0);
        send_frame.add(send_vbox);

        var send_scrolled = new ScrolledWindow(null, null);
        send_vbox.pack_start(send_scrolled, true);

        var send_text_view = new TextView();
        send_scrolled.add(send_text_view);

        var send_button = new Button();
        send_button.add(new Image.from_icon_name("mail-send", IconSize.BUTTON));
        send_button.set_tooltip_text(_("Send message"));
        send_vbox.pack_start(send_button, false);
      // Содержимое фрейма <--
    // Сообщение на отправку <--

    // Принятые сообщения -->
      var received_frame = new Frame(_("Log of received messages:"));
      received_frame.border_width = BORDER_WIDTH;
      vbox.pack_start(received_frame, true);

      var received_scrolled = new ScrolledWindow(null, null);
      received_frame.add(received_scrolled);

      var received_text_view = new TextView();
      received_text_view.editable = false;
      received_scrolled.add(received_text_view);

      { // Создадим закладку для автоскроллинга -->
        TextIter iter;
        var buf = received_text_view.buffer;
        buf.get_end_iter(out iter);
        buf.create_mark("autoscroll", iter, false);
      } // Создадим закладку для автоскроллинга <--
    // Принятые сообщения <--

    send_button.clicked.connect(() =>
    {
      var str = send_text_view.buffer.text.data;

      if(str.length != 0)
        str += '\0';

      clients.foreach((m, p, i) =>
      {
        if(clients_treeview.get_selection().iter_is_selected(i))
          try
          {
            Value value;
            m.get_value(i, Client.LANDLE, out value);

            // Можно в принципе переделать на новый API Landle.send_string().
            (value as Landle).send(str, compress_check.active);
          }
          catch(Error e)
          {
            Gp.dialog_fail(e.message, window);
            return true;
          }

        return false;
      });
    });

    serve_button.clicked.connect(() =>
    {
      if(serve_button.active)
      {
        cancellable.reset();

        try
        {
          message("Starting server...");
          landle_server = new Landle.Server((uint16)port_spin.value, cancellable);
          landle_server.connected.connect((l) =>
          {
            {
              var color = clients_colors.pop_head();
                TreeIter iter;
                clients.append(out iter);
                clients.set(iter,
                  Client.LANDLE, l,
                  Client.ADDRESS, l.address,
                  Client.TIME, new DateTime.now_local().format("%H:%M:%S"),
                  Client.COLOR, color);
              l.set_data<string>("landle-server-color", color);
              clients_colors.push_tail((owned)color);

              clients_treeview.get_selection().select_iter(iter);
            }

            l.received.connect((l_received, data) =>
            {
              var time = new DateTime.now_local();

              var buffer = received_text_view.buffer;
              TextIter iter;
              buffer.get_end_iter(out iter);

              var vadj = received_text_view.vadjustment;
              // Вместо простого '==', сделаем запас '+ 9' и проверим на '>'.
              // Т.к. если придут 2 сообщения подряд благодаря спецэффектам Gtk3
              // ScrolledWindow может не успеть прокрутиться до конца
              // после первого сообщения до прихода второго.
              bool autoscroll = ((vadj.value + 9 + vadj.page_size) > vadj.upper);

              #if VALA_0_28
                string? color = l_received.get_data<string>("landle-server-color");
                return_if_fail(color != null);

                buffer.insert_markup(ref iter, Markup.printf_escaped(
                  "<span color=\"" + color + "\">(%s) <b>%s:</b></span>",
                  time.format("%H:%M:%S"), l_received.address), -1);
              #else
                buffer.insert(ref iter,
                  "(%s) %s:".printf(time.format("%H:%M:%S"),
                  l_received.address), -1);
              #endif

              if(data.length != 0)
              {
                // Проверка наличия нуля в строке -->
                  bool null_terminated = false;

                  foreach(uint8 b in data)
                    if(b == '\0')
                    {
                      null_terminated = true;
                      break;
                    }
                // Проверка наличия нуля в строке <--

                if(null_terminated)
                  buffer.insert(ref iter, (string)data, -1);
                else
                  #if VALA_0_28
                    buffer.insert_markup(ref iter, Markup.printf_escaped(
                    "<span color=\"red\">%c%s%c</span>", '<', _("Received string is not null-terminated"), '>'), -1);
                  #else
                    buffer.insert(ref iter,
                    "%c%s%c".printf('<', _("Received string is not null-terminated"), '>'), -1);
                  #endif
              }
              else
                #if VALA_0_28
                  buffer.insert_markup(ref iter, Markup.printf_escaped(
                  "<span color=\"brown\">%c%s%c</span>", '<', _("Empty message"), '>'), -1);
                #else
                  buffer.insert(ref iter,
                  "%c%s%c".printf('<', _("Empty message"), '>'), -1);
                #endif

              buffer.insert(ref iter, "\n", 1);

              if(autoscroll)
              {
                var mark = buffer.get_mark("autoscroll");
                return_if_fail(mark != null);
                buffer.move_mark(mark, iter);
                received_text_view.scroll_mark_onscreen(mark);
              }
            });
          }); //< landle_server.connected.

          landle_server.disconnected.connect((l, e) =>
          {
            clients.foreach((m, p, i) =>
            {
              Value value;
              m.get_value(i, Client.LANDLE, out value);

              if((value as Landle) == l)
                return clients.remove(i);
              else
                return false;
            });

            if(e != null)
            {
              if(e is IOError.CANCELLED)
                message("Client '%s' has been cancelled: %s", l.address, e.message);
              else
              {
                var str = _("Client '%s' disconnected with error: %s").printf(l.address, e.message);
                Gp.dialog_fail(str, window);
              }
            }
          }); //< landle_server.disconnected.
        }
        catch(Error e)
        {
          serve_button.active = false;

          if(e is IOError.CANCELLED)
            message("Cancelled: %s", e.message);
          else
            Gp.dialog_fail(e.message, window);
        }
      }
      else
        if(landle_server != null)
        {
          landle_server.cancellable.cancel();
          landle_server = null;
        }
    });

    window.show_all();
    Gtk.main();

    return 0;
  }
}

