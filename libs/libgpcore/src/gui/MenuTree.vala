/*
 * GpMenuTree is a smartphone/FF-like menu widget.
 *
 * Copyright (C) 2017 Sergey Volkhin.
 *
 * GpMenuTree is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpMenuTree is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpMenuTree. If not, see <http://www.gnu.org/licenses/>.
 *
*/


using Gtk;


namespace Gp
{
  #if VALA_0_24
  /**
   * Виджет меню.
   * Меню чем-то похоже на меню iOS или Firefox.
   * Для задания иерархии меню используется TreeModel.
   * Модель можно заполнять и после создания объекта TreeMenu,
   * но после создания объекта уже нельзя удалять и менять элементы в модели.
   * Визуально меню построено в виде списков (как на смартфонах),
   * корень иерархии меню также можно показать в виде иконок как в FF (если icon_column != -1).
   * На концах веток меню -- пользовательские виджеты (в колонках widget_column).
   * Одни и те же виджеты можно использовать одновременно в разных объектах TreeMenu,
   * также для разных объектов TreeMenu можно использовать одну и ту же модель,
   * конфликты отображения пользовательских виджетов разрешаются объектом TreeMenu корректно.
   * Текст в списках и текст в навигации может отличаться (колонки detailed_column и brief_column
   * соответственно) или быть одинаковым (колонка brief_column, в случае detailed_column = -1).
   */
  public class MenuTree : Grid
  {
    /**
    * Модель, описывающая стркутуру меню.
    */
    public TreeModel model  { get; construct set; }
    /**
    * Номер колонки с пользовательскими виджетами для конечных точек дерева меню.
    */
    public int widget_column { get; construct set; default = -1; }
    /**
    * Номер колонки с текстом для пункта меню (-1, если можно использовать brief_column).
    */
    public int detailed_column  { get; construct set; default = -1; }
    /**
    * Номер колонки с коротким описанием пункта меню (для использования в навигации).
    */
    public int brief_column { get; construct set; default = -1; }
    /**
    * Номер колонки с иконками для меню (-1, если нужно показывать корневое меню просто в виде строк).
    */
    public int icon_column  { get; construct set; default = -1; }

    /**
    * Сигнал инициируется, когда нажата кнопка движения вверх по меню в корне меню.
    * Сам виджет в таких случаях не делает ничего, а вот пользователь волен использовать
    * данный сигнал по своему усмотрению.
    */
    public signal void root_up_clicked();

    private string? root_detailed = null;
    private string? root_up_detailed = null;
    private string root_brief = "";
    private string? root_up_brief = null;

    private Stack stack = new Stack();
    private Frame frame = new Frame(null);
    private Button up_butt = new Button.with_label("");
    private unowned Label up_label;
    private Label cur_label = new Label(null);
    private string? link_color;

    // Если мы перешли к пользовательскому виджету (level стал равен depth),
    // то мы должны запомнить предудыщий level, т.к. он совсем не обязательно равен level - 1,
    // ибо разные ветки дерева модели могут иметь разную длину.
    private int prev_level_for_widget;

    // Текуща глубина и выбранные пункты меню (по глубинам).
    private int depth = 0;
    private TreeIter[] iters;

    /**
    * Создание виджета MenuTree.
    * @param model Модель, описывающая стркутуру меню.
    * @param widget_column Номер колонки с пользовательскими виджетами для конечных точек дерева меню.
    * @param brief_column Номер колонки с коротким описанием пункта меню.
    * @param detailed_column Номер колонки с текстом для пункта меню (-1, если можно использовать brief_column).
    * @param icon_column Номер колонки с иконками для меню (-1, если нужно показывать корневое меню просто в виде строк).
    */
    public MenuTree(TreeModel model, int widget_column, int brief_column, int detailed_column = -1, int icon_column = -1)
      requires(widget_column >= 0)
      requires(brief_column >= 0)
    {
      Object(model:model, widget_column:widget_column, brief_column:brief_column, detailed_column:detailed_column, icon_column:icon_column);
    }


    private IconView create_iconview()
    {
      var iv = new IconView.with_model(model);

      iv.activate_on_single_click = true;
      iv.set_pixbuf_column(icon_column);

      if(detailed_column != -1)
        iv.set_text_column(detailed_column);
      else
        iv.set_text_column(brief_column);

      iv.item_activated.connect((path) => { on_menu_down(path); });
      iv.show();
      return iv;
    }


    private TreeView create_treeview()
    {
      var tv = new TreeView();
      tv.show_expanders = false;
      tv.headers_visible = false;
      tv.activate_on_single_click = true;

      var cr = new CellRendererText();
      cr.ypad = 8;

      if(detailed_column != -1)
        tv.insert_column_with_attributes(-1, "Text", cr, "text", detailed_column);
      else
        tv.insert_column_with_attributes(-1, "Text", cr, "text", brief_column);

      tv.row_activated.connect((path, column) => { on_menu_down(path); });
      tv.show();
      return tv;
    }


    private void on_menu_down(TreePath path)
    {
      var tv = stack.get_visible_child();
      var level = int.parse(stack.get_visible_child_name());

      TreeModel cur_model;
      tv.get("model", out cur_model);

      if(cur_model.get_iter(out iters[level], path))
      {
        Widget? user_widget;
        string brief;
        cur_model.get(iters[level], widget_column, out user_widget, brief_column, out brief);

        // Движение вниз в субменю.
        if(cur_model.iter_has_child(iters[level]))
        {
          level++;
          stack.set_visible_child_name(level.to_string());
          tv = stack.get_visible_child();
          var filtered_model = new TreeModelFilter(cur_model, path);
          tv.set_property("model", filtered_model);

          if(unlikely(user_widget != null))
            warning("User widget at path '%s' != null, but path has child", path.to_string());
        }
        else
        {
          // Движение вниз до пользовательского виджета.
          if(user_widget != null)
          {
            if(frame.get_child() != null)
              frame.remove(frame.get_child());

            // Такое может быть, если model уже используется в другом TreeMenu
            // или виджет используется в другой можели другого TreeMenu.
            // Тогда вытащим виджет из того контейнера, где он сейчас (перехватим его себе).
            if(user_widget.parent != null)
              user_widget.parent.remove(user_widget);

            frame.add(user_widget);
            prev_level_for_widget = level;
            stack.set_visible_child(frame);
            level++;
          }
          else
            warning("User widget at path '%s' == null and path has not child", path.to_string());
        }
      }
      else
        critical("Failed to get iter from path '%s'.", path.to_string());

      update_labels(level);
    }


    /**
    * Позволяет установить короткое название корня меню и надпись на кнопке перехода вверх в корневом меню.
    * @param root_brief Короткое описания корня меню.
    * @param root_up_brief Текст кнопки перехода вверх в корне меню.
    * @param root_detailed Длинное описание корня меню (выводится в тултипе).
    * @param root_up_detailed Текст тултипа кнопки перехода вверх в корне меню.
    */
    public void set_root_texts(string root_brief, string? root_up_brief = null, string? root_detailed = null, string? root_up_detailed = null)
    {
      this.root_brief = root_brief;
      this.root_up_brief = root_up_brief;
      this.root_detailed = root_detailed;
      this.root_up_detailed = root_up_detailed;
      update_labels(int.parse(stack.get_visible_child_name()));
    }

    public override void show_all()
    {
      if(get_no_show_all() == false)
      {
        model.foreach((m, path, iter) =>
        {
          Widget? user_widget;
          m.get(iter, widget_column, out user_widget);

          if(user_widget != null)
            user_widget.show_all();

          return false;
        });
      }

      base.show_all();
    }

    /**
    * Обновляет текст на кнопке вверх и надпись с текущем пунктом меню.
    * @param level Текущий уровень меню.
    */
    private void update_labels(int level)
    {
      string? cur_str, up_str;
      string? cur_tooltip, up_tooltip;
      TreeModel cur_model, up_model;

      if(level == depth)
        level = prev_level_for_widget + 1;

      switch(level)
      {
        case 0:
          cur_str = root_brief;
          cur_tooltip = root_detailed;
          up_str = root_up_brief;
          up_tooltip = root_up_detailed;
        break;

        case 1:
          up_str = root_brief;
          up_tooltip = root_detailed;
          stack.get_child_by_name((level - 1).to_string()).get("model", out cur_model);
          cur_model.get(iters[level - 1], brief_column, out cur_str, detailed_column, out cur_tooltip);
        break;

        default:
          stack.get_child_by_name((level - 1).to_string()).get("model", out cur_model);
          cur_model.get(iters[level - 1], brief_column, out cur_str, detailed_column, out cur_tooltip);
          stack.get_child_by_name((level - 2).to_string()).get("model", out up_model);
          up_model.get(iters[level - 2], brief_column, out up_str, detailed_column, out up_tooltip);
        break;
      }

      if(cur_str != null)
      {
        cur_label.set_markup(Markup.printf_escaped("<b>%s</b>", cur_str));

        if(cur_tooltip != null)
          cur_label.set_tooltip_text(cur_tooltip);
        else
          cur_label.has_tooltip = false;

        if(cur_label.parent == null)
          attach(cur_label, 1, 0, 1, 1);
      }
      else
        if(cur_label.parent != null)
          remove(cur_label);

      if(up_str != null)
      {
        if(up_tooltip != null)
          up_butt.set_tooltip_text(up_tooltip);
        else
          up_butt.has_tooltip = false;

        if(unlikely(link_color == null))
        {
          Gdk.Color col = { 0 };

          #if VALA_0_26
            Gdk.RGBA rgba = up_label.get_style_context().get_color(StateFlags.LINK);
            col.red   = (uint16)(65535 * rgba.red);
            col.green = (uint16)(65535 * rgba.green);
            col.blue  = (uint16)(65535 * rgba.blue);
          #else
            up_label.style_get("link-color", out col);
          #endif

          link_color = col.to_string();
        }

        if(level != 0)
          up_label.set_markup(Markup.printf_escaped("<span color='%s'>%c<small>%s</small></span>", link_color, '<', up_str));
        else
          up_label.set_markup(Markup.printf_escaped("<span color='%s'>%s</span>", link_color, up_str));

        if(up_butt.parent == null)
          attach(up_butt, 0, 0, 1, 1);
      }
      else
        if(up_butt.parent != null)
          remove(up_butt);
    }

    construct
    {
      attach(stack, 0, 1, 2, 1);

      model.foreach((model, path, iter) =>
      {
        depth = int.max(depth, path.get_depth());
        return false;
      });
      if(depth == 0)
        depth = 1;

      model.row_inserted.connect((path, iter) =>
      {
        var new_depth = path.get_depth();

        if(new_depth > depth)
        {
          iters.resize(depth + 1);
          stack.remove(frame);

          for(int i = depth; i < new_depth; i++)
            stack.add_named(create_treeview(), i.to_string());

          stack.add_named(frame, new_depth.to_string());

          depth = new_depth;
        }
      });

      iters = new TreeIter[depth + 1];

      for(int i = 0; i < depth; i++)
      {
        if(i != 0 || icon_column == -1)
          stack.add_named(create_treeview(), i.to_string());
        else
          stack.add_named(create_iconview(), i.to_string());

      }

      { // Место под пользовательский виджет -->
        stack.add_named(frame, depth.to_string());
        frame.remove.connect((widget) =>
        {
          // Удаляют пользовательский виджет из видимого фрейма.
          // Такое может быть, если есть еще одно TreeMenu,
          // и там открыли виджет, который уже октрыт у нас и перехватили его.
          // Это норма: у нас тут просто перейдем вверх по меню.
          if(stack.get_visible_child() == frame)
          {
            stack.set_visible_child_name(prev_level_for_widget.to_string());
            update_labels(prev_level_for_widget);
          }
        });
      } // Место под пользовательский виджет <--

      { // По умолчанию нужно отобразить корень меню -->
        var first = stack.get_child_by_name("0");
        first.set_property("model", model);
        stack.set_visible_child(first);
        update_labels(0);
      } // По умолчанию нужно отобразить корень меню <--

      cur_label.show();
      cur_label.hexpand = true;
      cur_label.ellipsize = Pango.EllipsizeMode.END;
      up_butt.show();
      up_butt.relief = ReliefStyle.NONE;
      up_label = up_butt.get_child() as Label;
      up_butt.clicked.connect(() =>
      {
        var level = int.parse(stack.get_visible_child_name());

        if(level != 0)
        {
          if(level == depth)
            level = prev_level_for_widget;
          else
            level--;

          stack.set_visible_child_name(level.to_string());

          update_labels(level);
        }
        else
          root_up_clicked();
      });

      stack.transition_type = StackTransitionType.SLIDE_LEFT_RIGHT;
      stack.show_all();
    }
  }
  #endif
}

