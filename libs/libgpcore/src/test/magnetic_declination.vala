using Gtk;

int main(string[] args)
{
  Gtk.init(ref args);

  var window = new Window ();
  window.title = "magnetic_declination";

  window.border_width = 10;
  window.window_position = WindowPosition.CENTER;
  window.set_default_size(300, 400);
  window.destroy.connect(Gtk.main_quit);

  var grid = new Grid();
  window.add(grid);


  var label_lat = new Label("Lat°:");
  label_lat.margin = 5;
  grid.attach(label_lat, 0, 0, 1, 1);

  var spin_lat = new SpinButton.with_range(-100, 100, 1);
  spin_lat.margin = 5;
  grid.attach_next_to(spin_lat, label_lat, PositionType.RIGHT, 1, 1);

  var label_lon = new Label("Lon°:");
  label_lon.margin = 5;
  grid.attach_next_to(label_lon, label_lat, PositionType.BOTTOM, 1, 1);

  var spin_lon = new SpinButton.with_range(-100, 100, 1);
  spin_lon.margin = 5;
  grid.attach_next_to(spin_lon, label_lon, PositionType.RIGHT, 1, 1);

  var label_hi = new Label("H:");
  label_hi.margin = 5;
  grid.attach_next_to(label_hi, label_lon, PositionType.BOTTOM, 1, 1);

  var spin_hi = new SpinButton.with_range(-100, 100, 1);
  spin_hi.margin = 5;
  grid.attach_next_to(spin_hi, label_hi, PositionType.RIGHT, 1, 1);

  var cal = new Calendar();
  cal.margin = 5;
  grid.attach(cal, 2, 0, 1, 3);

  var text_out = new TextView();
  text_out.margin = 5;
  text_out.expand = true;
  grid.attach(text_out, 0, 3, 3, 1);

  var button = new Button.with_label("_Apply");
  button.margin = 5;
  text_out.expand = true;
  grid.attach(button, 0, 4, 3, 1);

  var buffer = text_out.buffer;
  TextIter iter;
  buffer.get_iter_at_offset(out iter, 0);

  button.clicked.connect(() =>
  {
    GeoPoint geo_point = { 0 };
    geo_point.longitude = spin_lon.value;
    geo_point.latitude = spin_lat.value;

    uint year, month, day;
    cal.get_date(out year, out month, out day);

    Date date = {};
    date.clear();
    date.set_dmy((GLib.DateDay)day, (int)month, (GLib.DateYear)year);

    Gp.Point point = {};
    point.set_point(Gp.CoordSysType.WGS_84, 0, spin_lat.value, spin_lon.value, spin_hi.value, 1);

    double new_delc = point.get_magnetic_declination(date);
    double old_decl = geo_point.magnetic_declination(spin_hi.value, date);

    buffer.insert(ref iter, @"C: $old_decl \tVala: $new_delc\n", -1);

  });

  window.show_all ();
  Gtk.main ();

  return 0;
}

