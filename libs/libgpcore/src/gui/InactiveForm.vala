/*
 * GpInactiveForm is an enum that can make widget inactive in defferent forms.
 *
 * Copyright (C) 2017 Sergey Volkhin.
 *
 * GpInactiveForm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpInactiveForm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpInactiveForm. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using Gtk;


namespace Gp
{
  /**
  * Задает форму, как отображать неактивный элемент.
  */
  public enum InactiveForm
  {
    /**
    * Отображать неактивный элемент "зетененным".
    */
    UNSENSITIVE,
    /**
    * Скрыть неактивный элемент полностью.
    */
    HIDE;

    /**
    * Применить форму отображения к виджету.
    * @param w Виджет, к которому применить форму отображения.
    * @param inactive true, чтобы сделать виджет неактивным, false, чтобы вернуть его в обычное состояние.
    */
    public void apply(Widget w, bool inactive = true)
    {
      switch(this)
      {
        case UNSENSITIVE:
          w.sensitive = !inactive;
        break;

        case HIDE:
          w.no_show_all = inactive;
          w.visible = !inactive;
        break;
      }
    }
  }
}

