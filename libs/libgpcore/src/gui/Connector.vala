/*
 * GpConnector is a client connection GUI library.
 *
 * Copyright (C) 2015 Alexey Pankratov, Sergey Volkhin.
 *
 * GpConnector is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpConnector is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpConnector. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using Gtk;

namespace Gp
{
  #if GLIB_2_40
  private class ServerList : Box
  {
    const int TREE_VIEW_HEIGHT = 100;
    Gtk.ListStore store;
    TreeView treeview;
    CellRenderer[] renderers;

    public signal void row_activated(TreeIter iter);
    public signal void row_selected(TreeView treeview);
    public signal void toggled(TreeIter iter, int column, bool active);
    public signal void edited(TreeIter iter, int column, string text);

    public ServerList(Type[] types, string[] names)
    {
      this.set_orientation(Orientation.VERTICAL);

      var scrolled_win = new ScrolledWindow(null, null);
      scrolled_win.set_policy(PolicyType.AUTOMATIC, PolicyType.AUTOMATIC);
      scrolled_win.set_size_request(-1, TREE_VIEW_HEIGHT);
      this.pack_start(scrolled_win, true, true, 0);

      store = new Gtk.ListStore.newv(types);
      treeview = new TreeView();
      treeview.set_model(store);

      scrolled_win.add(treeview);

      for(int i = 0; i < types.length; i++)
      {
        if(types[i] == typeof(bool))
        {
          var renderer = new CellRendererStar();
          treeview.insert_column_with_attributes(-1, _(names[i]), renderer, "active", i);
          renderers += renderer;
          renderer.toggled.connect((toggle, path_str) =>
          {
            TreePath path = new TreePath.from_string(path_str);
            TreeIter iter;
            store.get_iter(out iter, path);
            for(int j = 0; j < renderers.length; j++)
            {
              if(renderer == renderers[j])
              {
                this.toggled(iter, j, !toggle.active);
                store.set(iter, j, !toggle.active);
                break;
              }
            }
          });
        }
        else if(types[i] == typeof(string))
        {
          var renderer = new CellRendererText();
          treeview.insert_column_with_attributes(-1, _(names[i]), renderer, "text", i);
          renderers += renderer;
        }
        else
        {
          warning("ServerList: type not supported");
        }
      }

      tree_view_export_menu(treeview);

      treeview.row_activated.connect((path) =>
      {
        TreeIter iter;
        store.get_iter(out iter, path);
        row_activated(iter);
      });

      var tree_selection = treeview.get_selection();
      tree_selection.changed.connect(() =>
      {
        row_selected(treeview);
      });
    }

    public void append(int[] columns, Value[] values)
    {
      TreeIter iter;
      store.append(out iter);
      store.set_valuesv(iter, columns, values);
    }

    public bool find(int column, Value value, out TreeIter iter_out)
    {
      TreeIter iter;
      for(bool next = store.get_iter_first(out iter); next; next = store.iter_next(ref iter))
      {
        Value tmp;
        store.get_value(iter, column, out tmp);
        if((string)tmp == (string)value)
        {
          iter_out = iter;
          return true;
        }
      }
      iter_out = TreeIter();
      return false;
    }

    public void remove_with_value(int column, Value value)
    {
      TreeIter iter;
      for(bool next = store.get_iter_first(out iter); next; next = store.iter_next(ref iter))
      {
        Value tmp;
        store.get_value(iter, column, out tmp);
        if((string)tmp == (string)value)
        {
          store.remove(iter);
        }
      }
    }

    public void get_value(TreeIter iter, int column, out Value value)
    {
      store.get_value(iter, column, out value);
    }

    public void set_value(TreeIter iter, int column, Value value)
    {
      store.set(iter, column, (bool)value, -1);
    }

    public void set_editable(int column)
    {
      (renderers[column] as CellRendererText).editable = true;
      (renderers[column] as CellRendererText).edited.connect((path_str, text) =>
      {
        TreePath path = new TreePath.from_string(path_str);
        TreeIter iter;
        store.get_iter(out iter, path);
        store.set_value(iter, column, text);
        edited(iter, column, text);
      });
    }
  }

  /**
   * Класс, реализующий виджет подключения к БД с последними и сохранёнными серверами
   * (а так же реализованы функции для найденых серверов).
   */
  public class Connector : Box
  {
    Gp.Bookmark bookmarks;
    Gtk.ListStore bookmarks_list_store;
    ServerList found_servers;

    public signal bool activated(string hostname);
    public signal void file_error(FileError error);

    enum FoundColumns
    {
      BOOKMARK,
      ADDRESS
    }

    enum RecentColumns
    {
      BOOKMARK,
      ADDRESS
    }

    enum SavedColumns
    {
      BOOKMARK,
      ADDRESS,
      DESCRIPTION
    }

    /**
     * Функция добавления найденого сервера в список.
     *
     * @param address Адрес сервера
     */
    public void add_found(string address)
    {
      if(address in bookmarks.get_saved())
        found_servers.append({FoundColumns.BOOKMARK, FoundColumns.ADDRESS},
                             {true                 , address});
      else
        found_servers.append({FoundColumns.BOOKMARK, FoundColumns.ADDRESS},
                             {false                , address});
      bookmarks_list_store.insert_with_values(null, -1, 0, address, -1);
    }

    /**
     * Функция удаления найденого сервера из списка.
     *
     * @param address Адрес сервера
     */
    public void remove_found(string address)
    {
      found_servers.remove_with_value(FoundColumns.ADDRESS, address);
      TreeIter iter;
      for(bool next = bookmarks_list_store.get_iter_first(out iter); next; next = bookmarks_list_store.iter_next(ref iter))
      {
        Value tmp_address;
        bookmarks_list_store.get_value(iter, 0, out tmp_address);
        if((string)tmp_address == (string)address)
        {
          bookmarks_list_store.remove(iter);
          break;
        }
      }
    }

    /**
     * Конструктор.
     *
     * @param config_path Папка для хранения конфигурационных файлов.
     */
    public Connector(string config_path)
    {
      this.set_orientation(Orientation.VERTICAL);

      try
      {
        bookmarks = new Gp.Bookmark(config_path, "bookmarks.ini");
      }
      catch(FileError error)
      {
        file_error(error);
      }

      var hbox = new Box(Orientation.HORIZONTAL, 10);
      this.pack_start(hbox, false, false, 0);

      var entry = new Entry();
      hbox.pack_start(entry, true, true, 0);

      var stack = new Stack();
      var stack_switcher = new StackSwitcher();
      stack_switcher.set_stack(stack);
      var stack_switcher_box = new Box(Orientation.HORIZONTAL, 10);
      stack_switcher_box.pack_start(stack_switcher, true, false, 0);

      var butt = new Button.with_label(_("Connect"));
      found_servers = new ServerList({typeof(bool) , typeof(string)},
                                     {_("Bookmark"), _("Address")});
      var recent_servers = new ServerList({typeof(bool) , typeof(string)},
                                          {_("Bookmark"), _("Address")});
      var saved_servers = new ServerList({typeof (bool) , typeof (string), typeof (string)},
                                         {_("Bookmark") , _("Address")   , _("Description")});

      stack.add_titled(found_servers, "Found", _("Found"));
      stack.add_titled(recent_servers, "Recent", _("Recent"));
      stack.add_titled(saved_servers, "Saved", _("Saved"));

      this.pack_start(stack_switcher_box, false, false, 10);
      this.pack_start(stack, true, true, 0);

      var entry_completion = new EntryCompletion();
      entry.set_completion(entry_completion);
      bookmarks_list_store = new Gtk.ListStore (1, typeof (string));
      entry_completion.set_model (bookmarks_list_store);
      entry_completion.set_text_column (0);

      string[] bookmarks_list_saved = bookmarks.get_saved();
      string[] desc = bookmarks.get_saved_description();
      for(int i = 0; i < bookmarks_list_saved.length; i++)
      {
        bookmarks_list_store.insert_with_values(null, -1, 0, bookmarks_list_saved[i], -1);
        saved_servers.append({SavedColumns.BOOKMARK, SavedColumns.ADDRESS   , SavedColumns.DESCRIPTION},
                             {true                 , bookmarks_list_saved[i], desc[i]});
      }

      string[] bookmarks_list_recent = bookmarks.get_recent();
      for(int i = 0; i < bookmarks_list_recent.length; i++)
      {
        bookmarks_list_store.insert_with_values(null, -1, 0, bookmarks_list_recent[i], -1);
        if(bookmarks_list_recent[i] in bookmarks_list_saved)
        {
          recent_servers.append({RecentColumns.BOOKMARK, RecentColumns.ADDRESS},
                                {true                  , bookmarks_list_recent[i]});
        }
        else
        {
          recent_servers.append({RecentColumns.BOOKMARK, RecentColumns.ADDRESS},
                                {false                 , bookmarks_list_recent[i]});
        }
      }

      var saved_add = new Button.with_label(_("_Save"));

      hbox.pack_start(butt, false, false, 0);
      hbox.pack_start(saved_add, false, false, 0);

      butt.sensitive = (entry.text != "");
      saved_add.sensitive = (entry.text != "");

      saved_add.clicked.connect(() =>
      {
        if( !(entry.get_text() in bookmarks.get_saved()) )
        {
          try
          {
            bookmarks.add_saved(entry.get_text());
          }
          catch(FileError error)
          {
            file_error(error);
          }
          saved_servers.append({SavedColumns.BOOKMARK, SavedColumns.ADDRESS},
                               {true                 , entry.get_text()});
          TreeIter iter;
          if(recent_servers.find(RecentColumns.ADDRESS, entry.get_text(), out iter))
          {
            recent_servers.set_value(iter, RecentColumns.BOOKMARK, true);
          }

          if(found_servers.find(FoundColumns.ADDRESS, entry.get_text(), out iter))
          {
            found_servers.set_value(iter, FoundColumns.BOOKMARK, true);
          }
        }
      });

      found_servers.row_selected.connect((treeview) =>
      {
        TreeIter iter;
        TreeModel tree_model;
        TreeSelection selection = treeview.get_selection();
        if(selection.get_selected(out tree_model, out iter) != false)
        {
          GLib.Value address;
          found_servers.get_value(iter, RecentColumns.ADDRESS, out address);
          entry.set_text((string) address);
        }
      });

      found_servers.row_activated.connect((iter) =>
      {
        butt.clicked();
      });

      found_servers.toggled.connect((iter, column, active) =>
      {
        if(!active)
        {
          Value address;
          found_servers.get_value(iter, FoundColumns.ADDRESS, out address);
          try
          {
            bookmarks.remove_saved((string) address);
          }
          catch(FileError error)
          {
            file_error(error);
          }
          saved_servers.remove_with_value(SavedColumns.ADDRESS, address);
          TreeIter tmp_iter;
          if(recent_servers.find(RecentColumns.ADDRESS, address, out tmp_iter))
          {
            recent_servers.set_value(tmp_iter, RecentColumns.BOOKMARK, false);
          }
        }
        else
        {
          Value address;
          found_servers.get_value(iter, FoundColumns.ADDRESS, out address);
          try
          {
            bookmarks.add_saved((string) address);
          }
          catch(FileError error)
          {
            file_error(error);
          }
          TreeIter tmp_iter;
          if(saved_servers.find(SavedColumns.ADDRESS, address, out tmp_iter))
          {
            saved_servers.set_value(tmp_iter, SavedColumns.BOOKMARK, true);
          }
          else
          {
            saved_servers.append({SavedColumns.BOOKMARK, SavedColumns.ADDRESS, SavedColumns.DESCRIPTION},
                                 {true                 , address             , ""});
          }
          if(recent_servers.find(RecentColumns.ADDRESS, address, out tmp_iter))
          {
            recent_servers.set_value(tmp_iter, RecentColumns.BOOKMARK, true);
          }
        }
      });

      recent_servers.row_selected.connect((treeview) =>
      {
        TreeIter iter;
        TreeModel tree_model;
        TreeSelection selection = treeview.get_selection();
        if(selection.get_selected(out tree_model, out iter) != false)
        {
          GLib.Value address;
          recent_servers.get_value(iter, RecentColumns.ADDRESS, out address);
          entry.set_text((string) address);
        }
      });

      recent_servers.row_activated.connect((iter) =>
      {
        butt.clicked();
      });

      recent_servers.toggled.connect((iter, column, active) =>
      {
        if(!active)
        {
          Value address;
          recent_servers.get_value(iter, RecentColumns.ADDRESS, out address);
          try
          {
            bookmarks.remove_saved((string) address);
          }
          catch(FileError error)
          {
            file_error(error);
          }
          saved_servers.remove_with_value(SavedColumns.ADDRESS, address);
          TreeIter tmp_iter;
          if(found_servers.find(FoundColumns.ADDRESS, address, out tmp_iter))
          {
            found_servers.set_value(tmp_iter, FoundColumns.BOOKMARK, false);
          }
        }
        else
        {
          Value address;
          recent_servers.get_value(iter, RecentColumns.ADDRESS, out address);
          try
          {
            bookmarks.add_saved((string) address);
          }
          catch(FileError error)
          {
            file_error(error);
          }
          TreeIter tmp_iter;
          if(saved_servers.find(SavedColumns.ADDRESS, address, out tmp_iter))
          {
            saved_servers.set_value(tmp_iter, SavedColumns.BOOKMARK, true);
          }
          else
          {
            saved_servers.append({SavedColumns.BOOKMARK, SavedColumns.ADDRESS, SavedColumns.DESCRIPTION},
                                 {true                 , address             , ""});
          }
          if(found_servers.find(FoundColumns.ADDRESS, address, out tmp_iter))
          {
            found_servers.set_value(tmp_iter, FoundColumns.BOOKMARK, true);
          }
        }
      });

      saved_servers.row_selected.connect((treeview) =>
      {
        TreeIter iter;
        TreeModel tree_model;
        TreeSelection selection = treeview.get_selection();
        if(selection.get_selected(out tree_model, out iter) != false)
        {
          GLib.Value address;
          saved_servers.get_value(iter, RecentColumns.ADDRESS, out address);
          entry.set_text((string) address);
        }
      });

      saved_servers.row_activated.connect((iter) =>
      {
        butt.clicked();
      });

      saved_servers.toggled.connect((iter, column, active) =>
      {
        if(!active)
        {
          Value address;
          saved_servers.get_value(iter, SavedColumns.ADDRESS, out address);
          try
          {
            bookmarks.remove_saved((string) address);
          }
          catch(FileError error)
          {
            file_error(error);
          }
          TreeIter tmp_iter;
          if(recent_servers.find(RecentColumns.ADDRESS, address, out tmp_iter))
          {
            recent_servers.set_value(tmp_iter, RecentColumns.BOOKMARK, false);
          }
          if(found_servers.find(FoundColumns.ADDRESS, entry.get_text(), out tmp_iter))
          {
            found_servers.set_value(tmp_iter, FoundColumns.BOOKMARK, false);
          }
        }
        else
        {
          Value address;
          Value description;
          saved_servers.get_value(iter, SavedColumns.ADDRESS, out address);
          saved_servers.get_value(iter, SavedColumns.DESCRIPTION, out description);
          try
          {
            bookmarks.add_saved((string) address);
            bookmarks.set_saved_description((string) address, (string) description);
          }
          catch(FileError error)
          {
            file_error(error);
          }
          TreeIter tmp_iter;
          if(recent_servers.find(RecentColumns.ADDRESS, address, out tmp_iter))
          {
            recent_servers.set_value(tmp_iter, RecentColumns.BOOKMARK, true);
          }
          if(found_servers.find(FoundColumns.ADDRESS, address, out tmp_iter))
          {
            found_servers.set_value(tmp_iter, FoundColumns.BOOKMARK, true);
          }
        }
      });

      saved_servers.set_editable(SavedColumns.DESCRIPTION);
      saved_servers.edited.connect((iter, column, text) =>
      {
        GLib.Value address;
        saved_servers.get_value(iter, SavedColumns.ADDRESS, out address);
        try
        {
          bookmarks.set_saved_description((string) address, text);
        }
        catch(FileError error)
        {
          file_error(error);
        }
      });

      entry.activate.connect(() => { if(entry.text != "") butt.clicked(); });
      entry.changed.connect (() =>
      {
        butt.sensitive = (entry.text != "");
        saved_add.sensitive = (entry.text != "");
      });
      butt.clicked.connect(() =>
      {
        if(!activated(entry.get_text()))
        {
          if(!(entry.get_text() in bookmarks.get_recent()))
          {
            try
            {
              bookmarks.add_recent(entry.get_text());
            }
            catch(FileError error)
            {
              file_error(error);
            }
            bookmarks_list_store.insert_with_values(null, -1, 0, entry.get_text(), -1);

            if(entry.get_text() in bookmarks.get_saved())
            {
              recent_servers.append({RecentColumns.BOOKMARK, RecentColumns.ADDRESS},
                                    {true                  , entry.get_text()});
            }
            else
            {
              recent_servers.append({RecentColumns.BOOKMARK, RecentColumns.ADDRESS},
                                    {false                 , entry.get_text()});
            }
          }
        }
      });
    }
  }
#endif
}

