/*
 * GpActivityButton is a widget showing completions of activities.
 *
 * Copyright (C) 2016 Sergey Volkhin.
 *
 * GpActivityButton is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpActivityButton is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpActivityButton. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using Gtk;

namespace Gp
{
  #if VALA_0_24
  /**
   * Виджет с информацией о прогрессе завершения ряда выполняемых активностей (неких действий).
   *
   * Виджет представляет собой кнопку с отображением доли завершения всех активностей на ней.
   * По клику на кнопку показыввается Gtk.Popover с виджетами Gtk.ProgressBar (список активностй),
   * каждый из которых показывает прогресс выполнения одной из активностй.
   * Добавить активность в список, либо обновить состояние активности,
   * можно методом Gp.ActivityButton.set_fraction().
   * Удалить активность из списка можно методом Gp.ActivityButton.done().
   * Выполняемые активности идентифицируются их именами,
   * эти же имена показываются в виджетах Gtk.ProgressBar.
   *
   * В метод set_fraction можно опционально передать объект для отмены активности (Cancellable).
   * Если Cancellable != null, то справа от Gtk.ProgressBar'а активности будет отрисована
   * кнопка отмены, при ее нажатии будет вызван метод Cancellable.cancel().
   * Аргумент Cancellable обрабатывается только для первого вызова set_fraction() для данной
   * активности, в последующих вызовах set_fraction можно передавать null вместо него.
   */
  public class ActivityButton : ToggleButton
  {
    private const int SIZE = 24;
    private double total_fraction = -1;
    private DrawingArea darea = new DrawingArea();
    private Popover popover;
    private Label no_activity_label;
    private Grid grid = new Grid();
    private HashTable<string, ProgressBar> table
      = new HashTable<string, ProgressBar>(GLib.str_hash, GLib.str_equal);

    construct
    {
      no_activity_label = new Label(_("No activities"));
      grid.add(no_activity_label);

      popover = new Popover(this);
      popover.set_modal(false);
      popover.set_position(PositionType.TOP);

      this.toggled.connect(() =>
      {
        popover.set_visible(this.get_active());
      });

      popover.add(grid);
      grid.show_all();
      
      this.add(darea);
      darea.set_size_request(SIZE, SIZE);
      darea.show();
      darea.draw.connect((cr) =>
      {
        draw_pie(cr, total_fraction, SIZE / 2);
      });
    }

    /**
     * Показывает информацию о доли завершения активности.
     * @param activity Имя активности.
     * @param fraction Доля завершения, от 0 до 1.
     * @param cancellable Объект для отмены активности.
    */
    public void set_fraction(string activity, double fraction = 0, Cancellable? cancellable = null)
    {
      string orig_key;
      ProgressBar bar;

      if(table.lookup_extended(activity, out orig_key, out bar))
        bar.set_fraction(fraction);
      else
      {
        if(table.length == 0)
          no_activity_label.visible = false;

        bar = new ProgressBar();
        bar.text = activity;
        bar.set_show_text(true);
        grid.attach_next_to(bar, null, PositionType.BOTTOM, 1, 1);
        bar.show();

        if(cancellable != null)
        {
          var cancel_butt = new Button.from_icon_name("process-stop", IconSize.MENU);
          cancel_butt.relief = ReliefStyle.NONE;
          cancel_butt.set_tooltip_text(_("Cancel"));
          bar.valign = Align.CENTER;

          // Если bar удаляется из grid'а, то и cancel_butt удалим.
          bar.parent_set.connect(() =>
          {
            grid.remove(cancel_butt);
          });

          cancel_butt.clicked.connect(() =>
          {
            cancellable.cancel();
            done(activity);
          });

          grid.attach_next_to(cancel_butt, bar, PositionType.RIGHT, 1, 1);
          cancel_butt.show();
        }

        table.insert(activity, bar);
      }

      update_total_fraction();
    }

    /**
    * Удаляет активность из списка.
    *
    * Если активности с таким именем нет в ActivityButton,
    * то это не считается ошибкой, функция просто молча вернет управление.
    *
    * @param activity Имя активности.
    */
    public void done(string activity)
    {
      string orig_key;
      ProgressBar bar;

      if(table.lookup_extended(activity, out orig_key, out bar))
      {
        grid.remove(bar);
        table.remove(activity);

        if(table.length == 0)
          no_activity_label.visible = true;
    
        update_total_fraction();
      }
    }

    private void update_total_fraction()
    {
      if(table.length != 0)
      {
        total_fraction = 0;

        table.foreach((activity, bar) =>
        {
          total_fraction += bar.fraction;
        });

        total_fraction /= table.length;
      }
      else
        total_fraction = -1;

      darea.queue_draw_area(0, 0, SIZE, SIZE);
    }
  }
  #endif
}
