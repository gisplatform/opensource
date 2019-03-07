/*
 * geotiff.vapi is a file with libgeotiff bindings for vala
 *
 * Copyright (C) 2015 Gennadiy Nefediev.
 *
 * geotiff.vapi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * geotiff.vapi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with geotiff.vapi. If not, see <http://www.gnu.org/licenses/>.
 *
*/


[CCode (cname = "", lower_case_cprefix = "")]
namespace GeoTiff {

  [CCode (cname = "tagtype_t", cprefix = "TYPE_", has_type_id = false, cheader_filename = "geotiff.h")]
  public enum TagType {
    BYTE, SHORT, LONG, RATIONAL, ASCII, FLOAT, DOUBLE, SBYTE, SSHORT, SLONG, UNKNOWN
	}

  [CCode (cname = "int", cprefix = "STT_", has_type_id = false, cheader_filename = "geo_simpletags.h")]
  public enum StType {
    SHORT, DOUBLE, ASCII
  }

  [CCode (cname = "gtiff_flags", cprefix = "FLAG_", has_type_id = false, cheader_filename = "geo_keyp.h")]
  public enum GTiffFlags {
	  FILE_OPEN, FILE_MODIFIED
  }

  [SimpleType]
  [CCode (cname = "tifftag_t", has_type_id = false, cheader_filename = "geotiff.h") ]
  public struct tifftag_t : ushort { }
  [SimpleType]
  [CCode (cname = "geocode_t", has_type_id = false, cheader_filename = "geotiff.h") ]
  public struct geocode_t : ushort { }
  [SimpleType]
  [CCode (cname = "pinfo_t", has_type_id = false, cheader_filename = "geo_tiffp.h") ]
  public struct pinfo_t : ushort { }
	[Compact]
	[CCode (cname = "void")]
	public class tdata_t { }
	[Compact]
  [CCode (cname = "tiff_t", has_type_id = false, cheader_filename = "geo_tiffp.h") ]
  public class tiff_t : Tiff.TIFF { }
  [Compact]
  [CCode (cname = "void", has_type_id = false, cheader_filename = "geo_simpletags.h") ]
  public class STIFF { }


	[CCode (cname = "GeoKey", cprefix = "gk_", has_type_id = false, cheader_filename = "geo_keyp.h")]
	public struct GeoKey {
	  public int     key;
	  public size_t  size;
	  public TagType type;
	  public long    count;
	  public char *data;  // ?
  }

	[CCode (cname = "KeyEntry", cprefix = "ent_", has_type_id = false, cheader_filename = "geo_keyp.h")]
	public struct KeyEntry {
    public pinfo_t key;
	  public pinfo_t location;
	  public pinfo_t count;
	  public pinfo_t val_offset;
  }

	[CCode (cname = "KeyHeader", cprefix = "hdr_", has_type_id = false, cheader_filename = "geo_keyp.h")]
	public struct KeyHeader {
    public pinfo_t version;
	  public pinfo_t rev_major;
	  public pinfo_t rev_minor;
	  public pinfo_t num_keys;
  }

	[CCode (cname = "TempKeyData", cprefix = "", has_type_id = false, cheader_filename = "geo_keyp.h")]
	public struct TempKeyData {
	  [CCode (cname = "tk_asciiParams")]
    public char* ascii_params;
	  [CCode (cname = "tk_asciiParamsLength")]
    public int ascii_params_length;
	  [CCode (cname = "tk_asciiParamsOffset")]
    public int ascii_params_offset;
  }

	[CCode (cname = "ST_KEY", cprefix = "", has_type_id = false, cheader_filename = "geo_simpletags.h")]
  public struct StKey {
    public int    tag;
    public int    count;
    public StType type;
    public void*  data;  //tdata_t data; ?
  }

	[CCode (cname = "ST_TIFF", unref_function = "ST_Destroy", cheader_filename = "geo_simpletags.h")]
  public class StTiff {
    int     key_count;
    StKey[] key_list;
    [CCode (cname = "ST_Create")]
    public StTiff ();
    [CCode (cname = "ST_GetKey")]
    public bool get_key (int tag, ref int count, ref StType st_type, ref tdata_t data);
    [CCode (cname = "ST_SetKey")]
    public bool set_key (int tag, int count, StType st_type, tdata_t data);
  }

	[CCode (cname = "GTIFDefn", cprefix = "", has_type_id = false, cheader_filename = "geo_normalize.h")]
  public struct Defn {
	  [CCode (cname = "Model")]             short  model;
	  [CCode (cname = "PCS")]               short  pcs;
	  [CCode (cname = "GCS")]               short  gcs;	      

	  [CCode (cname = "UOMLength")]         short  uom_length;
	  [CCode (cname = "UOMLengthInMeters")] double uom_length_in_meters;
	  [CCode (cname = "UOMAngle")]          short  uom_angle;
	  [CCode (cname = "UOMAngleInDegrees")] double uom_angle_in_degrees;

	  [CCode (cname = "Datum")]             short  datum;
	  [CCode (cname = "PM")]                short  pm;
	  [CCode (cname = "PMLongToGreenwich")] double pm_long_to_greenwich;
	  [CCode (cname = "Ellipsoid")]         short  ellipsoid;
	  [CCode (cname = "SemiMajor")]         double semi_major;
	  [CCode (cname = "SemiMinor")]         double semi_minor;

	  [CCode (cname = "TOWGS84Count")]      short  to_wgs84_count;
	  [CCode (cname = "TOWGS84")]           double[] to_wgs84;  //double TOWGS84[7];

	  [CCode (cname = "ProjCode")]          short  proj_code;
	  [CCode (cname = "Projection")]        short  projection;
	  [CCode (cname = "CTProjection")]      short  ct_projection;   

	  [CCode (cname = "nParms")]            int    n_parms;
		[CCode (cname = "ProjParm", array_length = false)]   double[] proj_parm;  //double ProjParm[MAX_GTIF_PROJPARMS];
		[CCode (cname = "ProjParmId", array_length = false)] int[] proj_parm_id;  //int ProjParmId[MAX_GTIF_PROJPARMS];

	  [CCode (cname = "MapSys")]            int    mapsys;
	  [CCode (cname = "Zone")]              int    zone;
	  [CCode (cname = "DefnSet")]           int    defn_set;

    [CCode (cname = "GTIFPrintDefn")]
    public void print (GLib.FileStream p1);
    [CCode (cname = "GTIFGetProj4Defn")]
    public char* get_proj4 ();
    [CCode (cname = "GTIFProj4ToLatLong")]
    public int proj4_to_latlong (int p1, ref double p2, ref double p3);  //(double*, double*)
    [CCode (cname = "GTIFProj4FromLatLong")]
    public int proj4_from_latlong (int p1, ref double p2, ref double p3);  //(double*, double*)
  }


	[CCode (cname = "TIFFMethod", has_type_id = false, cheader_filename = "geo_tiffp.h")]
  public struct TiffMethod {
    [CCode (cname = "get")]
    public GetFunction fget;
    [CCode (cname = "set")]
    public SetFunction fset;
    public TypeFunction type;
  }

	[CCode (cname = "GTGetFunction", has_target = false)]
  public delegate int GetFunction (tiff_t *tif, pinfo_t tag, ref int count, tdata_t val);
	[CCode (cname = "GTSetFunction", has_target = false)]
  public delegate int SetFunction (tiff_t *tif, pinfo_t tag, int count, tdata_t val);
	[CCode (cname = "GTTypeFunction", has_target = false)]
  public delegate TagType TypeFunction (tiff_t *tif, pinfo_t tag);

  [CCode (cname = "GTIFPrintMethod", has_target = false)]
  public delegate int PrintMethod (string p1, tdata_t p2); //(char *string, void *aux);
  [CCode (cname = "GTIFReadMethod", has_target = false)]
  public delegate int ReadMethod (string p1, tdata_t p2); //(char *string, void *aux);


	[CCode (cname = "struct gtiff", cprefix = "gt_", has_type_id = false, unref_function = "GTIFFree") ]
	public class GTIF {
    public tiff_t    tif;
    public TiffMethod methods;
    public int       flags;

    public pinfo_t   version;
    public pinfo_t   rev_major;
    public pinfo_t   rev_minor;

    public int       num_keys;
    public GeoKey[]  keys;
    public int[]     keyindex;
    public int       keymin;
    public int       keymax;

    [CCode (cname = "gt_short")]
    public pinfo_t[] gtshort;
    [CCode (cname = "gt_double")]
    public double[]  gtdouble;
    public int       nshorts;
    public int       ndoubles;

    [CCode (cname = "GTIFNew")]
    public GTIF (Tiff.TIFF tif);
    [CCode (cname = "GTIFNewSimpleTags")]
    public GTIF.simple_tags (Tiff.TIFF tif);
    [CCode (cname = "GTIFNewWithMethods")]
    public GTIF.with_methods (Tiff.TIFF tif, TiffMethod p2);

    [CCode (cname = "GTIFWriteKeys")]
    public bool write_keys();
    [CCode (cname = "GTIFDirectoryInfo")]
    public void directory_info ([CCode (array_length = false)] int[] versions, ref int keycount);
    [CCode (cname = "GTIFKeyInfo")]
    public bool key_info (GeoKeys key, ref int size, ref TagType type);
    [CCode (cname = "GTIFKeyGet")]
    public bool key_get (GeoKeys key, tdata_t val, int index, int count);
    [CCode (cname = "GTIFKeySet")]
    public bool key_set (GeoKeys key_id, TagType type, int count, ...);

    [CCode (cname = "GTIFPrint")]
    public void print (PrintMethod p1, tdata_t p2); //void GTIFPrint(GTIF *gtif, GTIFPrintMethod print, void *aux);
    [CCode (cname = "GTIFImport")]
    public int import (ReadMethod scan, tdata_t p2); //int GTIFImport(GTIF *gtif, GTIFReadMethod scan, void *aux);

    [CCode (cname = "GTIFImageToPCS")]
    public int image_to_pcs (ref double x, ref double y);  //(double*, double*)
    [CCode (cname = "GTIFPCSToImage")]
    public int pcs_to_image (ref double x, ref double y);  //(double*, double*)
    [CCode (cname = "GTIFSetFromProj4")]
    public int set_from_proj4 (string proj4);
    [CCode (cname = "GTIFGetDefn")]
    public int get_defn (Defn p1);
    [CCode (cname = "GTIFFreeDefn")]
    public void free_defn ();
  }


    [CCode (cname = "TIFFMergeFieldInfo")]
    public static int MergeFieldInfo_fix (Tiff.TIFF tif, Tiff.FieldInfo[] p1);
    //public bool MergeFieldInfo_fix (Tiff.TIFF tif, [CCode (array_length = false)] Tiff.FieldInfo[] p1, uint32 p2);

    [CCode (cname = "GTIFKeyName", cheader_filename = "geotiff.h")]
    public static char* key_name (GeoKeys key);
    [CCode (cname = "GTIFValueName")]
    public static char* value_name (GeoKeys key, int val);
    [CCode (cname = "GTIFTypeName")]
    public static char* type_name (TagType type);
    [CCode (cname = "GTIFTagName")]
    public static char* tag_name (int tag);
    [CCode (cname = "GTIFKeyCode")]
    public static GeoKeys key_code (string key);
    [CCode (cname = "GTIFValueCode")]
    public static int value_code (GeoKeys key, string val);
    [CCode (cname = "GTIFTypeCode")]
    public static TagType type_code (string type);
    [CCode (cname = "GTIFTagCode")]
    public static int tag_code (string tag);

    [CCode (cname = "GTIFGetPCSInfo", cheader_filename = "geo_normalize.h")]
    public static bool get_pcs_info (int pcs_code, ref string epsg_name, ref short proj_op,
                                     ref short uom_length_code, ref short geog_cs);
    [CCode (cname = "GTIFGetProjTRFInfo")]
    public static bool get_proj_trf_info (int proj_trf_code, ref string proj_trf_name,
                                          ref short proj_method, ref double proj_parms);
    [CCode (cname = "GTIFGetGCSInfo")]
    public static bool get_gcs_info (int gcs_code, ref string name, ref short datum, ref short pm,
                                     ref short uom_angle);
    [CCode (cname = "GTIFGetDatumInfo")]
    public static bool get_datum_info (int datum_code, ref string name, ref short ellipsoid);
    [CCode (cname = "GTIFGetEllipsoidInfo")]
    public static bool get_ellipsoid_inf (int ellipsoid, ref string name, ref double semi_major,
                                          ref double semi_minor);
    [CCode (cname = "GTIFGetPMInfo")]
    public static bool get_pm_info (int pm, ref string name, ref double long_to_greenwich);
    [CCode (cname = "GTIFGetUOMLengthInfo")]
    public static bool get_uom_length_info (int uom_length_code, ref string uom_name,
                                            ref double in_meters);
    [CCode (cname = "GTIFGetUOMAngleInfo")]
    public static bool get_uom_angle_info (int uom_angle_code, ref string uom_name,
                                           ref double in_degrees);
    [CCode (cname = "GTIFAngleStringToDD")]
    public static double angle_string_to_dd (string angle, int uom_angle);
    [CCode (cname = "GTIFAngleToDD")]
    public static double angle_to_dd (double angle, int uom_angle);

    [CCode (cname = "GTIFFreeMemory")]
    public static void gt_free_memory (string p1);
    [CCode (cname = "GTIFDeaccessCSV")]
    public static void deaccess_csv ();
    [CCode (cname = "GTIFDecToDMS")]
    public static string dec_to_dms (double p1, string p2, int p3);
    [CCode (cname = "SetCSVFilenameHook")]
    public static void set_csv_filename_hook (string p1); //(const char *(*CSVFileOverride)(const char *)) ?

    [CCode (cname = "GTIFMapSysToPCS")]
    public static int mapsys_to_pcs (int mapsys, int datum, int zone);
    [CCode (cname = "GTIFMapSysToProj")]
    public static int mapsys_to_proj (int mapsys, int zone);
    [CCode (cname = "GTIFPCSToMapSys")]
    public static int pcs_to_mapsys (int pcs_code, ref int datum, ref int zone);
    [CCode (cname = "GTIFProjToMapSys")]
    public static int proj_to_mapsys (int proj_code, ref int zone);

    [CCode (cname = "GTIFSetSimpleTagsMethods", cheader_filename = "geo_simpletags.h")]
    public static void set_simple_tags_methods (TiffMethod p1);
    [CCode (cname = "ST_TagType")]
    public static StType st_tag_type (int tag);


  [CCode (cheader_filename = "xtiffio.h")]
	public const tifftag_t TIFFTAG_GEOPIXELSCALE;
	public const tifftag_t TIFFTAG_INTERGRAPH_MATRIX;
	public const tifftag_t TIFFTAG_GEOTIEPOINTS;
	public const tifftag_t TIFFTAG_JPL_CARTO_IFD;
	public const tifftag_t TIFFTAG_GEOTRANSMATRIX;
	public const tifftag_t TIFFTAG_GEOKEYDIRECTORY;
	public const tifftag_t TIFFTAG_GEODOUBLEPARAMS;
	public const tifftag_t TIFFTAG_GEOASCIIPARAMS;

	public const tifftag_t TIFFPRINT_GEOKEYDIRECTORY;
	public const tifftag_t TIFFPRINT_GEOKEYPARAMS;

  [CCode (cheader_filename = "geo_tiffp.h")]
	public const tifftag_t GTIFF_GEOKEYDIRECTORY;
	public const tifftag_t GTIFF_DOUBLEPARAMS;
	public const tifftag_t GTIFF_ASCIIPARAMS;
	public const tifftag_t GTIFF_PIXELSCALE;
	public const tifftag_t GTIFF_TRANSMATRIX;
	public const tifftag_t GTIFF_INTERGRAPH_MATRIX;
	public const tifftag_t GTIFF_TIEPOINTS;
	public const tifftag_t GTIFF_LOCAL;

  [CCode (cname = "MapSys_UTM_North", cheader_filename = "geo_normalize.h")]
	public const tifftag_t MAPSYS_UTM_NORTH;
  [CCode (cname = "MapSys_UTM_South", cheader_filename = "geo_normalize.h")]
  public const tifftag_t MAPSYS_UTM_SOUTH;
  [CCode (cname = "MapSys_State_Plane_27", cheader_filename = "geo_normalize.h")]
	public const tifftag_t MAPSYS_STATE_PLANE_27;
  [CCode (cname = "MapSys_State_Plane_83", cheader_filename = "geo_normalize.h")]
	public const tifftag_t MAPSYS_STATE_PLANE_83;
  [CCode (cheader_filename = "geo_normalize.h")]
	public const tifftag_t MAX_GTIF_PROJPARMS;

  [CCode (cname = "GvCurrentRevision", cheader_filename = "geokeys.h")]
	public const tifftag_t GV_CURRENT_REVISION;
  [CCode (cname = "GvCurrentVersion", cheader_filename = "geotiff.h")]
	public const tifftag_t GV_CURRENT_VERSION;
	public const tifftag_t LIBGEOTIFF_VERSION;
  [CCode (cheader_filename = "geo_keyp.h")]
  public const tifftag_t MAX_KEYINDEX;
  public const tifftag_t MAX_KEYS;
  public const tifftag_t MAX_VALUES;


	[CCode (cname = "geokey_t", cprefix = "", has_type_id = false, cheader_filename = "geokeys.h")]
	public enum GeoKeys {
    BaseGeoKey,
    GTModelTypeGeoKey, GTRasterTypeGeoKey, GTCitationGeoKey,
    GeographicTypeGeoKey, GeogCitationGeoKey, GeogGeodeticDatumGeoKey,
    GeogPrimeMeridianGeoKey, GeogLinearUnitsGeoKey, GeogLinearUnitSizeGeoKey,
    GeogAngularUnitsGeoKey, GeogAngularUnitSizeGeoKey, GeogEllipsoidGeoKey,
    GeogSemiMajorAxisGeoKey, GeogSemiMinorAxisGeoKey, GeogInvFlatteningGeoKey,
    GeogAzimuthUnitsGeoKey, GeogPrimeMeridianLongGeoKey, GeogTOWGS84GeoKey,
    ProjectedCSTypeGeoKey, PCSCitationGeoKey, ProjectionGeoKey,
    ProjCoordTransGeoKey, ProjLinearUnitsGeoKey, ProjLinearUnitSizeGeoKey,
    ProjStdParallel1GeoKey, ProjStdParallelGeoKey, ProjStdParallel2GeoKey,
    ProjNatOriginLongGeoKey, ProjOriginLongGeoKey, ProjNatOriginLatGeoKey,
    ProjOriginLatGeoKey, ProjFalseEastingGeoKey, ProjFalseNorthingGeoKey,
    ProjFalseOriginLongGeoKey, ProjFalseOriginLatGeoKey, ProjFalseOriginEastingGeoKey,
    ProjFalseOriginNorthingGeoKey, ProjCenterLongGeoKey, ProjCenterLatGeoKey,
    ProjCenterEastingGeoKey, ProjCenterNorthingGeoKey, ProjScaleAtNatOriginGeoKey,
    ProjScaleAtOriginGeoKey, ProjScaleAtCenterGeoKey, ProjAzimuthAngleGeoKey,
    ProjStraightVertPoleLongGeoKey, ProjRectifiedGridAngleGeoKey,
    VerticalCSTypeGeoKey, VerticalCitationGeoKey, VerticalDatumGeoKey, VerticalUnitsGeoKey,
    ReservedEndGeoKey,
    PrivateBaseGeoKey,
    PrivateEndGeoKey,
    EndGeoKey
  }

  [CCode (cname = "modeltype_t", cprefix = "", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum ModelType {
    [CCode (cname = "ModelTypeProjected")] MTProjected,
    [CCode (cname = "ModelTypeGeographic")] MTGeographic,
    [CCode (cname = "ModelTypeGeocentric")] MTGeocentric,
	  ModelProjected,
    ModelGeographic,
    ModelGeocentric
  }

  [CCode (cname = "rastertype_t", cprefix = "Raster", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum RasterType {
    PixelIsArea,
    PixelIsPoint
  }

  [CCode (cname = "vdatum_t", cprefix = "", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum VDatum {
	  VDatumBase
  }

	[CCode (cname = "geographic_t", cprefix = "GCS_", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum GeographicCS {
    /* epsg_gcs.inc */
    /* Unspecified GCS based on ellipsoid */
    [CCode (cname = "GCSE_Airy1830")] Airy1830,
    [CCode (cname = "GCSE_AiryModified1849")] AiryModified1849,
    [CCode (cname = "GCSE_AustralianNationalSpheroid")] AustralianNationalSpheroid,
    [CCode (cname = "GCSE_Bessel1841")] Bessel1841,
    [CCode (cname = "GCSE_BesselModified")] BesselModified,
    [CCode (cname = "GCSE_BesselNamibia")] BesselNamibia,
    [CCode (cname = "GCSE_Clarke1858")] Clarke1858,
    [CCode (cname = "GCSE_Clarke1866")] Clarke1866,
    [CCode (cname = "GCSE_Clarke1866Michigan")] Clarke1866Michigan,
    [CCode (cname = "GCSE_Clarke1880_Benoit")] Clarke1880_Benoit,
    [CCode (cname = "GCSE_Clarke1880_IGN")] Clarke1880_IGN,
    [CCode (cname = "GCSE_Clarke1880_RGS")] Clarke1880_RGS,
    [CCode (cname = "GCSE_Clarke1880_Arc")] Clarke1880_Arc,
    [CCode (cname = "GCSE_Clarke1880_SGA1922")] Clarke1880_SGA1922,
    [CCode (cname = "GCSE_Everest1830_1937Adjustment")] Everest1830_1937Adjustment,
    [CCode (cname = "GCSE_Everest1830_1967Definition")] Everest1830_1967Definition,
    [CCode (cname = "GCSE_Everest1830_1975Definition")] Everest1830_1975Definition,
    [CCode (cname = "GCSE_Everest1830Modified")] Everest1830Modified,
    [CCode (cname = "GCSE_GRS1980")] GRS1980,
    [CCode (cname = "GCSE_Helmert1906")] Helmert1906,
    [CCode (cname = "GCSE_IndonesianNationalSpheroid")] IndonesianNationalSpheroid,
    [CCode (cname = "GCSE_International1924")] International1924,
    [CCode (cname = "GCSE_International1967")] International1967,
    [CCode (cname = "GCSE_Krassowsky1940")] Krassowsky1940,
    [CCode (cname = "GCSE_NWL9D")] NWL9D,
    [CCode (cname = "GCSE_NWL10D")] NWL10D,
    [CCode (cname = "GCSE_Plessis1817")] Plessis1817,
    [CCode (cname = "GCSE_Struve1860")] Struve1860,
    [CCode (cname = "GCSE_WarOffice")] WarOffice,
    [CCode (cname = "GCSE_WGS84")] E_WGS84,
    [CCode (cname = "GCSE_GEM10C")] GEM10C,
    [CCode (cname = "GCSE_OSU86F")] OSU86F,
    [CCode (cname = "GCSE_OSU91A")] OSU91A,
    [CCode (cname = "GCSE_Clarke1880")] Clarke1880,
    [CCode (cname = "GCSE_Sphere")] Sphere,
    /* New GCS */
    Greek, GGRS87, KKJ, RT90, EST92, Dealul_Piscului_1970, Greek_Athens,
    /* Standard GCS */
    Adindan, AGD66, AGD84, Ain_el_Abd, Afgooye, Agadez,
    Lisbon,
    Aratu, Arc_1950, Arc_1960,
    Batavia, Barbados, Beduaram, Beijing_1954, Belge_1950,
    Bermuda_1957, Bern_1898, Bogota, Bukit_Rimpah,
    Camacupa, Campo_Inchauspe, Cape, Carthage, Chua, Corrego_Alegre, Cote_d_Ivoire,
    Deir_ez_Zor, Douala,
    Egypt_1907, ED50, ED87,
    Fahud,
    Gandajika_1970, Garoua, Guyane_Francaise,
    Hu_Tzu_Shan, HD72,
    ID74, Indian_1954, Indian_1975,
    Jamaica_1875, JAD69,
    Kalianpur, Kandawala, Kertau, KOC,
    La_Canoa,
    PSAD56,
    Lake, Leigon, Liberia_1964, Lome, Luzon_1911,
    Hito_XVIII_1963, Herat_North,
    Mahe_1971, Makassar,
    EUREF89,
    Malongo_1987, Manoca, Merchich, Massawa, Minna, Mhast, Monte_Mario, M_poraloko,
    NAD27, NAD_Michigan, NAD83, Nahrwan_1967, Naparima_1972,
    GD49,
    NGO_1948,
    Datum_73,
    NTF, NSWC_9Z_2,
    OSGB_1936, OSGB70, OS_SN80,
    Padang, Palestine_1923, Pointe_Noire,
    GDA94,
    Pulkovo_1942,
    Qatar, Qatar_1948, Qornoq,
    Loma_Quintana,
    Amersfoort,
    RT38,
    SAD69, Sapper_Hill_1943, Schwarzeck, Segora, Serindung, Sudan,
    Tananarive, Timbalai_1948, TM65, TM75, Tokyo, Trinidad_1903, TC_1948,
    Voirol_1875, Voirol_Unifie,
    Bern_1938,
    Nord_Sahara_1959,
    Stockholm_1938,
    Yacare, Yoff,
    Zanderij,
    MGI,
    Belge_1972,
    DHDN,
    Conakry_1905,
    WGS_72, WGS_72BE, WGS_84,
    Bern_1898_Bern, Bogota_Bogota,
    Lisbon_Lisbon,
    Makassar_Jakarta, MGI_Ferro, Monte_Mario_Rome,
    NTF_Paris,
    Padang_Jakarta,
    Belge_1950_Brussels,
    Tananarive_Paris,
    Voirol_1875_Paris, Voirol_Unifie_Paris,
    Batavia_Jakarta,
    ATF_Paris, NDG_Paris,
    [CCode (cname = "geographic_end")] geographic_end
  }

	[CCode (cname = "geodeticdatum_t", cprefix = "Datum_", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum GeodeticDatum {
    /* epsg_datum.inc */
    /* New datums */
    Dealul_Piscului_1970,
    /* Datums for which only the ellipsoid is known */
    [CCode (cname = "DatumE_Airy1830")] Airy1830,
    [CCode (cname = "DatumE_AiryModified1849")] AiryModified1849,
    [CCode (cname = "DatumE_AustralianNationalSpheroid")] AustralianNationalSpheroid,
    [CCode (cname = "DatumE_Bessel1841")] Bessel1841,
    [CCode (cname = "DatumE_BesselModified")] BesselModified,
    [CCode (cname = "DatumE_BesselNamibia")] BesselNamibia,
    [CCode (cname = "DatumE_Clarke1858")] Clarke1858,
    [CCode (cname = "DatumE_Clarke1866")] Clarke1866,
    [CCode (cname = "DatumE_Clarke1866Michigan")] Clarke1866Michigan,
    [CCode (cname = "DatumE_Clarke1880_Benoit")] Clarke1880_Benoit,
    [CCode (cname = "DatumE_Clarke1880_IGN")] Clarke1880_IGN,
    [CCode (cname = "DatumE_Clarke1880_RGS")] Clarke1880_RGS,
    [CCode (cname = "DatumE_Clarke1880_Arc")] Clarke1880_Arc,
    [CCode (cname = "DatumE_Clarke1880_SGA1922")] Clarke1880_SGA1922,
    [CCode (cname = "DatumE_Everest1830_1937Adjustment")] Everest1830_1937Adjustment,
    [CCode (cname = "DatumE_Everest1830_1967Definition")] Everest1830_1967Definition,
    [CCode (cname = "DatumE_Everest1830_1975Definition")] Everest1830_1975Definition,
    [CCode (cname = "DatumE_Everest1830Modified")] Everest1830Modified,
    [CCode (cname = "DatumE_GRS1980")] GRS1980,
    [CCode (cname = "DatumE_Helmert1906")] Helmert1906,
    [CCode (cname = "DatumE_IndonesianNationalSpheroid")] IndonesianNationalSpheroid,
    [CCode (cname = "DatumE_International1924")] International1924,
    [CCode (cname = "DatumE_International1967")] International1967,
    [CCode (cname = "DatumE_Krassowsky1960")] Krassowsky1960,
    [CCode (cname = "DatumE_NWL9D")] NWL9D,
    [CCode (cname = "DatumE_NWL10D")] NWL10D,
    [CCode (cname = "DatumE_Plessis1817")] Plessis1817,
    [CCode (cname = "DatumE_Struve1860")] Struve1860,
    [CCode (cname = "DatumE_WarOffice")] WarOffice,
    [CCode (cname = "DatumE_WGS84")] E_WGS84,
    [CCode (cname = "DatumE_GEM10C")] GEM10C,
    [CCode (cname = "DatumE_OSU86F")] OSU86F,
    [CCode (cname = "DatumE_OSU91A")] OSU91A,
    [CCode (cname = "DatumE_Clarke1880")] Clarke1880,
    [CCode (cname = "DatumE_Sphere")] Sphere,
    /* Standard datums */
    Adindan,
    Australian_Geodetic_1966, Australian_Geodetic_1984,
    Ain_el_Abd_1970, Afgooye, Agadez,
    Lisbon,
    Aratu,
    Arc_1950, Arc_1960,
    Batavia, Barbados, Beduaram, Beijing_1954,
    Reseau_National_Belge_1950,
    Bermuda_1957, Bern_1898, Bogota, Bukit_Rimpah,
    Camacupa, Campo_Inchauspe, Cape, Carthage, Chua, Corrego_Alegre, Cote_d_Ivoire,
    Deir_ez_Zor, Douala,
    Egypt_1907,
    European_1950, European_1987,
    Fahud,
    Gandajika_1970, Garoua, Guyane_Francaise,
    Hu_Tzu_Shan, Hungarian_1972,
    Indonesian_1974,
    Indian_1954, Indian_1975,
    Jamaica_1875, Jamaica_1969,
    Kalianpur, Kandawala, Kertau, Kuwait_Oil_Company,
    La_Canoa,
    Provisional_S_American_1956,
    Lake, Leigon, Liberia_1964, Lome, Luzon_1911,
    Hito_XVIII_1963, Herat_North,
    Mahe_1971, Makassar,
    European_Reference_System_1989,
    Malongo_1987, Manoca, Merchich, Massawa, Minna, Mhast, Monte_Mario, M_poraloko,
    North_American_1927,
    NAD_Michigan,
    North_American_1983,
    Nahrwan_1967, Naparima_1972, New_Zealand_Geodetic_1949, NGO_1948,
    Datum_73,
    Nouvelle_Triangulation_Francaise, NSWC_9Z_2,
    OSGB_1936, OSGB_1970_SN, OS_SN_1980,
    Padang_1884, Palestine_1923, Pointe_Noire,
    Geocentric_of_Australia_1994,
    Pulkovo_1942,
    Qatar, Qatar_1948, Qornoq,
    Loma_Quintana,
    Amersfoort,
    RT38,
    South_American_1969, Sapper_Hill_1943, Schwarzeck, Segora, Serindung, Sudan,
    Tananarive_1925, Timbalai_1948,
    TM65, TM75,
    Tokyo, Trinidad_1903, Trucial_Coast_1948,
    Voirol_1875, Voirol_Unifie_1960,
    Bern_1938,
    Nord_Sahara_1959,
    Stockholm_1938,
    Yacare, Yoff,
    Zanderij,
    Militar_Geographische_Institut,
    Reseau_National_Belge_1972,
    Deutsche_Hauptdreiecksnetz,
    Conakry_1905,
    WGS72, WGS72_Transit_Broadcast_Ephemeris, WGS84,
    Ancienne_Triangulation_Francaise,
    Nord_de_Guerre,
    [CCode (cname = "geodeticdatum_end")] geodeticdatum_end
  }

	[CCode (cname = "geounits_t", cprefix = "", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum GeoUnits {
    /* epsg_units.inc */
    Linear_Meter,
    Linear_Foot, Linear_Foot_US_Survey, Linear_Foot_Modified_American,
    Linear_Foot_Clarke, Linear_Foot_Indian,
    Linear_Link, Linear_Link_Benoit, Linear_Link_Sears,
    Linear_Chain_Benoit, Linear_Chain_Sears,
    Linear_Yard_Sears, Linear_Yard_Indian,
    Linear_Fathom,
    Linear_Mile_International_Nautical,
    /* Angular Units */
    Angular_Radian, Angular_Degree, Angular_Arc_Minute, Angular_Arc_Second,
    Angular_Grad, Angular_Gon, Angular_DMS, Angular_DMS_Hemisphere,
    Unit_End
  }

	[CCode (cname = "ellipsoid_t", cprefix = "Ellipse_", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum Ellipsoid {
    /* epsg_ellipse.inc */
    Airy_1830, Airy_Modified_1849,
    Australian_National_Spheroid,
    Bessel_1841, Bessel_Modified, Bessel_Namibia,
    Clarke_1858, Clarke_1866, Clarke_1866_Michigan,
    Clarke_1880_Benoit, Clarke_1880_IGN, Clarke_1880_RGS,
    Clarke_1880_Arc, Clarke_1880_SGA_1922,
    Everest_1830_1937_Adjustment, Everest_1830_1967_Definition,
    Everest_1830_1975_Definition, Everest_1830_Modified,
    GRS_1980,
    Helmert_1906,
    Indonesian_National_Spheroid,
    International_1924, International_1967,
    Krassowsky_1940,
    NWL_9D, NWL_10D,
    Plessis_1817,
    Struve_1860,
    War_Office,
    WGS_84,
    GEM_10C,
    OSU86F, OSU91A,
    Clarke_1880,
    Sphere,
    [CCode (cname = "ellipsoid_end")] ellipsoid_end
  }

	[CCode (cname = "primemeridian_t", cprefix = "PM_", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum PrimeMeridian {
    /* epsg_pm.inc */
    Greenwich, Lisbon, Paris, Bogota, Madrid, Rome, Bern, Jakarta, Ferro, Brussels, Stockholm,
    [CCode (cname = "primemeridian_end")] primemeridian_end
  }

	[CCode (cname = "pcstype_t", cprefix = "PCS_", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum PcsType {
    /* epsg_pcs.inc */
    /* Newer PCS */
    Hjorsey_1955_Lambert, ISN93_Lambert_1993,
    ETRS89_Poland_CS2000_zone_5, ETRS89_Poland_CS2000_zone_6,
    ETRS89_Poland_CS2000_zone_7, ETRS89_Poland_CS2000_zone_8,
    ETRS89_Poland_CS92,
    /* New PCS */
    GGRS87_Greek_Grid,
    KKJ_Finland_zone_1, KKJ_Finland_zone_2, KKJ_Finland_zone_3, KKJ_Finland_zone_4,
    RT90_2_5_gon_W,
    Lietuvos_Koordinoei_Sistema_1994,
    Estonian_Coordinate_System_of_1992,
    HD72_EOV,
    Dealul_Piscului_1970_Stereo_70,
    Adindan_UTM_zone_37N, Adindan_UTM_zone_38N,
    AGD66_AMG_zone_48, AGD66_AMG_zone_49, AGD66_AMG_zone_50, AGD66_AMG_zone_51, AGD66_AMG_zone_52,
    AGD66_AMG_zone_53, AGD66_AMG_zone_54, AGD66_AMG_zone_55, AGD66_AMG_zone_56, AGD66_AMG_zone_57,
    AGD66_AMG_zone_58, AGD84_AMG_zone_48, AGD84_AMG_zone_49, AGD84_AMG_zone_50, AGD84_AMG_zone_51,
    AGD84_AMG_zone_52, AGD84_AMG_zone_53, AGD84_AMG_zone_54, AGD84_AMG_zone_55, AGD84_AMG_zone_56,
    AGD84_AMG_zone_57, AGD84_AMG_zone_58,
    Ain_el_Abd_UTM_zone_37N, Ain_el_Abd_UTM_zone_38N, Ain_el_Abd_UTM_zone_39N, Ain_el_Abd_Bahrain_Grid,
    Afgooye_UTM_zone_38N, Afgooye_UTM_zone_39N,
    Lisbon_Portugese_Grid,
    Aratu_UTM_zone_22S, Aratu_UTM_zone_23S, Aratu_UTM_zone_24S,
    Arc_1950_Lo13, Arc_1950_Lo15, Arc_1950_Lo17, Arc_1950_Lo19,
    Arc_1950_Lo21, Arc_1950_Lo23, Arc_1950_Lo25, Arc_1950_Lo27, Arc_1950_Lo29,
    Arc_1950_Lo31, Arc_1950_Lo33, Arc_1950_Lo35,
    Batavia_NEIEZ,
    Batavia_UTM_zone_48S, Batavia_UTM_zone_49S, Batavia_UTM_zone_50S,
    Beijing_Gauss_zone_13, Beijing_Gauss_zone_14, Beijing_Gauss_zone_15, Beijing_Gauss_zone_16,
    Beijing_Gauss_zone_17, Beijing_Gauss_zone_18, Beijing_Gauss_zone_19, Beijing_Gauss_zone_20,
    Beijing_Gauss_zone_21, Beijing_Gauss_zone_22, Beijing_Gauss_zone_23,
    Beijing_Gauss_13N, Beijing_Gauss_14N, Beijing_Gauss_15N, Beijing_Gauss_16N,
    Beijing_Gauss_17N, Beijing_Gauss_18N, Beijing_Gauss_19N, Beijing_Gauss_20N,
    Beijing_Gauss_21N, Beijing_Gauss_22N, Beijing_Gauss_23N,
    Belge_Lambert_50,
    Bern_1898_Swiss_Old,
    Bogota_UTM_zone_17N, Bogota_UTM_zone_18N,
    Bogota_Colombia_3W, Bogota_Colombia_Bogota, Bogota_Colombia_3E, Bogota_Colombia_6E,
    Camacupa_UTM_32S, Camacupa_UTM_33S,
    C_Inchauspe_Argentina_1, C_Inchauspe_Argentina_2, C_Inchauspe_Argentina_3, C_Inchauspe_Argentina_4,
    C_Inchauspe_Argentina_5, C_Inchauspe_Argentina_6, C_Inchauspe_Argentina_7,
    Carthage_UTM_zone_32N, Carthage_Nord_Tunisie, Carthage_Sud_Tunisie,
    Corrego_Alegre_UTM_23S, Corrego_Alegre_UTM_24S,
    Douala_UTM_zone_32N,
    Egypt_1907_Red_Belt, Egypt_1907_Purple_Belt, Egypt_1907_Ext_Purple,
    ED50_UTM_zone_28N, ED50_UTM_zone_29N, ED50_UTM_zone_30N, ED50_UTM_zone_31N,
    ED50_UTM_zone_32N, ED50_UTM_zone_33N, ED50_UTM_zone_34N, ED50_UTM_zone_35N,
    ED50_UTM_zone_36N, ED50_UTM_zone_37N, ED50_UTM_zone_38N,
    Fahud_UTM_zone_39N, Fahud_UTM_zone_40N,
    Garoua_UTM_zone_33N,
    ID74_UTM_zone_46N, ID74_UTM_zone_47N, ID74_UTM_zone_48N, ID74_UTM_zone_49N,
    ID74_UTM_zone_50N, ID74_UTM_zone_51N, ID74_UTM_zone_52N, ID74_UTM_zone_53N,
    ID74_UTM_zone_46S, ID74_UTM_zone_47S, ID74_UTM_zone_48S, ID74_UTM_zone_49S,
    ID74_UTM_zone_50S, ID74_UTM_zone_51S, ID74_UTM_zone_52S, ID74_UTM_zone_53S,
    ID74_UTM_zone_54S,
    Indian_1954_UTM_47N, Indian_1954_UTM_48N, Indian_1975_UTM_47N, Indian_1975_UTM_48N,
    Jamaica_1875_Old_Grid,
    JAD69_Jamaica_Grid,
    Kalianpur_India_0, Kalianpur_India_I, Kalianpur_India_IIa, Kalianpur_India_IIIa,
    Kalianpur_India_IVa, Kalianpur_India_IIb, Kalianpur_India_IIIb, Kalianpur_India_IVb,
    Kertau_Singapore_Grid, Kertau_UTM_zone_47N, Kertau_UTM_zone_48N,
    La_Canoa_UTM_zone_20N, La_Canoa_UTM_zone_21N,
    PSAD56_UTM_zone_18N, PSAD56_UTM_zone_19N, PSAD56_UTM_zone_20N, PSAD56_UTM_zone_21N,
    PSAD56_UTM_zone_17S, PSAD56_UTM_zone_18S, PSAD56_UTM_zone_19S, PSAD56_UTM_zone_20S,
    PSAD56_Peru_west_zone, PSAD56_Peru_central, PSAD56_Peru_east_zone,
    Leigon_Ghana_Grid,
    Lome_UTM_zone_31N,
    Luzon_Philippines_I, Luzon_Philippines_II, Luzon_Philippines_III,
    Luzon_Philippines_IV, Luzon_Philippines_V,
    Makassar_NEIEZ,
    Malongo_1987_UTM_32S,
    Merchich_Nord_Maroc, Merchich_Sud_Maroc, Merchich_Sahara,
    Massawa_UTM_zone_37N,
    Minna_UTM_zone_31N, Minna_UTM_zone_32N,
    Minna_Nigeria_West, Minna_Nigeria_Mid_Belt, Minna_Nigeria_East,
    Mhast_UTM_zone_32S,
    Monte_Mario_Italy_1, Monte_Mario_Italy_2,
    M_poraloko_UTM_32N, M_poraloko_UTM_32S,
    NAD27_UTM_zone_3N, NAD27_UTM_zone_4N, NAD27_UTM_zone_5N, NAD27_UTM_zone_6N,
    NAD27_UTM_zone_7N, NAD27_UTM_zone_8N, NAD27_UTM_zone_9N, NAD27_UTM_zone_10N,
    NAD27_UTM_zone_11N, NAD27_UTM_zone_12N, NAD27_UTM_zone_13N, NAD27_UTM_zone_14N,
    NAD27_UTM_zone_15N, NAD27_UTM_zone_16N, NAD27_UTM_zone_17N, NAD27_UTM_zone_18N,
    NAD27_UTM_zone_19N, NAD27_UTM_zone_20N, NAD27_UTM_zone_21N, NAD27_UTM_zone_22N,
    NAD27_Alabama_East, NAD27_Alabama_West,
    NAD27_Alaska_zone_1, NAD27_Alaska_zone_2, NAD27_Alaska_zone_3, NAD27_Alaska_zone_4,
    NAD27_Alaska_zone_5, NAD27_Alaska_zone_6, NAD27_Alaska_zone_7, NAD27_Alaska_zone_8,
    NAD27_Alaska_zone_9, NAD27_Alaska_zone_10,
    NAD27_California_I, NAD27_California_II, NAD27_California_III, NAD27_California_IV,
    NAD27_California_V, NAD27_California_VI, NAD27_California_VII,
    NAD27_Arizona_East, NAD27_Arizona_Central, NAD27_Arizona_West,
    NAD27_Arkansas_North, NAD27_Arkansas_South,
    NAD27_Colorado_North, NAD27_Colorado_Central, NAD27_Colorado_South,
    NAD27_Connecticut,
    NAD27_Delaware,
    NAD27_Florida_East, NAD27_Florida_West, NAD27_Florida_North,
    NAD27_Hawaii_zone_1, NAD27_Hawaii_zone_2, NAD27_Hawaii_zone_3,
    NAD27_Hawaii_zone_4, NAD27_Hawaii_zone_5,
    NAD27_Georgia_East, NAD27_Georgia_West,
    NAD27_Idaho_East, NAD27_Idaho_Central, NAD27_Idaho_West,
    NAD27_Illinois_East, NAD27_Illinois_West,
    NAD27_Indiana_East, NAD27_BLM_14N_feet, NAD27_Indiana_West, NAD27_BLM_15N_feet,
    NAD27_Iowa_North, NAD27_BLM_16N_feet, NAD27_Iowa_South, NAD27_BLM_17N_feet,
    NAD27_Kansas_North, NAD27_Kansas_South,
    NAD27_Kentucky_North, NAD27_Kentucky_South,
    NAD27_Louisiana_North, NAD27_Louisiana_South,
    NAD27_Maine_East, NAD27_Maine_West,
    NAD27_Maryland,
    NAD27_Massachusetts, NAD27_Massachusetts_Is,
    NAD27_Michigan_North, NAD27_Michigan_Central, NAD27_Michigan_South,
    NAD27_Minnesota_North, NAD27_Minnesota_Cent, NAD27_Minnesota_South,
    NAD27_Mississippi_East, NAD27_Mississippi_West,
    NAD27_Missouri_East, NAD27_Missouri_Central, NAD27_Missouri_West,
    NAD_Michigan_Michigan_East, NAD_Michigan_Michigan_Old_Central, NAD_Michigan_Michigan_West,
    NAD83_UTM_zone_3N, NAD83_UTM_zone_4N, NAD83_UTM_zone_5N, NAD83_UTM_zone_6N,
    NAD83_UTM_zone_7N, NAD83_UTM_zone_8N, NAD83_UTM_zone_9N, NAD83_UTM_zone_10N,
    NAD83_UTM_zone_11N, NAD83_UTM_zone_12N, NAD83_UTM_zone_13N, NAD83_UTM_zone_14N,
    NAD83_UTM_zone_15N, NAD83_UTM_zone_16N, NAD83_UTM_zone_17N, NAD83_UTM_zone_18N,
    NAD83_UTM_zone_19N, NAD83_UTM_zone_20N, NAD83_UTM_zone_21N, NAD83_UTM_zone_22N,
    NAD83_UTM_zone_23N,
    NAD83_Alabama_East, NAD83_Alabama_West,
    NAD83_Alaska_zone_1, NAD83_Alaska_zone_2, NAD83_Alaska_zone_3, NAD83_Alaska_zone_4,
    NAD83_Alaska_zone_5, NAD83_Alaska_zone_6, NAD83_Alaska_zone_7, NAD83_Alaska_zone_8,
    NAD83_Alaska_zone_9, NAD83_Alaska_zone_10,
    NAD83_California_1, NAD83_California_2, NAD83_California_3,
    NAD83_California_4, NAD83_California_5, NAD83_California_6,
    NAD83_Arizona_East, NAD83_Arizona_Central, NAD83_Arizona_West,
    NAD83_Arkansas_North, NAD83_Arkansas_South,
    NAD83_Colorado_North, NAD83_Colorado_Central, NAD83_Colorado_South,
    NAD83_Connecticut,
    NAD83_Delaware,
    NAD83_Florida_East, NAD83_Florida_West, NAD83_Florida_North,
    NAD83_Hawaii_zone_1, NAD83_Hawaii_zone_2, NAD83_Hawaii_zone_3, NAD83_Hawaii_zone_4, NAD83_Hawaii_zone_5,
    NAD83_Georgia_East, NAD83_Georgia_West,
    NAD83_Idaho_East, NAD83_Idaho_Central, NAD83_Idaho_West,
    NAD83_Illinois_East, NAD83_Illinois_West,
    NAD83_Indiana_East, NAD83_Indiana_West,
    NAD83_Iowa_North, NAD83_Iowa_South,
    NAD83_Kansas_North, NAD83_Kansas_South,
    NAD83_Kentucky_North, NAD83_Kentucky_South,
    NAD83_Louisiana_North, NAD83_Louisiana_South,
    NAD83_Maine_East, NAD83_Maine_West,
    NAD83_Maryland,
    NAD83_Massachusetts, NAD83_Massachusetts_Is,
    NAD83_Michigan_North, NAD83_Michigan_Central, NAD83_Michigan_South,
    NAD83_Minnesota_North, NAD83_Minnesota_Cent, NAD83_Minnesota_South,
    NAD83_Mississippi_East, NAD83_Mississippi_West,
    NAD83_Missouri_East, NAD83_Missouri_Central, NAD83_Missouri_West,
    Nahrwan_1967_UTM_38N, Nahrwan_1967_UTM_39N, Nahrwan_1967_UTM_40N,
    Naparima_UTM_20N,
    GD49_NZ_Map_Grid, GD49_North_Island_Grid, GD49_South_Island_Grid,
    Datum_73_UTM_zone_29N,
    ATF_Nord_de_Guerre,
    NTF_France_I, NTF_France_II, NTF_France_III,
    NTF_Nord_France, NTF_Centre_France, NTF_Sud_France,
    British_National_Grid,
    Point_Noire_UTM_32S,
    GDA94_MGA_zone_48, GDA94_MGA_zone_49, GDA94_MGA_zone_50, GDA94_MGA_zone_51,
    GDA94_MGA_zone_52, GDA94_MGA_zone_53, GDA94_MGA_zone_54, GDA94_MGA_zone_55,
    GDA94_MGA_zone_56, GDA94_MGA_zone_57, GDA94_MGA_zone_58,
    Pulkovo_Gauss_zone_4, Pulkovo_Gauss_zone_5, Pulkovo_Gauss_zone_6, Pulkovo_Gauss_zone_7,
    Pulkovo_Gauss_zone_8, Pulkovo_Gauss_zone_9, Pulkovo_Gauss_zone_10, Pulkovo_Gauss_zone_11,
    Pulkovo_Gauss_zone_12, Pulkovo_Gauss_zone_13, Pulkovo_Gauss_zone_14, Pulkovo_Gauss_zone_15,
    Pulkovo_Gauss_zone_16, Pulkovo_Gauss_zone_17, Pulkovo_Gauss_zone_18, Pulkovo_Gauss_zone_19,
    Pulkovo_Gauss_zone_20, Pulkovo_Gauss_zone_21, Pulkovo_Gauss_zone_22, Pulkovo_Gauss_zone_23,
    Pulkovo_Gauss_zone_24, Pulkovo_Gauss_zone_25, Pulkovo_Gauss_zone_26, Pulkovo_Gauss_zone_27,
    Pulkovo_Gauss_zone_28, Pulkovo_Gauss_zone_29, Pulkovo_Gauss_zone_30, Pulkovo_Gauss_zone_31,
    Pulkovo_Gauss_zone_32,
    Pulkovo_Gauss_4N, Pulkovo_Gauss_5N, Pulkovo_Gauss_6N, Pulkovo_Gauss_7N,
    Pulkovo_Gauss_8N, Pulkovo_Gauss_9N, Pulkovo_Gauss_10N, Pulkovo_Gauss_11N,
    Pulkovo_Gauss_12N, Pulkovo_Gauss_13N, Pulkovo_Gauss_14N, Pulkovo_Gauss_15N,
    Pulkovo_Gauss_16N, Pulkovo_Gauss_17N, Pulkovo_Gauss_18N, Pulkovo_Gauss_19N,
    Pulkovo_Gauss_20N, Pulkovo_Gauss_21N, Pulkovo_Gauss_22N, Pulkovo_Gauss_23N,
    Pulkovo_Gauss_24N, Pulkovo_Gauss_25N, Pulkovo_Gauss_26N, Pulkovo_Gauss_27N,
    Pulkovo_Gauss_28N, Pulkovo_Gauss_29N, Pulkovo_Gauss_30N, Pulkovo_Gauss_31N,
    Pulkovo_Gauss_32N,
    Qatar_National_Grid,
    RD_Netherlands_Old, RD_Netherlands_New,
    SAD69_UTM_zone_18N, SAD69_UTM_zone_19N, SAD69_UTM_zone_20N, SAD69_UTM_zone_21N,
    SAD69_UTM_zone_22N, SAD69_UTM_zone_17S, SAD69_UTM_zone_18S, SAD69_UTM_zone_19S,
    SAD69_UTM_zone_20S, SAD69_UTM_zone_21S, SAD69_UTM_zone_22S, SAD69_UTM_zone_23S,
    SAD69_UTM_zone_24S, SAD69_UTM_zone_25S,
    Sapper_Hill_UTM_20S, Sapper_Hill_UTM_21S,
    Schwarzeck_UTM_33S,
    Sudan_UTM_zone_35N, Sudan_UTM_zone_36N,
    Tananarive_Laborde, Tananarive_UTM_38S, Tananarive_UTM_39S,
    Timbalai_1948_Borneo, Timbalai_1948_UTM_49N, Timbalai_1948_UTM_50N,
    TM65_Irish_Nat_Grid,
    Trinidad_1903_Trinidad,
    TC_1948_UTM_zone_39N, TC_1948_UTM_zone_40N,
    Voirol_N_Algerie_ancien, Voirol_S_Algerie_ancien,
    Voirol_Unifie_N_Algerie, Voirol_Unifie_S_Algerie,
    Bern_1938_Swiss_New,
    Nord_Sahara_UTM_29N, Nord_Sahara_UTM_30N, Nord_Sahara_UTM_31N, Nord_Sahara_UTM_32N,
    Yoff_UTM_zone_28N,
    Zanderij_UTM_zone_21N,
    MGI_Austria_West, MGI_Austria_Central, MGI_Austria_East,
    Belge_Lambert_72,
    DHDN_Germany_zone_1, DHDN_Germany_zone_2, DHDN_Germany_zone_3, DHDN_Germany_zone_4, DHDN_Germany_zone_5,
    NAD27_Montana_North, NAD27_Montana_Central, NAD27_Montana_South,
    NAD27_Nebraska_North, NAD27_Nebraska_South,
    NAD27_Nevada_East, NAD27_Nevada_Central, NAD27_Nevada_West,
    NAD27_New_Hampshire,
    NAD27_New_Jersey,
    NAD27_New_Mexico_East, NAD27_New_Mexico_Cent, NAD27_New_Mexico_West,
    NAD27_New_York_East, NAD27_New_York_Central, NAD27_New_York_West, NAD27_New_York_Long_Is,
    NAD27_North_Carolina,
    NAD27_North_Dakota_N, NAD27_North_Dakota_S,
    NAD27_Ohio_North, NAD27_Ohio_South,
    NAD27_Oklahoma_North, NAD27_Oklahoma_South,
    NAD27_Oregon_North, NAD27_Oregon_South,
    NAD27_Pennsylvania_N, NAD27_Pennsylvania_S,
    NAD27_Rhode_Island,
    NAD27_South_Carolina_N, NAD27_South_Carolina_S,
    NAD27_South_Dakota_N, NAD27_South_Dakota_S,
    NAD27_Tennessee,
    NAD27_Texas_North, NAD27_Texas_North_Cen, NAD27_Texas_Central, NAD27_Texas_South_Cen, NAD27_Texas_South,
    NAD27_Utah_North, NAD27_Utah_Central, NAD27_Utah_South,
    NAD27_Vermont,
    NAD27_Virginia_North, NAD27_Virginia_South,
    NAD27_Washington_North, NAD27_Washington_South,
    NAD27_West_Virginia_N, NAD27_West_Virginia_S,
    NAD27_Wisconsin_North, NAD27_Wisconsin_Cen, NAD27_Wisconsin_South,
    NAD27_Wyoming_East, NAD27_Wyoming_E_Cen, NAD27_Wyoming_W_Cen, NAD27_Wyoming_West,
    NAD27_Puerto_Rico,
    NAD27_St_Croix,
    NAD83_Montana,
    NAD83_Nebraska,
    NAD83_Nevada_East, NAD83_Nevada_Central, NAD83_Nevada_West,
    NAD83_New_Hampshire,
    NAD83_New_Jersey,
    NAD83_New_Mexico_East, NAD83_New_Mexico_Cent, NAD83_New_Mexico_West,
    NAD83_New_York_East, NAD83_New_York_Central, NAD83_New_York_West, NAD83_New_York_Long_Is,
    NAD83_North_Carolina,
    NAD83_North_Dakota_N, NAD83_North_Dakota_S,
    NAD83_Ohio_North, NAD83_Ohio_South,
    NAD83_Oklahoma_North, NAD83_Oklahoma_South,
    NAD83_Oregon_North, NAD83_Oregon_South,
    NAD83_Pennsylvania_N, NAD83_Pennsylvania_S,
    NAD83_Rhode_Island,
    NAD83_South_Carolina,
    NAD83_South_Dakota_N, NAD83_South_Dakota_S,
    NAD83_Tennessee,
    NAD83_Texas_North, NAD83_Texas_North_Cen, NAD83_Texas_Central, NAD83_Texas_South_Cen, NAD83_Texas_South,
    NAD83_Utah_North, NAD83_Utah_Central, NAD83_Utah_South,
    NAD83_Vermont,
    NAD83_Virginia_North, NAD83_Virginia_South,
    NAD83_Washington_North, NAD83_Washington_South,
    NAD83_West_Virginia_N, NAD83_West_Virginia_S,
    NAD83_Wisconsin_North, NAD83_Wisconsin_Cen, NAD83_Wisconsin_South,
    NAD83_Wyoming_East, NAD83_Wyoming_E_Cen, NAD83_Wyoming_W_Cen, NAD83_Wyoming_West,
    NAD83_Puerto_Rico_Virgin_Is,
    WGS72_UTM_zone_1N, WGS72_UTM_zone_2N, WGS72_UTM_zone_3N, WGS72_UTM_zone_4N, WGS72_UTM_zone_5N,
    WGS72_UTM_zone_6N, WGS72_UTM_zone_7N, WGS72_UTM_zone_8N, WGS72_UTM_zone_9N, WGS72_UTM_zone_10N,
    WGS72_UTM_zone_11N, WGS72_UTM_zone_12N, WGS72_UTM_zone_13N, WGS72_UTM_zone_14N, WGS72_UTM_zone_15N,
    WGS72_UTM_zone_16N, WGS72_UTM_zone_17N, WGS72_UTM_zone_18N, WGS72_UTM_zone_19N, WGS72_UTM_zone_20N,
    WGS72_UTM_zone_21N, WGS72_UTM_zone_22N, WGS72_UTM_zone_23N, WGS72_UTM_zone_24N, WGS72_UTM_zone_25N,
    WGS72_UTM_zone_26N, WGS72_UTM_zone_27N, WGS72_UTM_zone_28N, WGS72_UTM_zone_29N, WGS72_UTM_zone_30N,
    WGS72_UTM_zone_31N, WGS72_UTM_zone_32N, WGS72_UTM_zone_33N, WGS72_UTM_zone_34N, WGS72_UTM_zone_35N,
    WGS72_UTM_zone_36N, WGS72_UTM_zone_37N, WGS72_UTM_zone_38N, WGS72_UTM_zone_39N, WGS72_UTM_zone_40N,
    WGS72_UTM_zone_41N, WGS72_UTM_zone_42N, WGS72_UTM_zone_43N, WGS72_UTM_zone_44N, WGS72_UTM_zone_45N,
    WGS72_UTM_zone_46N, WGS72_UTM_zone_47N, WGS72_UTM_zone_48N, WGS72_UTM_zone_49N, WGS72_UTM_zone_50N,
    WGS72_UTM_zone_51N, WGS72_UTM_zone_52N, WGS72_UTM_zone_53N, WGS72_UTM_zone_54N, WGS72_UTM_zone_55N,
    WGS72_UTM_zone_56N, WGS72_UTM_zone_57N, WGS72_UTM_zone_58N, WGS72_UTM_zone_59N, WGS72_UTM_zone_60N,
    WGS72_UTM_zone_1S, WGS72_UTM_zone_2S, WGS72_UTM_zone_3S, WGS72_UTM_zone_4S, WGS72_UTM_zone_5S,
    WGS72_UTM_zone_6S, WGS72_UTM_zone_7S, WGS72_UTM_zone_8S, WGS72_UTM_zone_9S, WGS72_UTM_zone_10S,
    WGS72_UTM_zone_11S, WGS72_UTM_zone_12S, WGS72_UTM_zone_13S, WGS72_UTM_zone_14S, WGS72_UTM_zone_15S,
    WGS72_UTM_zone_16S, WGS72_UTM_zone_17S, WGS72_UTM_zone_18S, WGS72_UTM_zone_19S, WGS72_UTM_zone_20S,
    WGS72_UTM_zone_21S, WGS72_UTM_zone_22S, WGS72_UTM_zone_23S, WGS72_UTM_zone_24S, WGS72_UTM_zone_25S,
    WGS72_UTM_zone_26S, WGS72_UTM_zone_27S, WGS72_UTM_zone_28S, WGS72_UTM_zone_29S, WGS72_UTM_zone_30S,
    WGS72_UTM_zone_31S, WGS72_UTM_zone_32S, WGS72_UTM_zone_33S, WGS72_UTM_zone_34S, WGS72_UTM_zone_35S,
    WGS72_UTM_zone_36S, WGS72_UTM_zone_37S, WGS72_UTM_zone_38S, WGS72_UTM_zone_39S, WGS72_UTM_zone_40S,
    WGS72_UTM_zone_41S, WGS72_UTM_zone_42S, WGS72_UTM_zone_43S, WGS72_UTM_zone_44S, WGS72_UTM_zone_45S,
    WGS72_UTM_zone_46S, WGS72_UTM_zone_47S, WGS72_UTM_zone_48S, WGS72_UTM_zone_49S, WGS72_UTM_zone_50S,
    WGS72_UTM_zone_51S, WGS72_UTM_zone_52S, WGS72_UTM_zone_53S, WGS72_UTM_zone_54S, WGS72_UTM_zone_55S,
    WGS72_UTM_zone_56S, WGS72_UTM_zone_57S, WGS72_UTM_zone_58S, WGS72_UTM_zone_59S, WGS72_UTM_zone_60S,
    WGS72BE_UTM_zone_1N, WGS72BE_UTM_zone_2N, WGS72BE_UTM_zone_3N, WGS72BE_UTM_zone_4N, WGS72BE_UTM_zone_5N,
    WGS72BE_UTM_zone_6N, WGS72BE_UTM_zone_7N, WGS72BE_UTM_zone_8N, WGS72BE_UTM_zone_9N, WGS72BE_UTM_zone_10N,
    WGS72BE_UTM_zone_11N, WGS72BE_UTM_zone_12N, WGS72BE_UTM_zone_13N, WGS72BE_UTM_zone_14N, WGS72BE_UTM_zone_15N,
    WGS72BE_UTM_zone_16N, WGS72BE_UTM_zone_17N, WGS72BE_UTM_zone_18N, WGS72BE_UTM_zone_19N, WGS72BE_UTM_zone_20N,
    WGS72BE_UTM_zone_21N, WGS72BE_UTM_zone_22N, WGS72BE_UTM_zone_23N, WGS72BE_UTM_zone_24N, WGS72BE_UTM_zone_25N,
    WGS72BE_UTM_zone_26N, WGS72BE_UTM_zone_27N, WGS72BE_UTM_zone_28N, WGS72BE_UTM_zone_29N, WGS72BE_UTM_zone_30N,
    WGS72BE_UTM_zone_31N, WGS72BE_UTM_zone_32N, WGS72BE_UTM_zone_33N, WGS72BE_UTM_zone_34N, WGS72BE_UTM_zone_35N,
    WGS72BE_UTM_zone_36N, WGS72BE_UTM_zone_37N, WGS72BE_UTM_zone_38N, WGS72BE_UTM_zone_39N, WGS72BE_UTM_zone_40N,
    WGS72BE_UTM_zone_41N, WGS72BE_UTM_zone_42N, WGS72BE_UTM_zone_43N, WGS72BE_UTM_zone_44N, WGS72BE_UTM_zone_45N,
    WGS72BE_UTM_zone_46N, WGS72BE_UTM_zone_47N, WGS72BE_UTM_zone_48N, WGS72BE_UTM_zone_49N, WGS72BE_UTM_zone_50N,
    WGS72BE_UTM_zone_51N, WGS72BE_UTM_zone_52N, WGS72BE_UTM_zone_53N, WGS72BE_UTM_zone_54N, WGS72BE_UTM_zone_55N,
    WGS72BE_UTM_zone_56N, WGS72BE_UTM_zone_57N, WGS72BE_UTM_zone_58N, WGS72BE_UTM_zone_59N, WGS72BE_UTM_zone_60N,
    WGS72BE_UTM_zone_1S, WGS72BE_UTM_zone_2S, WGS72BE_UTM_zone_3S, WGS72BE_UTM_zone_4S, WGS72BE_UTM_zone_5S,
    WGS72BE_UTM_zone_6S, WGS72BE_UTM_zone_7S, WGS72BE_UTM_zone_8S, WGS72BE_UTM_zone_9S, WGS72BE_UTM_zone_10S,
    WGS72BE_UTM_zone_11S, WGS72BE_UTM_zone_12S, WGS72BE_UTM_zone_13S, WGS72BE_UTM_zone_14S, WGS72BE_UTM_zone_15S,
    WGS72BE_UTM_zone_16S, WGS72BE_UTM_zone_17S, WGS72BE_UTM_zone_18S, WGS72BE_UTM_zone_19S, WGS72BE_UTM_zone_20S,
    WGS72BE_UTM_zone_21S, WGS72BE_UTM_zone_22S, WGS72BE_UTM_zone_23S, WGS72BE_UTM_zone_24S, WGS72BE_UTM_zone_25S,
    WGS72BE_UTM_zone_26S, WGS72BE_UTM_zone_27S, WGS72BE_UTM_zone_28S, WGS72BE_UTM_zone_29S, WGS72BE_UTM_zone_30S,
    WGS72BE_UTM_zone_31S, WGS72BE_UTM_zone_32S, WGS72BE_UTM_zone_33S, WGS72BE_UTM_zone_34S, WGS72BE_UTM_zone_35S,
    WGS72BE_UTM_zone_36S, WGS72BE_UTM_zone_37S, WGS72BE_UTM_zone_38S, WGS72BE_UTM_zone_39S, WGS72BE_UTM_zone_40S,
    WGS72BE_UTM_zone_41S, WGS72BE_UTM_zone_42S, WGS72BE_UTM_zone_43S, WGS72BE_UTM_zone_44S, WGS72BE_UTM_zone_45S,
    WGS72BE_UTM_zone_46S, WGS72BE_UTM_zone_47S, WGS72BE_UTM_zone_48S, WGS72BE_UTM_zone_49S, WGS72BE_UTM_zone_50S,
    WGS72BE_UTM_zone_51S, WGS72BE_UTM_zone_52S, WGS72BE_UTM_zone_53S, WGS72BE_UTM_zone_54S, WGS72BE_UTM_zone_55S,
    WGS72BE_UTM_zone_56S, WGS72BE_UTM_zone_57S, WGS72BE_UTM_zone_58S, WGS72BE_UTM_zone_59S, WGS72BE_UTM_zone_60S,
    WGS84_UTM_zone_1N, WGS84_UTM_zone_2N, WGS84_UTM_zone_3N, WGS84_UTM_zone_4N, WGS84_UTM_zone_5N,
    WGS84_UTM_zone_6N, WGS84_UTM_zone_7N, WGS84_UTM_zone_8N, WGS84_UTM_zone_9N, WGS84_UTM_zone_10N,
    WGS84_UTM_zone_11N, WGS84_UTM_zone_12N, WGS84_UTM_zone_13N, WGS84_UTM_zone_14N, WGS84_UTM_zone_15N,
    WGS84_UTM_zone_16N, WGS84_UTM_zone_17N, WGS84_UTM_zone_18N, WGS84_UTM_zone_19N, WGS84_UTM_zone_20N,
    WGS84_UTM_zone_21N, WGS84_UTM_zone_22N, WGS84_UTM_zone_23N, WGS84_UTM_zone_24N, WGS84_UTM_zone_25N,
    WGS84_UTM_zone_26N, WGS84_UTM_zone_27N, WGS84_UTM_zone_28N, WGS84_UTM_zone_29N, WGS84_UTM_zone_30N,
    WGS84_UTM_zone_31N, WGS84_UTM_zone_32N, WGS84_UTM_zone_33N, WGS84_UTM_zone_34N, WGS84_UTM_zone_35N,
    WGS84_UTM_zone_36N, WGS84_UTM_zone_37N, WGS84_UTM_zone_38N, WGS84_UTM_zone_39N, WGS84_UTM_zone_40N,
    WGS84_UTM_zone_41N, WGS84_UTM_zone_42N, WGS84_UTM_zone_43N, WGS84_UTM_zone_44N, WGS84_UTM_zone_45N,
    WGS84_UTM_zone_46N, WGS84_UTM_zone_47N, WGS84_UTM_zone_48N, WGS84_UTM_zone_49N, WGS84_UTM_zone_50N,
    WGS84_UTM_zone_51N, WGS84_UTM_zone_52N, WGS84_UTM_zone_53N, WGS84_UTM_zone_54N, WGS84_UTM_zone_55N,
    WGS84_UTM_zone_56N, WGS84_UTM_zone_57N, WGS84_UTM_zone_58N, WGS84_UTM_zone_59N, WGS84_UTM_zone_60N,
    WGS84_UTM_zone_1S, WGS84_UTM_zone_2S, WGS84_UTM_zone_3S, WGS84_UTM_zone_4S, WGS84_UTM_zone_5S,
    WGS84_UTM_zone_6S, WGS84_UTM_zone_7S, WGS84_UTM_zone_8S, WGS84_UTM_zone_9S, WGS84_UTM_zone_10S,
    WGS84_UTM_zone_11S, WGS84_UTM_zone_12S, WGS84_UTM_zone_13S, WGS84_UTM_zone_14S, WGS84_UTM_zone_15S,
    WGS84_UTM_zone_16S, WGS84_UTM_zone_17S, WGS84_UTM_zone_18S, WGS84_UTM_zone_19S, WGS84_UTM_zone_20S,
    WGS84_UTM_zone_21S, WGS84_UTM_zone_22S, WGS84_UTM_zone_23S, WGS84_UTM_zone_24S, WGS84_UTM_zone_25S,
    WGS84_UTM_zone_26S, WGS84_UTM_zone_27S, WGS84_UTM_zone_28S, WGS84_UTM_zone_29S, WGS84_UTM_zone_30S,
    WGS84_UTM_zone_31S, WGS84_UTM_zone_32S, WGS84_UTM_zone_33S, WGS84_UTM_zone_34S, WGS84_UTM_zone_35S,
    WGS84_UTM_zone_36S, WGS84_UTM_zone_37S, WGS84_UTM_zone_38S, WGS84_UTM_zone_39S, WGS84_UTM_zone_40S,
    WGS84_UTM_zone_41S, WGS84_UTM_zone_42S, WGS84_UTM_zone_43S, WGS84_UTM_zone_44S, WGS84_UTM_zone_45S,
    WGS84_UTM_zone_46S, WGS84_UTM_zone_47S, WGS84_UTM_zone_48S, WGS84_UTM_zone_49S, WGS84_UTM_zone_50S,
    WGS84_UTM_zone_51S, WGS84_UTM_zone_52S, WGS84_UTM_zone_53S, WGS84_UTM_zone_54S, WGS84_UTM_zone_55S,
    WGS84_UTM_zone_56S, WGS84_UTM_zone_57S, WGS84_UTM_zone_58S, WGS84_UTM_zone_59S, WGS84_UTM_zone_60S,
    [CCode (cname = "pcstype_end")] pcstype_end
  }

	[CCode (cname = "projection_t", cprefix = "Proj_", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum Projection {
    /* epsg_proj.inc */
    /* New codes */
    Stereo_70,
    /* Old codes */
    Alabama_CS27_East, Alabama_CS27_West,
    Alabama_CS83_East, Alabama_CS83_West,
    Arizona_Coordinate_System_east, Arizona_Coordinate_System_Central, Arizona_Coordinate_System_west,
    Arizona_CS83_east, Arizona_CS83_Central, Arizona_CS83_west,
    Arkansas_CS27_North, Arkansas_CS27_South,
    Arkansas_CS83_North, Arkansas_CS83_South,
    California_CS27_I, California_CS27_II, California_CS27_III, California_CS27_IV,
    California_CS27_V, California_CS27_VI, California_CS27_VII,
    California_CS83_1, California_CS83_2, California_CS83_3,
    California_CS83_4, California_CS83_5, California_CS83_6,
    Colorado_CS27_North, Colorado_CS27_Central, Colorado_CS27_South,
    Colorado_CS83_North, Colorado_CS83_Central, Colorado_CS83_South,
    Connecticut_CS27,
    Connecticut_CS83,
    Delaware_CS27,
    Delaware_CS83,
    Florida_CS27_East, Florida_CS27_West, Florida_CS27_North,
    Florida_CS83_East, Florida_CS83_West, Florida_CS83_North,
    Georgia_CS27_East, Georgia_CS27_West,
    Georgia_CS83_East, Georgia_CS83_West,
    Idaho_CS27_East, Idaho_CS27_Central, Idaho_CS27_West,
    Idaho_CS83_East, Idaho_CS83_Central, Idaho_CS83_West,
    Illinois_CS27_East, Illinois_CS27_West,
    Illinois_CS83_East, Illinois_CS83_West,
    Indiana_CS27_East, Indiana_CS27_West,
    Indiana_CS83_East, Indiana_CS83_West,
    Iowa_CS27_North, Iowa_CS27_South,
    Iowa_CS83_North, Iowa_CS83_South,
    Kansas_CS27_North, Kansas_CS27_South,
    Kansas_CS83_North, Kansas_CS83_South,
    Kentucky_CS27_North, Kentucky_CS27_South,
    Kentucky_CS83_North, Kentucky_CS83_South,
    Louisiana_CS27_North, Louisiana_CS27_South,
    Louisiana_CS83_North, Louisiana_CS83_South,
    Maine_CS27_East, Maine_CS27_West,
    Maine_CS83_East, Maine_CS83_West,
    Maryland_CS27,
    Maryland_CS83,
    Massachusetts_CS27_Mainland, Massachusetts_CS27_Island,
    Massachusetts_CS83_Mainland, Massachusetts_CS83_Island,
    Michigan_State_Plane_East, Michigan_State_Plane_Old_Central, Michigan_State_Plane_West,
    Michigan_CS27_North, Michigan_CS27_Central, Michigan_CS27_South,
    Michigan_CS83_North, Michigan_CS83_Central, Michigan_CS83_South,
    Minnesota_CS27_North, Minnesota_CS27_Central, Minnesota_CS27_South,
    Minnesota_CS83_North, Minnesota_CS83_Central, Minnesota_CS83_South,
    Mississippi_CS27_East, Mississippi_CS27_West,
    Mississippi_CS83_East, Mississippi_CS83_West,
    Missouri_CS27_East, Missouri_CS27_Central, Missouri_CS27_West,
    Missouri_CS83_East, Missouri_CS83_Central, Missouri_CS83_West,
    Montana_CS27_North, Montana_CS27_Central, Montana_CS27_South,
    Montana_CS83,
    Nebraska_CS27_North, Nebraska_CS27_South,
    Nebraska_CS83,
    Nevada_CS27_East, Nevada_CS27_Central, Nevada_CS27_West,
    Nevada_CS83_East, Nevada_CS83_Central, Nevada_CS83_West,
    New_Hampshire_CS27,
    New_Hampshire_CS83,
    New_Jersey_CS27,
    New_Jersey_CS83,
    New_Mexico_CS27_East, New_Mexico_CS27_Central, New_Mexico_CS27_West,
    New_Mexico_CS83_East, New_Mexico_CS83_Central, New_Mexico_CS83_West,
    New_York_CS27_East, New_York_CS27_Central, New_York_CS27_West, New_York_CS27_Long_Island,
    New_York_CS83_East, New_York_CS83_Central, New_York_CS83_West, New_York_CS83_Long_Island,
    North_Carolina_CS27,
    North_Carolina_CS83,
    North_Dakota_CS27_North, North_Dakota_CS27_South,
    North_Dakota_CS83_North, North_Dakota_CS83_South,
    Ohio_CS27_North, Ohio_CS27_South,
    Ohio_CS83_North, Ohio_CS83_South,
    Oklahoma_CS27_North, Oklahoma_CS27_South,
    Oklahoma_CS83_North, Oklahoma_CS83_South,
    Oregon_CS27_North, Oregon_CS27_South,
    Oregon_CS83_North, Oregon_CS83_South,
    Pennsylvania_CS27_North, Pennsylvania_CS27_South,
    Pennsylvania_CS83_North, Pennsylvania_CS83_South,
    Rhode_Island_CS27,
    Rhode_Island_CS83,
    South_Carolina_CS27_North, South_Carolina_CS27_South,
    South_Carolina_CS83,
    South_Dakota_CS27_North, South_Dakota_CS27_South,
    South_Dakota_CS83_North, South_Dakota_CS83_South,
    Tennessee_CS27,
    Tennessee_CS83,
    Texas_CS27_North, Texas_CS27_North_Central, Texas_CS27_Central, Texas_CS27_South_Central, Texas_CS27_South,
    Texas_CS83_North, Texas_CS83_North_Central, Texas_CS83_Central, Texas_CS83_South_Central, Texas_CS83_South,
    Utah_CS27_North, Utah_CS27_Central, Utah_CS27_South,
    Utah_CS83_North, Utah_CS83_Central, Utah_CS83_South,
    Vermont_CS27,
    Vermont_CS83,
    Virginia_CS27_North, Virginia_CS27_South,
    Virginia_CS83_North, Virginia_CS83_South,
    Washington_CS27_North, Washington_CS27_South,
    Washington_CS83_North, Washington_CS83_South,
    West_Virginia_CS27_North, West_Virginia_CS27_South,
    West_Virginia_CS83_North, West_Virginia_CS83_South,
    Wisconsin_CS27_North, Wisconsin_CS27_Central, Wisconsin_CS27_South,
    Wisconsin_CS83_North, Wisconsin_CS83_Central, Wisconsin_CS83_South,
    Wyoming_CS27_East, Wyoming_CS27_East_Central, Wyoming_CS27_West_Central, Wyoming_CS27_West,
    Wyoming_CS83_East, Wyoming_CS83_East_Central, Wyoming_CS83_West_Central, Wyoming_CS83_West,
    Alaska_CS27_1, Alaska_CS27_2, Alaska_CS27_3, Alaska_CS27_4, Alaska_CS27_5,
    Alaska_CS27_6, Alaska_CS27_7, Alaska_CS27_8, Alaska_CS27_9, Alaska_CS27_10,
    Alaska_CS83_1, Alaska_CS83_2, Alaska_CS83_3, Alaska_CS83_4, Alaska_CS83_5,
    Alaska_CS83_6, Alaska_CS83_7, Alaska_CS83_8, Alaska_CS83_9, Alaska_CS83_10,
    Hawaii_CS27_1, Hawaii_CS27_2, Hawaii_CS27_3, Hawaii_CS27_4, Hawaii_CS27_5,
    Hawaii_CS83_1, Hawaii_CS83_2, Hawaii_CS83_3, Hawaii_CS83_4, Hawaii_CS83_5,
    Puerto_Rico_CS27,
    St_Croix,
    Puerto_Rico_Virgin_Is,
    BLM_14N_feet, BLM_15N_feet, BLM_16N_feet, BLM_17N_feet,
    UTM_zone_1N, UTM_zone_2N, UTM_zone_3N, UTM_zone_4N, UTM_zone_5N,
    UTM_zone_6N, UTM_zone_7N, UTM_zone_8N, UTM_zone_9N, UTM_zone_10N,
    UTM_zone_11N, UTM_zone_12N, UTM_zone_13N, UTM_zone_14N, UTM_zone_15N,
    UTM_zone_16N, UTM_zone_17N, UTM_zone_18N, UTM_zone_19N, UTM_zone_20N,
    UTM_zone_21N, UTM_zone_22N, UTM_zone_23N, UTM_zone_24N, UTM_zone_25N,
    UTM_zone_26N, UTM_zone_27N, UTM_zone_28N, UTM_zone_29N, UTM_zone_30N,
    UTM_zone_31N, UTM_zone_32N, UTM_zone_33N, UTM_zone_34N, UTM_zone_35N,
    UTM_zone_36N, UTM_zone_37N, UTM_zone_38N, UTM_zone_39N, UTM_zone_40N,
    UTM_zone_41N, UTM_zone_42N, UTM_zone_43N, UTM_zone_44N, UTM_zone_45N,
    UTM_zone_46N, UTM_zone_47N, UTM_zone_48N, UTM_zone_49N, UTM_zone_50N,
    UTM_zone_51N, UTM_zone_52N, UTM_zone_53N, UTM_zone_54N, UTM_zone_55N,
    UTM_zone_56N, UTM_zone_57N, UTM_zone_58N, UTM_zone_59N, UTM_zone_60N,
    UTM_zone_1S, UTM_zone_2S, UTM_zone_3S, UTM_zone_4S, UTM_zone_5S,
    UTM_zone_6S,  UTM_zone_7S, UTM_zone_8S, UTM_zone_9S, UTM_zone_10S,
    UTM_zone_11S, UTM_zone_12S, UTM_zone_13S, UTM_zone_14S, UTM_zone_15S,
    UTM_zone_16S, UTM_zone_17S, UTM_zone_18S, UTM_zone_19S, UTM_zone_20S,
    UTM_zone_21S, UTM_zone_22S, UTM_zone_23S, UTM_zone_24S, UTM_zone_25S,
    UTM_zone_26S, UTM_zone_27S, UTM_zone_28S, UTM_zone_29S, UTM_zone_30S,
    UTM_zone_31S, UTM_zone_32S, UTM_zone_33S, UTM_zone_34S, UTM_zone_35S,
    UTM_zone_36S, UTM_zone_37S, UTM_zone_38S, UTM_zone_39S, UTM_zone_40S,
    UTM_zone_41S, UTM_zone_42S, UTM_zone_43S, UTM_zone_44S, UTM_zone_45S,
    UTM_zone_46S, UTM_zone_47S, UTM_zone_48S, UTM_zone_49S, UTM_zone_50S,
    UTM_zone_51S, UTM_zone_52S, UTM_zone_53S, UTM_zone_54S, UTM_zone_55S,
    UTM_zone_56S, UTM_zone_57S, UTM_zone_58S, UTM_zone_59S, UTM_zone_60S,
    Gauss_Kruger_zone_0, Gauss_Kruger_zone_1, Gauss_Kruger_zone_2,
    Gauss_Kruger_zone_3, Gauss_Kruger_zone_4, Gauss_Kruger_zone_5,
    Map_Grid_of_Australia_48, Map_Grid_of_Australia_49, Map_Grid_of_Australia_50,
    Map_Grid_of_Australia_51, Map_Grid_of_Australia_52, Map_Grid_of_Australia_53,
    Map_Grid_of_Australia_54, Map_Grid_of_Australia_55, Map_Grid_of_Australia_56,
    Map_Grid_of_Australia_57, Map_Grid_of_Australia_58,
    Australian_Map_Grid_48, Australian_Map_Grid_49, Australian_Map_Grid_50,
    Australian_Map_Grid_51, Australian_Map_Grid_52, Australian_Map_Grid_53,
    Australian_Map_Grid_54, Australian_Map_Grid_55, Australian_Map_Grid_56,
    Australian_Map_Grid_57, Australian_Map_Grid_58,
    Argentina_1, Argentina_2, Argentina_3, Argentina_4, Argentina_5, Argentina_6, Argentina_7,
    Colombia_3W, Colombia_Bogota, Colombia_3E, Colombia_6E,
    Egypt_Red_Belt, Egypt_Purple_Belt, Extended_Purple_Belt,
    New_Zealand_North_Island_Nat_Grid, New_Zealand_South_Island_Nat_Grid,
    Bahrain_Grid,
    Netherlands_E_Indies_Equatorial,
    RSO_Borneo,
    [CCode (cname = "projection_end")] projection_end
  }

  [CCode (cname = "coordtrans_t", cprefix = "CT_", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum CoordTrans {
    /* geo_ctrans.inc */
    TransverseMercator, TransvMercator_Modified_Alaska,
    ObliqueMercator, ObliqueMercator_Laborde, ObliqueMercator_Rosenmund, ObliqueMercator_Spherical,
    Mercator,
    LambertConfConic_2SP, LambertConfConic, LambertConfConic_1SP,
    LambertConfConic_Helmert, LambertAzimEqualArea, AlbersEqualArea,
    AzimuthalEquidistant, EquidistantConic,
    Stereographic, PolarStereographic, ObliqueStereographic,
    Equirectangular, CassiniSoldner, Gnomonic, MillerCylindrical, Orthographic,
    Polyconic, Robinson, Sinusoidal, VanDerGrinten, NewZealandMapGrid,
    /* Added for 1.0 */
    TransvMercator_SouthOrientated,
    /* Added Feb 2005 */
    CylindricalEqualArea,
    /* Aliases */
    SouthOrientedGaussConformal, AlaskaConformal, TransvEquidistCylindrical,
    ObliqueMercator_Hotine, SwissObliqueCylindrical,
    GaussBoaga, GaussKruger, TransvMercator_SouthOriented,
    [CCode (cname = "coordtrans_end")] coordtrans_end
  }

  [CCode (cname = "vertcstype_t", cprefix = "VertCS_", has_type_id = false, cheader_filename = "geovalues.h")]
  public enum VertCSType {
    /* epsg_vertcs.inc */
    Airy_1830_ellipsoid, Airy_Modified_1849_ellipsoid,
    ANS_ellipsoid,
    Bessel_1841_ellipsoid, Bessel_Modified_ellipsoid, Bessel_Namibia_ellipsoid,
    Clarke_1858_ellipsoid, Clarke_1866_ellipsoid, Clarke_1880_Benoit_ellipsoid,
    Clarke_1880_IGN_ellipsoid, Clarke_1880_RGS_ellipsoid,
    Clarke_1880_Arc_ellipsoid, Clarke_1880_SGA_1922_ellipsoid,
    Everest_1830_1937_Adjustment_ellipsoid, Everest_1830_1967_Definition_ellipsoid,
    Everest_1830_1975_Definition_ellipsoid, Everest_1830_Modified_ellipsoid,
    GRS_1980_ellipsoid,
    Helmert_1906_ellipsoid,
    INS_ellipsoid,
    International_1924_ellipsoid, International_1967_ellipsoid,
    Krassowsky_1940_ellipsoid,
    NWL_9D_ellipsoid, NWL_10D_ellipsoid,
    Plessis_1817_ellipsoid,
    Struve_1860_ellipsoid,
    War_Office_ellipsoid,
    WGS_84_ellipsoid,
    GEM_10C_ellipsoid,
    OSU86F_ellipsoid, OSU91A_ellipsoid,
    /* Other established Vertical CS */
    Newlyn,
    North_American_Vertical_Datum_1929, North_American_Vertical_Datum_1988,
    Yellow_Sea_1956, Baltic_Sea, Caspian_Sea,
    [CCode (cname = "vertcs_end")] vertcs_end
  }

}
