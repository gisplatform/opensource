/*
 * GpScreenDoctor is an object preventing screen timeout.
 *
 * Copyright (C) 2016 Sergey Volkhin.
 *
 * GpScreenDoctor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpScreenDoctor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpScreenDoctor. If not, see <http://www.gnu.org/licenses/>.
 *
*/



namespace Gp
{
  #if G_OS_UNIX
    [DBus (name = "org.freedesktop.ScreenSaver")]
    private interface ScreenSaverIface : Object
    {
      public abstract uint Inhibit(string app_name, string localized_reason) throws Error;
      public abstract void UnInhibit(uint cookie) throws Error;
      public abstract void SimulateUserActivity() throws Error;
    }
  #endif



  /**
   * Объект, блокирующий скринсейвер (а также режим энергосбережения,
   * выключающий монитор при простое) на время своего существования.
   * Можно создавать несколько экземпляров ScreenDoctor,
   * скринсейвер будет разблокирован после уничтожения последнего из них.
   * Совместим с идеомой RAII.
   */
  public class ScreenDoctor : Object
  {
    // Внутренние поля класса ScreenDoctor вынесены в этот отдельный класс,
    // чтобы создавать лямбда-функции, не содержащие ссылок на ScreenDoctor.
    private class ScreenDoctorInfo
    {
      public uint counter = 0;

      #if G_OS_UNIX
        public unowned X.Display display;

        // DBus.
        public uint cookie;
        public ScreenSaverIface? screensaver_iface = null;

        // XScreenSaver.
        public int timeout;
        public int interval;
        public int prefer_blanking;
        public int allow_exposures;

        // DPMS.
        public char dpms_state;

        public void start_simulate_user_activity()
        {
          Timeout.add(180000, ()=>
          {
            try
            {
              screensaver_iface.SimulateUserActivity();
            }
            catch(Error e)
            {
              warning("Failed to simulate user activity via DBus: %s", e.message);
              return false;
            }

            return (counter != 0);
          });
        }
      #endif
    }

    // Информация одна для всех экземпляров ScreenDoctor.
    private class ScreenDoctorInfo info = new ScreenDoctorInfo();

    /*
    * Создание блокировщика скринсейвера.
    * @param widget_on_display Виджет, отображенный на экране, скринсейвер на котором нужно заблокировать.
    */
    public ScreenDoctor(Gtk.Widget widget_on_display)
    {
      if(unlikely(widget_on_display.get_mapped() == false))
      {
        critical("Failed to block screensaver: widget_on_display must be mappped!");
        return;
      }

      if(info.counter == 0)
      {
        #if G_OS_UNIX
          unowned Gdk.Display gdk_display = widget_on_display.get_display();
        #if VALA_0_24
          unowned Gdk.X11.Display gdk_x11_display = (Gdk.X11.Display)gdk_display;
          info.display = gdk_x11_display.get_xdisplay();
        #else
          unowned Gdk.X11Display gdk_x11_display = (Gdk.X11Display)gdk_display;
          info.display = Gdk.X11Display.get_xdisplay(gdk_x11_display);
        #endif

          //XScreenSaver.
          info.display.get_screensaver(out info.timeout, out info.interval, out info.prefer_blanking, out info.allow_exposures);
          info.display.set_screensaver(0, 0, info.prefer_blanking, info.allow_exposures);

          // DPMS.
          uint16 power_level;
          DPMS.info(info.display, out power_level, out info.dpms_state);

          if(info.dpms_state != 0)
            DPMS.disable(info.display);

          // DBus.
          try
          {
            string app_name;
            Gtk.Window? toplevel = widget_on_display.get_toplevel() as Gtk.Window;

            if(toplevel != null && toplevel.get_title() != null)
              app_name = toplevel.get_title();
            else
              app_name = "App with GpScreenDoctor";

            info.screensaver_iface = Bus.get_proxy_sync(BusType.SESSION, "org.freedesktop.ScreenSaver", "/ScreenSaver", DBusProxyFlags.NONE);
            info.cookie = info.screensaver_iface.Inhibit(app_name, "Inhibited by GpScreenDoctor");
          }
          catch(Error e)
          {
            warning("Failed to block screensaver via DBus: %s", e.message);
          }

          info.start_simulate_user_activity();
        #endif
      }

      info.counter++;
    }

    ~ScreenDoctor()
    {
      // Такое могло произойти, если с ошибкой отработал конструктор (и ошибку мы уже показали).
      if(unlikely(info.counter == 0))
        return;

      info.counter--;

      if(info.counter == 0)
      {
        #if G_OS_UNIX
          //DBus.
          if(info.screensaver_iface != null)
          {
            try
            {
              info.screensaver_iface.UnInhibit(info.cookie);
            }
            catch(Error e)
            {
              warning("Failed to unblock screensaver via DBus: %s", e.message);
            }
          }

          //XScreenSaver.
          info.display.set_screensaver(info.timeout, info.interval, info.prefer_blanking, info.allow_exposures);

          // DPMS.
          if(info.dpms_state != 0)
            DPMS.enable(info.display);
        #endif
      }
    }

    /*
    * Функция получения счетчика блокировок скринсейвера (т.е. кол-ва объектов ScreenDoctor).
    * @return Счетчик блокировок скринсейвера (т.е. кол-во объектов ScreenDoctor).
    */
    public uint get_counter()
    {
      return info.counter;
    }

    /*
    * Информация о состоянии ScreenDoctor в текстовом виде.
    * @return Строка с информацией.
    */
    public string to_string()
    {
      return "ScreenDoctor:"
        #if G_OS_UNIX
          + "\nDPMS state was " + ((uint8)info.dpms_state).to_string()
          + "\ntimeout was "    + info.timeout.to_string()
          + "\ninterval was "   + info.interval.to_string()
        #endif
        ;
    }
  }
}

