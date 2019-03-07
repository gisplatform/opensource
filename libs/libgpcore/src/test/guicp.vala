using Gtk;


void run(Window parent, File src, File dest)
{
  var cancellable = new Cancellable();
  var dialog = new MessageDialog(parent, DialogFlags.DESTROY_WITH_PARENT,
    MessageType.INFO, ButtonsType.CANCEL, "Copying...");

  { // Кладем в Box spinner -->
    var spinner = new Spinner();

    // FIXME Костыль, чтобы собиралось GTK3 + C++.
    (dialog.get_child() as Box).add(spinner);

    spinner.start();
    spinner.show();
  } // Кладем в Box spinner <--

  try
  {
    // Тред с копированием файлов -->
    var thread = new Thread<int>.try("recursive_copy_thread", () =>
    {
      try
      {
        Gp.recursive_copy(src, dest, FileCopyFlags.NONE, cancellable);
      }
      catch(Error e)
      {
        string error_text = e.message;

        // Если во время копирования было брошено исключение --
        // покажем диалог с ошибкой с подробностями из исключения.
        GLib.Idle.add(() =>
        {
          var fail_dialog = new MessageDialog(dialog, 0,
            MessageType.WARNING, ButtonsType.OK, "Failed to copy: %s", error_text);
          fail_dialog.run();
          fail_dialog.destroy();
          return false;
        });
      }

      // Как файлы скопируются -- убираем диалог со спиннером.
      GLib.Idle.add(() =>
      {
        dialog.destroy();
        return false;
      });

      return 0;
    });
    // Тред с копированием файлов <--

    // При нажатии Cancel -- отменяем копирование.
    dialog.response.connect(() => { cancellable.cancel(); });

    // Запускаем диалог со спиннером.
    dialog.run();

    // Диалог отработал -- значит и тред можно джойнить.
    thread.join();
  }
  catch(Error e)
  {
    critical("Failed to create recursive_copy_thread");
  }

}


int main(string[] args)
{
  if(args.length != 3)
  {
    warning("Usage: %s SRC DEST\n", args[0]);
    return 0;
  }

  Gtk.init(ref args);

  var window = new Window();
  window.title = "My Gtk.MessageDialog";
  window.window_position = WindowPosition.CENTER;
  window.destroy.connect(main_quit);
  window.set_default_size(350, 70);

  var button = new Button.with_label("Run copying!");
  window.add(button);

  button.clicked.connect(() => { run(window, File.new_for_commandline_arg(args[1]), File.new_for_commandline_arg(args[2])); });

  window.show_all();

  Gtk.main();

  return 0;
}

