using Gtk;

public class DrawBackGroundSample : Window
{
  private const string FILENAME_PNG = "checker-test.png";
  private const string FILENAME_HTML = "checker-test.html";

  private const int CHECK_SIZE = 40;

  private int dr_area_width;
  private int dr_area_height;

  private SpinButton width_spin;
  private SpinButton height_spin;
  private CheckButton resize_check;
  private Statusbar statusbar;
  public uint STATUSBAR_CONTEXT_ID = 0;

  private Array<uchar> png_raw = new Array<uchar>();

  private void save_html()
  {
    Cairo.Surface surface = new Cairo.ImageSurface(Cairo.Format.ARGB32, dr_area_width, dr_area_height);
    var cr = new Cairo.Context(surface);

    Gp.draw_checker_background(cr, (uint)dr_area_width, (uint)dr_area_height, CHECK_SIZE);

    if(resize_check.active)
      surface = Gp.cairo_surface_resize((int)width_spin.value, (int)height_spin.value,
        surface, dr_area_width, dr_area_height);

    try
    {
      FileUtils.remove(FILENAME_HTML);
      File file = File.new_for_path(FILENAME_HTML);
      var dos = new DataOutputStream(file.create(FileCreateFlags.REPLACE_DESTINATION));

      var status = surface.write_to_png_stream((png_raw_part) =>
      {
        png_raw.append_vals(png_raw_part, png_raw_part.length);
        message("Writing to PNG stream: += " + format_size(png_raw_part.length));
        return Cairo.Status.SUCCESS;
      });

      message("Writing to PNG stream: DONE with size = " + format_size(png_raw.length));

      string png_base64 = Base64.encode(png_raw.data);
      png_raw.set_size(0);

      if(status != Cairo.Status.SUCCESS)
        throw new IOError.FAILED("Failed to write PNG stream: %s".printf(status.to_string()));

      size_t written;
      dos.write_all("<!DOCTYPE HTML><html><body><table border=\"1\"><tr><th>Data as URL:</th></tr><tr><td><a\n".data, out written);
      dos.write_all("href=\"data:image/png;base64,".data, out written);
      dos.write_all(png_base64.data, out written);
      dos.write_all("\"\n>Click here to open data URL</a></td></tr><tr><th>Data as image:</th></tr><tr><td><img\n".data, out written);
      dos.write_all("src=\"data:image/png;base64,".data, out written);
      dos.write_all(png_base64.data, out written);
      dos.write_all("\"\n></td></tr></table></body></html>".data, out written);

      statusbar.push(STATUSBAR_CONTEXT_ID, "File " + FILENAME_HTML + " has been written.");
    }
    catch(Error e)
    {
      statusbar.push(STATUSBAR_CONTEXT_ID, "Failed to save HTML: " + e.message);
    }
  }

  private void save_png()
  {
    Cairo.Surface surface = new Cairo.ImageSurface(Cairo.Format.ARGB32, dr_area_width, dr_area_height);
    var cr = new Cairo.Context(surface);

    Gp.draw_checker_background(cr, (uint)dr_area_width, (uint)dr_area_height, CHECK_SIZE);

    if(resize_check.active)
      surface = Gp.cairo_surface_resize((int)width_spin.value, (int)height_spin.value,
        surface, dr_area_width, dr_area_height);

    var status = surface.write_to_png(FILENAME_PNG);

    if(status == Cairo.Status.SUCCESS)
      statusbar.push(STATUSBAR_CONTEXT_ID, "File " + FILENAME_PNG + " has been written.");
    else
      statusbar.push(STATUSBAR_CONTEXT_ID, "Failed to save PNG: %s".printf(status.to_string()));
  }

  public static int main (string[] args)
  {
    Gtk.init(ref args);
    var win = new DrawBackGroundSample();
    win.set_title("DrawCeckerBackGround Sample");
    win.set_default_size(640, 480);

    var vbox = new Box(Orientation.VERTICAL, 3);
    win.add(vbox);

    var dr_area = new DrawingArea();
    vbox.pack_start(dr_area, true, true);

    dr_area.configure_event.connect((e) =>
    {
      var ec = (Gdk.EventConfigure)e;

      if(win.dr_area_width != ec.width ||  win.dr_area_height != ec.height)
      {
        win.dr_area_width = ec.width;
        win.dr_area_height = ec.height;
        win.statusbar.push(win.STATUSBAR_CONTEXT_ID,
          "Area resized to %d x %d".printf(ec.width, ec.height));
      }

      return false;
    });

    dr_area.draw.connect((cr) =>
    {
      Gp.draw_checker_background(cr, (uint)win.dr_area_width, (uint)win.dr_area_height, CHECK_SIZE);
      return false;
    });

    {
      var hbox = new Box(Orientation.HORIZONTAL, 3);
      vbox.pack_start(hbox, false, false);

      win.resize_check = new CheckButton.with_label("Resize file to height =");
      win.resize_check.active = true;
      hbox.pack_start(win.resize_check);

      win.width_spin = new SpinButton.with_range(10, 10000, 1);
      win.width_spin.value = 1024;
      hbox.pack_start(win.width_spin);
    }

    {
      var hbox = new Box(Orientation.HORIZONTAL, 3);
      vbox.pack_start(hbox, false, false);

      hbox.pack_start(new Label("and width ="));

      win.height_spin = new SpinButton.with_range(10, 10000, 1);
      win.height_spin.value = 768;
      hbox.pack_start(win.height_spin);
    }

    var save_png_butt = new Button.with_label("Save to " + FILENAME_PNG);
    vbox.pack_start(save_png_butt, false, false);
    save_png_butt.clicked.connect(win.save_png);

    var save_html_butt = new Button.with_label("Save to " + FILENAME_HTML);
    vbox.pack_start(save_html_butt, false, false);
    save_html_butt.clicked.connect(win.save_html);

    win.statusbar = new Statusbar();
    vbox.pack_start(win.statusbar, false, false);

    win.destroy.connect(Gtk.main_quit);
    win.show_all();

    Gtk.main();
    return 0;
  }
}

