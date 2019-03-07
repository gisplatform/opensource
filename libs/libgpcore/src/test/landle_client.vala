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

  const string HYPER_NEEDLE = "data:";

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

    var files_to_clean = new File[0];

    Landle? landle = null;
    var cancellable = new Cancellable();

    Gtk.init(ref args);

    var window = new Window();
    window.set_default_size(350, 480);
    window.title = _("Client") + " - Landle";
    window.set_icon_name("network-transmit-receive");
    window.destroy.connect(Gtk.main_quit);

    var vbox = new Box(Orientation.VERTICAL, 5);
    window.add(vbox);

    vbox.pack_start(new Label(_("Conncetion (host address : port):")), false);

    // Адрес, порт хоста, кнопка подключения-->
      var connect_hbox = new Box(Orientation.HORIZONTAL, 5);
      connect_hbox.border_width = BORDER_WIDTH;
      vbox.pack_start(connect_hbox, false);

      var host_entry = new Entry();
      connect_hbox.pack_start(host_entry);
      host_entry.text = "127.0.0.1";

      connect_hbox.pack_start(new Label(":"), false);

      var port_spin = new SpinButton.with_range(1, uint16.MAX, 1);
      connect_hbox.pack_start(port_spin);
      port_spin.value = 4797;

      var connect_icon = new Image.from_icon_name("network-transmit-receive", IconSize.BUTTON);
      var connect_spinner = new Spinner();
      connect_spinner.show();

      var connect_button = new ToggleButton();
      connect_button.add(connect_icon);
      connect_button.set_tooltip_text(_("Connect"));
      connect_hbox.pack_start(connect_button);

      host_entry.activate.connect(() => { connect_button.active = true; });
      port_spin.activate.connect(() => { connect_button.active = true; });
    // Адрес, порт хоста, кнопка подключения <--

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

    // Тэг для ссылки -->
      var hyper_tag = received_text_view.buffer.create_tag("hyper-tag",
        "foreground", "blue", "underline", Pango.Underline.SINGLE);

      received_text_view.event_after.connect((ev) =>
      {
        double ex, ey;
        int x, y;

        if(ev.type == Gdk.EventType.BUTTON_RELEASE)
        {
          var event = (Gdk.EventButton*)ev;

          if(event->button != Gdk.BUTTON_PRIMARY)
            return;

          ex = event->x;
          ey = event->y;
        }
        else
          if(ev.type == Gdk.EventType.TOUCH_END)
          {
            var event = (Gdk.EventTouch*)ev;

            ex = event->x;
            ey = event->y;
          }
          else
            return;

        received_text_view.window_to_buffer_coords(TextWindowType.WIDGET, (int)ex, (int)ey, out x, out y);

        TextIter iter;
        received_text_view.get_iter_at_location(out iter, x, y);

        if(iter.has_tag(hyper_tag))
        {
          iter.forward_to_tag_toggle(hyper_tag);

          unichar c = '\0';
          var start = iter;
          var end = iter;

          do // Найдем конец data-последовательности.
          {
            c = end.get_char();
          }
          while(c != '&' && c != '<' && c != '>' && c != '"' && c != '\'' &&
            (c.isprint() || c.isspace()) && end.forward_char());

          try
          {
            FileIOStream ios;
            File file = File.new_tmp("landle-XXXXXX.html", out ios);

            var dos = new DataOutputStream(ios.output_stream);

            start.backward_chars(HYPER_NEEDLE.length);
            var url = received_text_view.buffer.get_slice(start, end, false);

            size_t written;
            dos.write_all("<!DOCTYPE HTML><html><body><table border=\"1\"><tr><th>Data as URL:</th></tr><tr><td><a\nhref=\"".data, out written);
            dos.write_all(url.data, out written);
            dos.write_all("\"\n>Click here to open data URL</a></td></tr><tr><th>Data as image:</th></tr><tr><td><img\nsrc=\"".data, out written);
            dos.write_all(url.data, out written);
            dos.write_all("\"\n></td></tr></table></body></html>".data, out written);

            if(dialog_question(_("File with embedded data") + "\n'" +
                                file.get_path() + "'\n" +
                               _("has been created.") + "\n" +
                               _("Open it?"), window))
              Gtk.show_uri(null, file.get_uri(), Gtk.get_current_event_time());

            files_to_clean += file;
          }
          catch(Error e)
          {
            Gp.dialog_fail(e.message, window);
          }
        }
      });
    // Тэг для ссылки <--

    send_button.clicked.connect(() =>
    {
      try
      {
        if(unlikely(landle == null))
          #if VALA_0_28
            throw new IOError.NOT_CONNECTED(_("Not connected to server."));
          #else
            throw new IOError.FAILED(_("Not connected to server."));
          #endif

        var str = send_text_view.buffer.text.data;

        if(str.length != 0)
          str += '\0';

        // Можно в принципе переделать на новый API Landle.send_string().
        landle.send(str, compress_check.active);
      }
      catch(Error e)
      {
        Gp.dialog_fail(e.message, window);
      }
    });

    connect_button.toggled.connect(() =>
    {
      if(connect_button.active)
      {
        cancellable.reset();

        try
        {
          Resolver resolver = Resolver.get_default();
          List<InetAddress> addresses = resolver.lookup_by_name(host_entry.text, null);
          InetAddress address = addresses.nth_data(0);

          SocketConnection connection = null;
          SocketClient client = new SocketClient();

          connect_button.remove(connect_button.get_child());
          connect_button.add(connect_spinner);
          connect_spinner.start();

          client.connect_async.begin(new InetSocketAddress(address, (uint16)port_spin.value), cancellable, (obj, res) =>
          {
            connect_spinner.stop();
            connect_button.remove(connect_button.get_child());
            connect_button.add(connect_icon);

            try
            {
              connection = client.connect_async.end(res);

              landle = new Landle(connection, cancellable);

              landle.received.connect((l, data) =>
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
                  buffer.insert_markup(ref iter, Markup.printf_escaped(
                    "<span color=\"brown\">(%s) <b>%s:</b></span>", time.format("%H:%M:%S"), l.address), -1);
                #else
                  buffer.insert(ref iter,
                    "(%s) %s:".printf(time.format("%H:%M:%S"),
                    l.address), -1);
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
                  {
                    unowned string haystack = (string)data;
                    int start = 0, end = 0; //< Начало и конец текста вне тэга.

                    while((end = haystack.index_of(HYPER_NEEDLE, end)) >= 0)
                    {
                      buffer.insert(ref iter, haystack[start:end], end - start);

                      #if VALA_0_28
                        buffer.insert_with_tags(ref iter, HYPER_NEEDLE, HYPER_NEEDLE.length, hyper_tag);
                      #else
                      {
                        buffer.insert(ref iter, HYPER_NEEDLE, HYPER_NEEDLE.length);

                        var start_iter = iter;
                        start_iter.backward_chars(HYPER_NEEDLE.length);

                        buffer.apply_tag_by_name("hyper-tag", start_iter, iter);
                      }
                      #endif

                      start = end = end + HYPER_NEEDLE.length;
                    }

                    buffer.insert(ref iter, haystack[start:haystack.length], -1);
                  }
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
              }); //<landle.received.connect...

              landle.receiver_async.begin((obj, res) =>
              {
                connect_button.active = false;

                try
                {
                  landle.receiver_async.end(res);
                }
                catch(Error e)
                {
                  landle = null;

                  if(e is IOError.CANCELLED)
                    message("Receiver cancelled: %s", e.message);
                  else
                    Gp.dialog_fail(e.message, window);
                }
              });
            }
            catch(Error e)
            {
              connect_button.active = false;
              Gp.dialog_fail(e.message, window);
            }
          }); //< client.connect_async.begin...
        }
        catch(Error e)
        {
          connect_button.active = false;
          Gp.dialog_fail(e.message, window);
        }
      }
      else //< if(connect_button.active)
      {
        cancellable.cancel();
        landle = null;
      }
    }); //< connect_button.toggled.connect...

    window.show_all();
    Gtk.main();

    try
    {
      foreach(unowned File f in files_to_clean)
        f.delete();
    }
    catch(Error e)
    {
      Gp.dialog_fail(e.message, window);
    }

    return 0;
  }
}

