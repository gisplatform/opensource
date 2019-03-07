[CCode (cheader_filename = "gdal.h")]
namespace Gdal
{
// CPL - Common Portability Library.
[CCode (cheader_filename = "cpl_conv.h")]
namespace Cpl
{
  [CCode (cname="CPLSetConfigOption")]
  void set_config_option(string key, string? value);
}

public class DriverManager
{
  [CCode (cname="GDALGetDriverByName")]
  public static unowned Driver? get_driver_by_name(string name);
  [CCode (cname="GDALGetDriver")]
  public static unowned Driver? get_driver(int idx);

  [CCode (cname="GDALGetDriverCount")]
  public static int get_driver_count();

  [CCode (cname="GDALAllRegister")]
  public static void register_all();
}

[CCode (cname = "GDALDriverH", free_function = "GDALDestroyDriver")]
[Compact]
public class Driver : Gdal.MajorObject
{
  [CCode (cname = "GDALCreate")]
  public Dataset? create(string name, int xsize, int ysize, int bands, DataType type,
      [CCode (array_null_terminated = true)] string[] options);
}

[CCode (cname = "GDALMajorObjectH")]
[Compact]
public class MajorObject
{
  [CCode (cname="GDALGetDescription")]
  public unowned string get_description();

  [CCode (cname="GDALGetMetadata")]
  public string[] get_metadata(string domain = "");
}

[CCode (cname = "struct GDALDataType")]
public enum DataType
{
  [CCode (cname = "GDT_Unknown")]
  UNKNOWN,
  [CCode (cname = "GDT_Byte")]
  BYTE,
  [CCode (cname = "GDT_UInt16")]
  UINT16,
  [CCode (cname = "GDT_Int16")]
  INT16
}

[CCode (cname = "GDALAccess")]
public enum Access
{
  [CCode (cname = "GA_ReadOnly")]
  READ_ONLY,
  [CCode (cname = "GA_Update")]
  UPDATE
}


// Open in read-only mode.  Used by GDALOpenEx().
public const int OF_READONLY;

// Open in update mode. Used by GDALOpenEx().
public const int OF_UPDATE;

// Allow raster and vector drivers to be used. Used by GDALOpenEx().
public const int OF_ALL;

// Allow raster drivers to be used. Used by GDALOpenEx().
public const int OF_RASTER;

// Allow vector drivers to be used. Used by GDALOpenEx().
public const int OF_VECTOR;


// Allow gnm drivers to be used. Used by GDALOpenEx().
public const int OF_GNM;


[CCode (cname = "GDALVectorTranslateOptionsForBinary", cheader_filename = "gdal_utils.h")]
[Compact]
public class VectorTranslateOptionsForBinary
{
}


[CCode (cname = "GDALVectorTranslateOptions", free_function = "GDALVectorTranslateOptionsFree", cheader_filename = "gdal_utils.h")]
[Compact]
public class VectorTranslateOptions
{
  [CCode (cname = "GDALVectorTranslateOptionsNew 	")]
  public VectorTranslateOptions(
    [CCode (array_length = false, array_null_terminated = true)] string[] argv,
    VectorTranslateOptionsForBinary? options_for_binary = null);
}


[CCode (cname = "GDALDatasetH", free_function = "GDALClose")]
[Compact]
public class Dataset : MajorObject
{
  [CCode (cname = "GDALOpen")]
  public Dataset(string filename, Access access);

  [CCode (cname = "GDALOpenEx")]
  public Dataset.ex(string filename, uint open_flags,
    [CCode (array_length = false, array_null_terminated = true)] string[]? allowed_drivers = null,
    [CCode (array_length = false, array_null_terminated = true)] string[]? open_options = null,
    [CCode (array_length = false, array_null_terminated = true)] string[]? sibling_files = null);

  [CCode (cname = "GDALGetDatasetDriver")]
  public unowned Driver get_driver();

  [CCode (cname = "GDALVectorTranslate", cheader_filename = "gdal_utils.h")]
  public static Dataset? vector_translate(string? dest, Dataset? dst_ds,
    [CCode (array_length_pos = 2.5)] Dataset[] src_ds,
    VectorTranslateOptions? options_in, out int usage_error);
}
}
