// Нерабочий вариант!
// Здесь нет чтения, файл переоткрывается,
// а после открытия он каждый раз видится с выставленным POLL.
// Как результат, все циклится.

public class Poller : Object
{
  public string file_path { public get; construct; }
  public MainLoop loop { public get; construct; }

  private IOChannel channel;

  public Poller(string file_path)
  {
    Object(file_path: file_path, loop: new MainLoop());
  }
  
  public bool reopen_file()
  {
    try
    {
      this.channel = new IOChannel.file(this.file_path, "r");
      var src = this.channel.create_watch(IOCondition.PRI | IOCondition.ERR);
      src.set_callback(this.reopen_file);
      src.attach(this.loop.get_context());
      print("'%s' triggered!\n", Path.get_basename(this.file_path));
    }
    catch(FileError e)
    {
      critical("Failed to open channel: %s", e.message);
      this.loop.quit();
    }

    return false;
  }
}

int main(string[] args)
{
  if(args.length != 2)
  {
    print("Poll event listener (based on libgio).\n");
    print("Usage: %s <path to file to poll on>\n", args[0]);
    return -1;
  }

  // Костыль, чтобы запускать и под старыми версиями GLib.
  Dualit.legacy_type_init();

  var poller = new Poller(args[1]);

  {
    var src = new IdleSource();
    src.set_callback(poller.reopen_file);
    src.attach(poller.loop.get_context());
  }

  poller.loop.run();

  return 0;
}

