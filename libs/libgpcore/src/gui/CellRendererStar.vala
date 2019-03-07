/*
 * GpCellRendererStar is a library for bookmarks representation n GtkTreeView.
 *
 * Copyright (C) 2015 Alexey Pankratov, Sergey Volkhin.
 *
 * GpCellRendererStar is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpCellRendererStar is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpCellRendererStar. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using Gtk;

namespace Gp{
  class CellRendererStar : CellRendererToggle
  {
    public const string[] STARRED_XMP =
    {
      "24 24 2 1", " 	c None", ".	c #FFFF00",
      "                        ", "                        ", "                        ",
      "           ..           ", "           ..           ", "          ....          ",
      "          ....          ", "          ....          ", "         ......         ",
      "   ..................   ", "   ..................   ", "   ..................   ",
      "     ..............     ", "      ............      ", "       ..........       ",
      "       ..........       ", "       ..........       ", "      ............      ",
      "      .....  .....      ", "      ....    ....      ", "      ..        ..      ",
      "                        ", "                        ", "                        "
    };

    public const string[] NOT_STARRED_XMP =
    {
      "24 24 2 1",
      " 	c None",
      ".	c #A6A6A6",
      "                        ", "                        ", "                        ",
      "           ..           ", "           ..           ", "          ....          ",
      "          ....          ", "          ....          ", "         ......         ",
      "   ..................   ", "   ..................   ", "   ..................   ",
      "     ..............     ", "      ............      ", "       ..........       ",
      "       ..........       ", "       ..........       ", "      ............      ",
      "      .....  .....      ", "      ....    ....      ", "      ..        ..      ",
      "                        ", "                        ", "                        "
    };

    private Gdk.Pixbuf pixbuf_starred;
    private Gdk.Pixbuf not_pixbuf_starred;
    /* icon property set by the tree column */
    private Gdk.Pixbuf icon;

    public CellRendererStar()
    {
      GLib.Object();

      pixbuf_starred = new Gdk.Pixbuf.from_xpm_data(STARRED_XMP);
      not_pixbuf_starred = new Gdk.Pixbuf.from_xpm_data(NOT_STARRED_XMP);

      if(this.active)
      {
        this.icon = pixbuf_starred;
      }
      else
      {
        this.icon = not_pixbuf_starred;
      }
      this.notify["active"].connect(() =>
        {
          if(this.active)
          {
            this.icon = pixbuf_starred;
          }
          else
          {
            this.icon = not_pixbuf_starred;
          }
        });
    }

    /* get_size method, always request a 50x50 area */
    public override void get_size(Widget widget, Gdk.Rectangle? cell_area,
                                  out int x_offset, out int y_offset,
                                  out int width, out int height)
    {
      x_offset = 0;
      y_offset = 0;
      width = 26;
      height = 26;
    }

    /* render method */
    public override void render(Cairo.Context ctx, Widget widget,
                                Gdk.Rectangle background_area,
                                Gdk.Rectangle cell_area,
                                CellRendererState flags)
    {
      Gdk.cairo_rectangle(ctx, background_area);
      if(icon != null) {
        ctx.translate(background_area.width / 2 - 12, background_area.height / 2 - 12);
        /* draw a pixbuf on a cairo context */
        Gdk.cairo_set_source_pixbuf(ctx, icon,
                                     background_area.x,
                                     background_area.y);
        ctx.fill();
      }
    }
  }
}
