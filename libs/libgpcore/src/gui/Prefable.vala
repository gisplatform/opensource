/*
 * GpPrefable is a library with interface for customizable objects.
 *
 * Copyright (C) 2016 Sergey Volkhin, Andrey Vodilov.
 *
 * GpPrefable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpPrefable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpPrefable. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using Gtk;

namespace Gp
{
  /**
   * Интерфейс для настраиваемых объектов.
   *
   * Объект, который нужно настраивать через GUI и/или хранить настройки в ini-файле,
   * есть смысл наследовать от данного универсального интерфейса.
   *
   * = Виджет с настройками. =
   *
   * Если настраиваемый объект предоставляет некий виджет с настройками,
   * следует переопределить метод "get_pref_widget".
   * По умолчанию метод "get_pref_widget" возвращает null, что значит,
   * что объект не имеет виджета с настройками.
   *
   * Соглашениями предполагается, что виджет с настройками должен компактно масштабироваться
   * по горизонтали. С тем, чтобы его можно было поместить как в отдельное диалоговое окно,
   * так и в SideBar.
   *
   * = Настройки в ini-файле. =
   *
   * Если требуется хранить настройки в ini-файле,
   * следует переопределить методы "config_save" и "config_load" (по умолчанию они не делают ничего).
   *
   * Для операций с ini-файлами в методах "config_save" и "config_load"
   * следует использовать группу, полученную методом "config_group", пример:
   * {{{
   * some_toggle_button.set_active(kf.get_boolean(this.config_group(), "some_bool"));
   * ...
   * kf.set_boolean(this.config_group(), "some_bool", some_toggle_button.get_active());
   * }}}
   * Хорошей идеей, как в примере выше, является хранение каждого параметра только в одном месте,
   * например, в виде состояния виджета, предназначенного для настрокйи этого параметра.
   *
   * Метод "config_group" по умолчанию возвращает имя класса,
   * и обычно его переопределять не требуется.
   */
  public interface Prefable : GLib.Object
  {
    /**
     * Метод получения группы ini-файла с настройками для данного класса.
     * @return Имя группы в ini-файле.
    */
    public virtual unowned string config_group()
    {
      return this.get_type().name();
    }

    /**
     * Метод загрузки настроек из ini-файла.
     * @param kf Ini-файл.
    */
    public virtual void config_load(KeyFile kf) throws KeyFileError
    {
      ;
    }

    /**
     * Метод сохранения настроек в ini-файл.
     * @param kf Ini-файл.
    */
    public virtual void config_save(KeyFile kf)
    {
      ;
    }

    /**
     * Метод получения ссылки на виджет с настройками объекта.
     * @return Виджет с настройками объекта, если таковой имеется.
    */
    public virtual unowned Widget? get_pref_widget()
    {
      return null;
    }

    /**
     * Метод получения строки с названием для виджета настроек объекта.
     * @return Строка с названием виджета.
    */
    public virtual unowned string? get_pref_widget_name()
    {
      return null;
    }

    /**
    * Статическая функция, которая загружает настройки для массива "prefables" из файла "kf".
    * Безопасно обрабатывает случаи отсутсвия каких-либо ключей в файле
    * (не бросает в таких случаях исключения, а просто печатает на экран информацию с помощью message).
     * @param kf Ini-файл.
     * @param prefables Массив реализаций Prefable.
    */
    public static void config_load_safe_propagate(KeyFile kf, Prefable[] prefables) throws KeyFileError
    {
      foreach(unowned Gp.Prefable p in prefables)
      {
        try
        {
          p.config_load(kf);
        }
        catch(KeyFileError e)
        {
          if(e is KeyFileError.KEY_NOT_FOUND || e is KeyFileError.GROUP_NOT_FOUND)
            message("Key hasn't been found in config: %s", e.message);
          else
            throw e;
        }
      }
    }
  }
}

