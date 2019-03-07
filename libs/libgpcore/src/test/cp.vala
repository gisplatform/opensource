int main(string[] args)
{
  if(args.length != 3)
  {
    warning("Usage: %s SRC DEST\n", args[0]);
    return 0;
  }

  try
  {
    Gp.recursive_copy(
      File.new_for_commandline_arg(args[1]),
      File.new_for_commandline_arg(args[2])
    );
  }
  catch(Error e)
  {
    critical("%s\n", e.message);
    return 1;
  }

  return 0;
}

