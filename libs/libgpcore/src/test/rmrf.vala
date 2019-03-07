int main(string[] args)
{
  if (args.length != 2)
  {
    warning("Usage: %s TARGET\n", args[0]);
    return 0;
  }

  try
  {
    Gp.recursive_delete(File.new_for_commandline_arg(args[1]));
  }
  catch(Error e)
  {
    critical("%s\n", e.message);
    return 1;
  }

  return 0;
}

