// Based on https://wiki.gnome.org/Projects/Vala/GTKSample#TreeView_with_ListStore

using Gtk;

public class TreeViewSample : Window
{
  public TreeViewSample()
  {
    this.title = "TreeView Sample";
    set_default_size(250, 100);

    var box = new Gtk.Box (Gtk.Orientation.VERTICAL, 5);
    add(box);

    var view = new TreeView();
    setup_treeview(view);
    box.add(view);

    Gp.tree_view_export_menu(view, "Invisible", 4);

    this.destroy.connect(Gtk.main_quit);
  }

  private void setup_treeview(TreeView view)
  {
    var listmodel = new Gtk.ListStore(5, typeof(string), typeof(string), typeof(string), typeof(string), typeof(int));
    view.set_model(listmodel);

    view.insert_column_with_attributes(-1, "Account Name", new CellRendererText (), "text", 0);
    view.insert_column_with_attributes(-1, "Type", new CellRendererText (), "text", 1);

    var cell = new CellRendererText();
    cell.set("foreground_set", true);
    view.insert_column_with_attributes(-1, "Balance", cell, "text", 2, "foreground", 3);

    TreeIter iter;
    listmodel.append(out iter);
    listmodel.set(iter, 0, "My Visacard", 1, "card", 2, "102,10", 3, "red", 4, 234);

    listmodel.append(out iter);
    listmodel.set(iter, 0, "My Mastercard", 1, "card", 2, "10,20", 3, "red", 4, 456);
  }

  public static int main (string[] args)
  {
    Gtk.init(ref args);

    var prog_bin_dir = GLib.Path.get_dirname(args[0]);
    var prog_root_dir = GLib.Path.build_filename(prog_bin_dir, "..", null);
    var locale_path = GLib.Path.build_filename(prog_root_dir, "share", "locale", null);
    GLib.Intl.bindtextdomain("libgpcore", locale_path);

    var sample = new TreeViewSample();
    sample.show_all();
    Gtk.main();

    return 0;
  }
}
