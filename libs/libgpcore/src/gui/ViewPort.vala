/*
 * GpViewPort is a container with overlay panel.
 *
 * Copyright (C) 2015 Sergey Volkhin, Andrey Vodilov, Alexey Pankratov.
 *
 * GpViewPort is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpViewPort is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpViewPort. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using Gtk;

namespace Gp
{
  #if VALA_0_24
  /**
   * Виджет, обеспечивающий отображение основного виджета и панели (справа или слева) поверх него.
   */
  public class ViewPort<T> : Gtk.Overlay
  {
    /**
    * Показывать ли сейчас панель.
    */
    public bool reveal_panel
    {
      set
      {
        revealer.set_reveal_child(value);
      }
      get
      {
        return revealer.get_reveal_child();
      }
    }

    private Revealer revealer = new Revealer();
    private T panel;

    construct
    {
      revealer.set_valign(Align.FILL);
      base.add_overlay(revealer);
    }

    /**
    * Добавление панели.
    * Метод можно вызвать повторно, тогда новая панель просто заменит собой старую.
    * @param panel Виджет с содержимым панели (должен быть типом, занаследованным от GtkWidget).
    * @param position Слева панель или справа.
    */
    public void attach_panel(T panel, PositionType position = Gtk.PositionType.LEFT)
    {
      if(revealer.get_child() != null)
        revealer.remove(revealer.get_child());

      switch(position)
      {
        case PositionType.LEFT:
          revealer.set_transition_type(RevealerTransitionType.SLIDE_LEFT);
          revealer.set_halign(Align.START);
        break;
        case PositionType.RIGHT:
          revealer.set_transition_type(RevealerTransitionType.SLIDE_RIGHT);
          revealer.set_halign(Align.END);
        break;
        default:
          critical("Wrong position = %s", position.to_string());
        break;
      }

      revealer.add(panel as Widget);
      this.panel = panel;
    }

    /**
    * Функция получения содержимого панели.
    * @return Виджет с содержимым панели (тип, занаследованный от GtkWidget).
    */
    public unowned T get_panel()
    {
      return panel;
    }
  }
  #endif
}
