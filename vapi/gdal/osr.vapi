[CCode (cheader_filename = "ogr_srs_api.h")]
namespace Osr {

//[CCode (cname = "struct OGRSpatialReferenceHS", ref_function = "OSRReference", unref_function = "OSRDereference")]
[CCode (cname = "void", ref_function = "OSRReference", unref_function = "OSRDereference")]
[Compact]
public class SpatialReference {

    [CCode (cname="OSRNewSpatialReference")]
    public SpatialReference(string wkt = "");

    [CCode (cname="OSRClone")]
    public SpatialReference clone();

    [CCode (cname="OSRReference")]
    public void @ref();
    [CCode (cname="OSRDereference")]
    public void unref();

    [CCode (cname="OSRValidate")]
    public int validate();

    [CCode (cname="OSRImportFromEPSG")]
    public void import_from_epsg(int epsg);

    [CCode (cname="OSRExportToWkt")]
    public int export_to_wkt(out string wkt);

    [CCode (cname="OSRExportToProj4")]
    public int export_to_proj4(out string proj4);

    public string to_string() {
        string str;
        export_to_wkt(out str);
        return str;
    }
}

[CCode (cname = "struct OGRCoordinateTransformationHS", free_function = "OSRDestroyCoordinateTransformation")]
public class CoordinateTransformation {

    [CCode (cname = "OCTNewCoordinateTransformation")]
    public CoordinateTransformation(SpatialReference source, SpatialReference target);

    [CCode (cname = "OCTTransform")]
    int transform(int count, double[] x, double[] y, double[]z); 

}

}
