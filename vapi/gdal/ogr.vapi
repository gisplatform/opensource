[CCode (cheader_filename = "ogr_api.h,gdal.h,ogr_srs_api.h")]
namespace Ogr {

[CCode (cname = "struct OGRDataSourceHS", ref_function = "OGR_DS_Reference", unref_function = "OGR_DS_Dereference")]
//[CCode (cname = "OGRDataSourceHS", free_function="OGRReleaseDataSource")]
[Compact]
public class DataSource {

    [CCode (cname = "OGR_DS_Reference")]
    public void @ref();
    [CCode (cname = "OGR_DS_Dereference")]
    public void unref();

    [CCode (cname = "OGR_DS_GetName")]
    public unowned string get_name();

    [CCode (cname = "OGR_DS_CreateLayer")]
    public Layer? create_layer(string name, Osr.SpatialReference spatial_ref, GeometryType type, [CCode (array_null_terminated = true, array_length = false)] string[] options);

    [CCode (cname = "OGR_DS_GetLayer")]
    public Layer? get_layer(int idx);
}

public class DriverRegistrar {

    [CCode (cname="OGRGetDriverByName")]
    public static unowned Driver? get_driver_by_name(string name);
    [CCode (cname="OGRGetDriver")]
    public static unowned Driver? get_driver(int idx);
    
    [CCode (cname="OGRGetDriverCount")]
    public static int get_driver_count();

    [CCode (cname="OGRRegisterAll")]
    public static void register_all();

    [CCode (cname="OGROpen")]
    public static DataSource? open(string filename, bool update = false, out Driver? driver = null);
}

[CCode (cname = "struct OGRSFDriverHS")] //, free_function="OGRSFDestroyDriver")]
[Compact]
public class Driver : Gdal.MajorObject {

    [CCode (cname = "OGR_Dr_CreateDataSource")]
    public DataSource? create_datasource(string name,
        [CCode (array_null_terminated = true, array_length = false)] string[] options);

}

[CCode (cname = "OGREnvelope")]
[Compact]
public struct Envelope {
    [CCode (cname = "MinX")]
    double minx;
    [CCode (cname = "MaxX")]
    double maxx;
    [CCode (cname = "MinY")]
    double miny;
    [CCode (cname = "MaxY")]
    double maxy;
}

[CCode (cname = "struct OGRFeatureDefnHS", ref_function = "OGR_FD_Reference", unref_function = "OGR_FD_Dereference")]
[Compact]
public class FeatureDefn {

    [CCode (cname = "OGR_FD_Create")]
    public FeatureDefn(string name);

    [CCode (cname = "OGR_FD_Reference")]
    public void @ref();
    [CCode (cname = "OGR_FD_Dereference")]
    public void unref();

    [CCode (cname = "OGR_FD_GetName")]
    public unowned string get_name();
    [CCode (cname = "OGR_FD_GetFieldCount")]
    public int get_field_count();

}

[CCode (cname = "struct OGRFeatureHS", free_function = "OGR_F_Destroy")]
[Compact]
public class Feature {

    [CCode (cname = "OGR_F_Create")]
    public Feature(FeatureDefn defn);

    [CCode (cname = "OGR_F_GetGeometryRef")]
    public unowned Geometry? get_geometry();
    [CCode (cname = "OGR_F_SetGeometry")]
    public void set_geometry(Ogr.Geometry geom);

    [CCode (cname = "OGR_F_SetFieldString")]
    public void set_field_string(int key, string val);
    [CCode (cname = "OGR_F_GetFieldAsString")]
    public unowned string get_field_string(int key);

}

public enum FieldType {
    [CCode (cname = "OFTInteger")]
    INTEGER = 0,
    [CCode (cname = "OFTIntegerList")]
    INTEGER_LIST = 1,
    [CCode (cname = "OFTReal")]
    REAL = 2,
    [CCode (cname = "OFTRealList")]
    REAL_LIST = 3,
    [CCode (cname = "OFTString")]
    STRING = 4,
    STRING_LIST = 5,
    WIDE_STRING = 6,
    WIDE_STRING_LIST = 7,
    BINARY = 8,
    DATE = 9,
    TIME = 10,
    DATETIME = 11,
    INTEGER64 = 12,
    INTEGER64_LIST = 13
}

[CCode (cname = "struct OGRFieldDefnHS", free_function = "OGR_Fld_Destroy")]
[Compact]
public class FieldDefn {

    [CCode (cname = "OGR_Fld_Create")]
    public FieldDefn(string name, FieldType type);

    [CCode (cname = "OGR_Fld_SetWidth")]
    public void set_width(int width);

}

[CCode (cname = "OGRwkbGeometryType", has_type_id = false)]
public enum GeometryType {
    [CCode (cname = "wkbUnknown")]
    UNKNOWN,
    [CCode (cname = "wkbPoint")]
    POINT,
    [CCode (cname = "wkbLineString")]
    LINE_STRING,
    [CCode (cname = "wkbPolygon")]
    POLYGON,
    [CCode (cname = "wkbMultiPoint")]
    MULTI_POINT,
    [CCode (cname = "wkbMultiLineString")]
    MULTI_LINE_STRING,
    [CCode (cname = "wkbMultiPolygon")]
    MULTI_POLYGON,
    [CCode (cname = "wkbGeometryCollection")]
    GEOMETRY_COLLECTION,
/* SS-FIX FOR OLD VERSION
    [CCode (cname = "wkbCircularString")]
    CIRCULAR_STRING,
    [CCode (cname = "wkbCompoundCurve")]
    COMPOUND_CURVE,
    [CCode (cname = "wkbCurvePolygon")]
    CURVE_POLYGON,
    [CCode (cname = "wkbMultiCurve")]
    MULTI_CURVE,
    [CCode (cname = "wkbMultiSurface")]
    MULTI_SURFACE,
*/

    [CCode (cname = "wkbNone")]
    NONE,
    [CCode (cname = "wkbLinearRing")]
    LINEAR_RING,

/* SS-FIX FOR OLD VERSION
    [CCode (cname = "wkbCircularStringZ")]
    CIRCULAR_STRING_Z,
    [CCode (cname = "wkbCompoundCurveZ")]
    COMPOUND_CURVE_Z,
    [CCode (cname = "wkbCurvePolygonZ")]
    CURVE_POLYGON_Z,
    [CCode (cname = "wkbMultiCurveZ")]
    MULTI_CURVE_Z,
    [CCode (cname = "wkbMultiSurfaceZ")]
    MULTI_SURFACE_Z,
*/

    [CCode (cname = "wkbPoint25D")]
    POINT_25D,
    [CCode (cname = "wkbLineString25D")]
    LINE_STRING_25D,
    [CCode (cname = "wkbPolygon25D")]
    POLYGON_25D,
    [CCode (cname = "wkbMultiPoint25D")]
    MULTI_POINT_25D,
    [CCode (cname = "wkbMultiLineString25D")]
    MULTI_LINE_STRING_25D,
    [CCode (cname = "wkbMultiPolygon25D")]
    MULTI_POLYGON_25D,
    [CCode (cname = "wkbGeometryCollection25D")]
    GEOMETRY_COLLECTION_25D
}

//[CCode (cname = "struct OGRGeometryHS", free_function="OGR_G_DestroyGeometry")]
[CCode (cname = "void", free_function="OGR_G_DestroyGeometry")]
[Compact]
public class Geometry {

    [CCode (cname="OGR_G_CreateGeometry")]
    public Geometry(GeometryType geom_type);

    [CCode (cname="OGR_G_Clone")]
    public Geometry clone();

    [CCode (cname="OGR_G_ExportToWkt")]
    public int export_to_wkt(out string wkt);

    [CCode (cname="OGR_G_GetDimension")]
    public int get_dimension();

    [CCode (cname="OGR_G_AddPoint")]
    public void add_point(double x, double y, double z);
    [CCode (cname="OGR_G_AddPoint_2D")]
    public void add_point_2d(double x, double y, double z);

    [CCode (cname="OGR_G_GetGeometryName")]
    public unowned string get_geometry_name();
    [CCode (cname="OGR_G_GetGeometryType")]
    public GeometryType get_geometry_type();

    [CCode (cname="OGR_G_GetEnvelope")]
    public void get_envelope(out Envelope envelope);

    [CCode (cname="OGR_G_IsEmpty")]
    public bool is_empty();
    [CCode (cname="OGR_G_IsValid")]
    public bool is_valid();
    [CCode (cname="OGR_G_IsSimple")]
    public bool is_simple();
    [CCode (cname="OGR_G_IsRing")]
    public bool is_ring();

    [CCode (cname="OGR_G_Area")]
    public double get_area();
    [CCode (cname="OGR_G_Length")]
    public double get_length();
    [CCode (cname="OGR_G_Distance")]
    public double distance(Geometry other);
    [CCode (cname="OGR_G_Centroid")]
    public int centroid(Geometry other);

    [CCode (cname="OGR_G_ExportToGML")]
    public string export_to_gml();
    [CCode (cname="OGR_G_ExportToKML")]
    public string export_to_kml(string? altitude_mode = null);
    [CCode (cname="OGR_G_ExportToJson")]
    public string export_to_json();

    public string to_string() {
        string str;
        export_to_wkt(out str);
        return str;
    }
}

[CCode (cname = "struct OGRLayerHS", ref_function = "OGR_L_Reference", unref_function = "OGR_L_Dereference")]
[Compact]
public class Layer {

    [CCode (cname = "OGR_L_Reference")]
    public void @ref();
    [CCode (cname = "OGR_L_Dereference")]
    public void unref();

    [CCode (cname = "OGR_L_GetName")]
    public unowned string get_name();

    [CCode (cname = "OGR_L_GetLayerDefn")]
    public unowned FeatureDefn get_layer_defn();

    [CCode (cname = "OGR_L_CreateField")]
    public int create_field(FieldDefn field, bool approx_ok = false);

    [CCode (cname = "OGR_L_GetFeature")]
    public Feature? get_feature(int64 index);
    [CCode (cname = "OGR_L_GetFeatureCount")]
    public int64 get_feature_count(bool force = false);
    [CCode (cname = "OGR_L_SetFeature")]
    public void set_feature(Feature feature);
    [CCode (cname = "OGR_L_CreateFeature")]
    public void create_feature(Feature feature);

    [CCode (cname = "OGR_L_GetGeomType")]
    public GeometryType get_geometry_type();

    [CCode (cname = "OGR_L_GetSpatialFilter")]
    public unowned Geometry? get_spatial_filter();

    [CCode (cname = "OGR_L_GetSpatialRef")]
    public unowned Osr.SpatialReference? get_spatial_reference();

    [CCode (cname = "OGR_L_GetExtent")]
    public int get_extent(out Envelope extent, bool force = false);
}

}
