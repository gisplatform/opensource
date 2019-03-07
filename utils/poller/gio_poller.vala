// Sysfs poll listener. Based on poller.c and
// http://stackoverflow.com/questions/16367623/using-the-linux-sysfs-notify-call
// by Vilhelm Gray (and thanks to http://stackoverflow.com/users/1401351/peter).

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

  IOChannel channel;

  try
  {
    channel = new IOChannel.file(args[1], "r");
  }
  catch(FileError e)
  {
    critical("Failed to open channel: %s", e.message);
    return -2;
  }

  var src = channel.create_watch(IOCondition.PRI | IOCondition.ERR);

  src.set_callback((source, condition) =>
  {
    string str_return = null;

    try
    {
      size_t length, terminator_pos;
      channel.seek_position(0, SeekType.SET);
      channel.read_line(out str_return, out length, out terminator_pos);
    }
    catch(Error e)
    {
      critical("Failed to read line: %s", e.message);
    }

    stdout.printf("'%s' triggered with cond = 0x%x, str = %s", Path.get_basename(args[1]), condition, str_return);
    stdout.flush();

    return true;
  });


  {
    var loop = new MainLoop();
    src.attach(loop.get_context());
    loop.run();
  }

  return 0;
}

