/*
 * GpBarista is a scrolled container with toolbar.
 *
 * Copyright (C) 2017 Sergey Volkhin.
 *
 * GpBarista is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpBarista is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpBarista. If not, see <http://www.gnu.org/licenses/>.
 *
*/


using Gtk;


namespace Gp
{
  /**
   * Виджет, содержащий ScrolledWindow и (по умолчанию пустой) Toolbar.
   * Имеет смысл использовать его с GpMenuTree.
   */
  public class Barista : Grid
  {
    public PositionType bar_position { public get; construct set; default = PositionType.BOTTOM; }
    public ScrolledWindow scrolled  { public get; private set; }
    public Toolbar toolbar  { public get; private  set; }

    construct
    {
      scrolled = new ScrolledWindow(null, null);
      scrolled.hexpand = true;
      scrolled.vexpand = true;
      scrolled.hscrollbar_policy = PolicyType.NEVER;
      attach(scrolled, 0, 0, 1, 1);

      toolbar = new Toolbar();
      attach_next_to(toolbar, scrolled, bar_position, 1, 1);
    }
  }
}

