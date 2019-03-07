struct Globals
{
  public static bool only_ipv4 = false;
  public static bool only_ipv6 = false;

  public static const OptionEntry[] options =
  {
    { "ipv4", '4', 0, OptionArg.NONE, ref Globals.only_ipv4, "List only IPv4 addresses", null },
    { "ipv6", '6', 0, OptionArg.NONE, ref Globals.only_ipv6, "List only IPv6 addresses", null },
    { null }
  };
}

public class Main : Object
{

  public static int main(string[] args)
  {
    var family = SocketFamily.INVALID;

    try
    {
      var opt_context = new OptionContext("- NEt INfo");
      opt_context.set_help_enabled(true);
      opt_context.add_main_entries(Globals.options, null);
      opt_context.parse(ref args);
    }
    catch(OptionError e)
    {
      stdout.printf("Failed to parse options: %s.\n", e.message);
      stdout.printf("Run '%s --help' to see a full list of available command line options.\n", args[0]);
      return -1;
    }

    if(Globals.only_ipv4 == true && Globals.only_ipv6 == true)
    {
      stdout.printf("Can't use both 'ipv4' and 'ipv6' options.\n");
      return -2;
    }

    if(Globals.only_ipv4 == true)
      family = SocketFamily.IPV4;

    if(Globals.only_ipv6 == true)
      family =  SocketFamily.IPV6;

    try
    {
      var neins = Gp.Nein.collect(family);

      foreach(var n in neins)
        print("IP: '%s', iface: '%s'\n", n.ip, n.iface);
    }
    catch(Error e)
    {
      stdout.printf("Failed to collect net info: %s\n", e.message);
      return -1;
    }

    return 0;
  }
}
