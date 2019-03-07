[SimpleType]
[CCode (cheader_filename = "magnetic_model_and_geo_point.h")]
public struct GeoPoint
{
  public double longitude;
  public double latitude;

  [CCode (cname = "magnetic_declination")]
  public double magnetic_declination(double height, GLib.Date date);
}
