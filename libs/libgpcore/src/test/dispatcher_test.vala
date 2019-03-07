
// Тест DispatcherLoop и DispatcherThreadPool.
// См. --help.

#if VALA_0_18
class WorkerExample : Object, Gp.Worker
{
  public string thread_name { private set; get; }
  public int x_times { private set; get; }

  public WorkerExample(string name, int x)
  {
    this.thread_name = name;
    this.x_times = x;
  }

  public void run()
  {
    for(int i = 0; i < this.x_times ; i++)
    {
      stdout.printf ("[%"+ int64.FORMAT + "] %s: %d/%d\n",
        (new DateTime.now_utc()).to_unix(), this.thread_name, i + 1, this.x_times);
      Thread.usleep(100000); // wait 0.1 second
    }
  }
}
#endif

public class Main : Object
{
#if VALA_0_18
  private static int threads_num = 0;
  private static Gp.Dispatcher dispatcher;

  private const GLib.OptionEntry[] options =
  {
    { "threads-num", 'n', 0, OptionArg.INT, ref threads_num, "Number of threads", null },
    { null }
  };

#endif
  public static int main (string[] args)
  {
#if VALA_0_18
    try
    {
      var opt_context = new OptionContext ("- Dispatcher test");
      opt_context.set_help_enabled(true);
      opt_context.add_main_entries(options, null);
      opt_context.parse (ref args);
    }
    catch(OptionError e)
    {
      warning("error: %s\n", e.message);
      warning("Run '%s --help' to see a full list of available command line options.\n", args[0]);
      return -1;
    }

    try
    {
      if(threads_num == 0)
        dispatcher = new Gp.DispatcherLoop();
      else
        dispatcher = new Gp.DispatcherThreadPool(threads_num, false);

      // Assign some tasks:
      dispatcher.add(new WorkerExample("Thread 1", 1));
      dispatcher.add(new WorkerExample("Thread 2", 2));
      dispatcher.add(new WorkerExample("Thread 3", 3));
      dispatcher.add(new WorkerExample("Thread 4", 4));
    }
    catch(ThreadError e)
    {
      stdout.printf("ThreadError: %s\n", e.message);
      return -1;
    }

    int main_x_times = 12;
    for(int i = 0; i < main_x_times ; i++)
    {
      stdout.printf("[%"+ int64.FORMAT + "] Thread M: %d/%d\n",
        (new DateTime.now_utc()).to_unix(), i + 1, main_x_times);
      Thread.usleep(100000); // wait 0.1 second
    }

#endif
    return 0;
  }
}

