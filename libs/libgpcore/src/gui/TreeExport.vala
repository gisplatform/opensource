/*
 * GpTreeExport is a library for GtkTreeView or GtkTreeModel contents export as text.
 *
 * Copyright (C) 2016 Sergey Volkhin.
 *
 * GpTreeExport is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpTreeExport is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpTreeExport. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using Gtk;


namespace Gp
{
  /**
  * Экспортирует содержимое GtkTreeModel в текстовый файл.
  *
  * Если @columns_names не равен null, то первой строкой в файле будут идти имена колонок.
  * В таком случае размер массива @columns должен быть раван размеру массива @columns_names.
  * Если же @columns_names равен null, то в файл будут записаны только строки с данными.
  *
  * @param model Модель, данные из которой нужно экспортировать.
  * @param dos Файл, в который писать данные.
  * @param separator Разделитель столбцов в файле.
  * @param columns Идентификаторы колонок, которые нужно экспортировать.
  * @param columns_names Имена колонок, соответвующие идентификаторам из массива @columns.
  */
  public void tree_model_export(TreeModel model, DataOutputStream dos, string separator,
    int[] columns, string[]? columns_names = null) throws IOError
  {
    if(columns_names != null)
    {
      return_if_fail(columns.length == columns_names.length);

      foreach(var c in columns_names)
      {
        dos.put_string(c);
        dos.put_string(separator);
      }

      dos.put_byte('\n');
    }

    TreeIter iter;

    if(model.get_iter_first(out iter))
      do
      {
        foreach(var column in columns)
        {
          Value value, string_value = Value(typeof(string));
          model.get_value(iter, column, out value);

          if(value.transform(ref string_value))
            dos.put_string((string)string_value);
          else
            dos.put_string("FAILED");

          dos.put_string(separator);
        }

        dos.put_byte('\n');
      }
      while(model.iter_next(ref iter));
  }


  /**
  * Экспортирует содержимое GtkTreeModel в текстовый файл.
  *
  * Функция сама показывает все необходимые диалоги GUI.
  *
  * @param model Модель, данные из которой нужно экспортировать.
  * @param widget_in_parent Любой виджет на окне, поверх которого необходимо показывать окна GUI.
  * @param columns Идентификаторы колонок, которые нужно экспортировать.
  * @param columns_names Имена колонок, соответвующие идентификаторам из массива @columns.
  */
  public void tree_model_export_gui(TreeModel model, Widget widget_in_parent,
    int[] columns, string[] columns_names)
  {
    var dialog = new FileChooserDialog(_("Select file export to"), null, FileChooserAction.SAVE,
       _("_Cancel"), ResponseType.CANCEL, _("_Export"), ResponseType.ACCEPT);
    dialog.set_current_folder(Environment.get_home_dir());
    dialog.set_current_name(_("new_file.txt"));
    dialog_customize(dialog, widget_in_parent);

  #if ! GTK2
    var hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 5);
  #else
    var hbox = new HBox(false, 5);
  #endif

    var expander = new Expander(_("File format"));
    expander.add(hbox);

    (dialog.get_content_area() as Box).add(expander);

    hbox.pack_start(new Label(_("Separator between columns: ")), false, false);

    var separ = new Gtk.Entry();
    separ.text = "\t";
    hbox.pack_start(separ, false, false);

  #if ! GTK2
    hbox.pack_start(new Gtk.Separator(Gtk.Orientation.VERTICAL), false, false);
  #else
    hbox.pack_start(new Gtk.VSeparator(), false, false);
  #endif

    var with_columns_names = new CheckButton.with_label(_("Add columns names"));
    with_columns_names.active = true;
    hbox.pack_start(with_columns_names, false, false);

    expander.show_all();

    if(dialog.run() == ResponseType.ACCEPT)
    {
      try
      {
        FileOutputStream fos;
        var file = dialog.get_file();

        try
        {
          fos = file.create(FileCreateFlags.NONE);
        }
        catch(IOError e)
        {
          if(e is IOError.EXISTS && dialog_question(_("File already exists, overwrite it?"), dialog))
            fos = file.replace(null, false, FileCreateFlags.NONE);
          else
            throw e;
        }

        tree_model_export(model, new DataOutputStream(fos), separ.text, columns,
          with_columns_names.active ? columns_names : null);

        dialog_info(_("Data has been successfully exported to file ") + file.get_path(), dialog);
      }
      catch(Error e)
      {
        dialog_fail(e.message, dialog);
      }
    }

    dialog.destroy();
  }


  /**
  * Функция получения списка колонок GtkTreeView: имен и идентификаторов в GtkTreeModel.
  *
  * Функция предполагает, что в каждой колонке есть один и только один CellRenderer.
  * Если в какой-то колонке более одного CellRenderer'а,
  * то будет возвращен идентификатор в GtkTreeModel, соответвующий первому CellRenderer'у.
  *
  * @param tree_view Виджет GtkTreeView.
  * @param columns_out Идентификаторы колонок в GtkTreeModel.
  * @param columns_names_out Имена колонок, соответвующие идентификаторам из массива @columns.
  */
  public void tree_view_get_columns(TreeView tree_view, out int[] columns_out, out string[] columns_names_out)
  {
    var columns = new int[0];
    var columns_names = new string[0];

  #if ! GTK2
  #if VALA_0_26
    var columns_list = tree_view.get_columns();

    foreach(unowned TreeViewColumn c in columns_list)
    {
      // Получаем список CellRenderer'ов для колонки.
      var cells = c.get_cells();

      // Берем первый же CellRenderer из списка, надеемся, что он нужный (обычно он в колонке один и есть).
      if(cells.data != null)
      {
        // Пробуем найти атрибут "text".
        var column_id = c.cell_area.attribute_get_column(cells.data, "text");

        // Не получилось "text", -- ищем "active".
        if(column_id == -1)
          column_id = c.cell_area.attribute_get_column(cells.data, "active");

        if(column_id != -1)
        {
          columns += column_id;
          columns_names += c.title;
        }
      }
    }
  #endif
  #endif

    columns_out = columns;
    columns_names_out = columns_names;
  }


  /**
  * Экспортирует все содержимое GtkTreeView в текстовый файл плюс (опционально) дополнительные колонки из модели.
  *
  * Функция сама показывает все необходимые графические диалоги.
  *
  * @param tree_view Таблица, строки которой нужно экспортировать.
  * @param ... Опциональный список "имя колонки" - "идентификатор колонки", которые нужно экспортировать плюс к тем колонкам, которые есть в GtkTreeView.
  */
  public void tree_view_export(TreeView tree_view, ...)
  {
    int[] columns;
    string[] columns_names;
    tree_view_get_columns(tree_view, out columns, out columns_names);

    { // Парсим дополнительные колонки, если есть -->
      var l = va_list();

      while(true)
      {
        string? column_name = l.arg();

        if(column_name == null)
          break;

        int column_id = l.arg();

        columns += column_id;
        columns_names += column_name;
      }
    } // Парсим дополнительные колонки, если есть <--

    tree_model_export_gui(tree_view.model, tree_view, columns, columns_names);
  }


  /**
  * Создает меню правого клика для GtkTreeView с пунктом экспорта в текстовый файл.
  *
  * Функция сама показывает все необходимые графические диалоги по клику на пункт экспорта.
  *
  * @param tree_view Таблица, по клику на которой нужно показывать меню.
  * @param ... Опциональный список "имя колонки" - "идентификатор колонки", которые нужно экспортировать плюс к тем колонкам, которые есть в GtkTreeView.
  * @return Ссылка на меню, куда пользователь может добавить свои пункты.
  */
  public unowned Gtk.Menu tree_view_export_menu(TreeView tree_view, ...)
  {
    int[] columns;
    string[] columns_names;
    tree_view_get_columns(tree_view, out columns, out columns_names);

    { // Парсим дополнительные колонки, если есть -->
      var l = va_list();

      while(true)
      {
        string? column_name = l.arg();

        if(column_name == null)
          break;

        int column_id = l.arg();

        columns += column_id;
        columns_names += column_name;
      }
    } // Парсим дополнительные колонки, если есть <--

    var menu = new Gtk.Menu();

    var item = new Gtk.MenuItem.with_label(_("Export to file"));
    item.show();
    menu.append(item);
    item.activate.connect(() =>
    {
      tree_model_export_gui(tree_view.model, tree_view, columns, columns_names);
    });

    tree_view.button_press_event.connect((widget, ev) =>
    {
      if(ev.button == 3)
      {
        menu.popup(null, null, null, ev.button, ev.time);
        return true;
      }
      else
        return false;
    });

    return menu as unowned Gtk.Menu;
  }
}

