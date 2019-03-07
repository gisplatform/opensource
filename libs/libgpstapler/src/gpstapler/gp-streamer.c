/*!
 * \file gp-streamer.c
 *
 * \author Gennady Nefediev
 * \date 17.08.2017
 *
 * Объект Streamer формирует несколько плиток и экспортирует изображение в файл.
 *
*/

#include "gp-streamer.h"
#include <gp-core.h>
#include <gp-stapler.h>

#if GEOTIFF_FOUND
  //#include <tiffio.h>
  #include <xtiffio.h>
  #include <geotiffio.h>
#endif


#define  GP_TILES_PER_FILE  100
#define  GP_TILES_PER_SIDE  30
#define  GP_TILES_PER_BUF   GP_TILES_PER_SIDE + 1


struct _GpStreamerPriv
{
  gdouble from_x;  ///Границы всей области отображения
  gdouble to_x;
  gdouble from_y;
  gdouble to_y;

  gdouble x;    ///Начало области отображения для отдельного файла
  gdouble y;
  gdouble x2;   ///Конец области отображения для отдельного файла
  gdouble y2;

  gint nx;  ///Кол-во плиток в отдельном файле
  gint ny;

  gint side;    ///Размер стороны плитки, см
  gint type;    ///Тип плиток
  guint zone;   ///Зона UTM

  gboolean started;  ///Запущена подготовка изображения и запись в очередной файл
  gboolean single;   ///Экспорт укладывается в один файл

  GpTileStatus *tile_status;  ///Статус формирования для каждой плитки
  uint32_t *tiles_buf;        ///Массив пикселей, плитка за плиткой

  gint fcnt;         ///Счетчик файлов
  gchar *file_name;  ///Полный путь и имя файла, но без расширения
  gchar *file_ext;   ///Расширение файла

  gchar *act_name;  ///Имя активности
  gdouble acnt;     ///Счетчик активности
  gdouble frac;     ///Приращение счетчика активности
  GCancellable *cancellable;  ///Опциональный объект для отмены процесса записи данных

  GpTiler *tiler;
};


#define GP_STREAMER_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), GP_STREAMER_TYPE, GpStreamerPriv))

G_DEFINE_TYPE( GpStreamer, gp_streamer, G_TYPE_INITIALLY_UNOWNED);


static void gp_streamer_finalize( GObject *object);
static gboolean gp_streamer_save_one_image( GpStreamer *streamer, gdouble x, gdouble y, gdouble x2, gdouble y2);
#if GEOTIFF_FOUND
static gboolean gp_streamer_save_gtiff( GpStreamer *streamer, void *buf, gint pix_x, gint pix_y, gdouble cx, gdouble cy, const gchar *filename, GError **error);
#endif


//Настройка класса
static void gp_streamer_class_init( GpStreamerClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

  gobject_class->finalize = gp_streamer_finalize;

  g_type_class_add_private( klass, sizeof(GpStreamerPriv));
}

//Инициализация объекта
static void gp_streamer_init( GpStreamer *streamer)
{
  streamer->priv = G_TYPE_INSTANCE_GET_PRIVATE( streamer, GP_STREAMER_TYPE, GpStreamerPriv);
}

//Уничтожение объекта
static void gp_streamer_finalize( GObject *object)
{
  GpStreamer *streamer = GP_STREAMER(object);
  GpStreamerPriv *priv = streamer->priv;

  if (priv->tile_status) g_free( priv->tile_status);
  if (priv->tiles_buf) g_free( priv->tiles_buf);
  if (priv->file_name) g_free( priv->file_name);
  if (priv->file_ext) g_free( priv->file_ext);
  if (priv->act_name) g_free( priv->act_name);

  g_clear_object( &priv->cancellable);

  G_OBJECT_CLASS(gp_streamer_parent_class)->finalize(object);
}


///GQuark для использования в исключениях GpStreamer.
GQuark gp_streamer_error_quark(void)
{
  return g_quark_from_static_string("gpstreamer-error");
}


///Метод создания объекта Streamer
GpStreamer *gp_streamer_new( void)
{
  GpStreamer *streamer = g_object_new( GP_STREAMER_TYPE, NULL);

  GpStreamerPriv *priv = streamer->priv;

  priv->tiler = NULL;

  priv->from_x = 0;
  priv->to_x = 0;
  priv->from_y = 0;
  priv->to_y = 0;

  priv->type = 0;
  priv->side = 0;

  priv->frac = 0;
  priv->acnt = 0;
  priv->fcnt = 0;
  priv->started = FALSE;
  priv->single = FALSE;

  priv->tile_status = NULL;
  priv->tiles_buf = NULL;

  priv->act_name = NULL;
  priv->file_name = NULL;
  priv->file_ext = NULL;

  priv->cancellable = g_cancellable_new();

  return streamer;
}


void gp_streamer_set_area( GpStreamer *streamer, gdouble x_min, gdouble x_max, gdouble y_min, gdouble y_max)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  priv->from_x = x_min;
  priv->to_x = x_max;
  priv->from_y = y_min;
  priv->to_y = y_max;

  priv->x = priv->from_x;
  priv->y = priv->to_y;
  priv->x2 = priv->to_x;
  priv->y2 = priv->from_y;
}


void gp_streamer_set_utm_zone( GpStreamer *streamer, guint zone)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  priv->zone = zone;
}


void gp_streamer_set_tile_side( GpStreamer *streamer, gint side)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  priv->side = side;
}


void gp_streamer_set_view_type( GpStreamer *streamer, gint type)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  priv->type = type;
}


void gp_streamer_set_file_name( GpStreamer *streamer, gchar *name, gchar *ext)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  priv->file_name = g_strdup( name);
  priv->file_ext = g_strdup( ext);
}


void gp_streamer_set_tiler( GpStreamer *streamer, GpTiler *tiler)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  priv->tiler = tiler;
}


void gp_streamer_set_act_name( GpStreamer *streamer, gchar *name)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  priv->act_name = g_strdup( name);
}


const char* gp_streamer_get_act_name( GpStreamer *streamer)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  if (priv->act_name) return priv->act_name;
    else return "Unknown activity name";
}


const GCancellable* gp_streamer_get_cancellable( GpStreamer *streamer)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  return priv->cancellable;
}


gdouble gp_streamer_get_fraction( GpStreamer *streamer)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  return priv->acnt;
}


///Метод экспорта в файл
gboolean gp_streamer_save( GpStreamer *streamer)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  g_return_val_if_fail( priv->from_x < priv->to_x && priv->from_y < priv->to_y, TRUE);
  g_return_val_if_fail( priv->side, TRUE);
  g_return_val_if_fail( priv->tiler, TRUE);

  if (g_cancellable_is_cancelled(priv->cancellable)) return TRUE;


  gdouble width = priv->side / 100.0;  //Сторона плитки в метрах
  gdouble step = GP_TILES_PER_SIDE * width; //Смещение в метрах от файла к файлу


  //Настройка первого фрагмента (возможно единственного)
  if (!priv->tile_status)
  {
    //Кол-во плиток для всего экспорта
    gint x_size = (gint)(priv->to_x / width) - (gint)(priv->from_x / width) + 1;
    gint y_size = (gint)(priv->to_y / width) - (gint)(priv->from_y / width) + 1;

    if (x_size <= GP_TILES_PER_BUF && y_size <= GP_TILES_PER_BUF) priv->single = TRUE;

    //Кол-во плиток для отдельного файла
    priv->nx = (x_size < GP_TILES_PER_BUF) ? x_size : GP_TILES_PER_BUF;
    priv->ny = (y_size < GP_TILES_PER_BUF) ? y_size : GP_TILES_PER_BUF;
    priv->frac = 1.0 / (gdouble)((x_size / priv->nx + 1) * (y_size / priv->ny + 1));

    //Резервируем буферы
    gint mem_size = priv->nx * priv->ny * GP_TILE_DATA_SIZE;
    priv->tiles_buf = g_try_malloc0( mem_size);
    priv->tile_status = g_try_malloc0( priv->nx * priv->ny * sizeof(GpTileStatus));

    if (!priv->tile_status || !priv->tiles_buf)
    {
      g_critical("%s: Cannot allocate enough memory: %d bytes", G_STRFUNC, mem_size);
      return TRUE;
    }

    //Нач. координаты для первого буфера
    priv->x = priv->from_x;
    priv->y = priv->to_y;

    //Конеч. координаты для первого буфера
    priv->x2 = (GP_TILES_PER_BUF < x_size) ? priv->x + step : priv->to_x;
    priv->y2 = (GP_TILES_PER_BUF < y_size) ? priv->y - step : priv->from_y;

    priv->started = TRUE;  //Первый буфер настроен
  }


  //Настройка каждого последующего фрагмента
  if (priv->started == FALSE)
  {
    //Смотрим, к каким плиткам относятся координаты для след. порций
    gint x_cur = (gint)((priv->x + step) / width);
    gint y_cur = (gint)((priv->y - step) / width);

    gint x_num = (gint)(priv->to_x / width);
    gint y_num = (gint)(priv->from_y / width);

    //Выходим ли за рамки экспорта по обеим осям?
    if (x_cur >= x_num && y_cur <= y_num) return TRUE;

    //Обнуляем буферы под следующий файл
    memset( priv->tiles_buf, 0, priv->nx * priv->ny * GP_TILE_DATA_SIZE);
    memset( priv->tile_status, 0, priv->nx * priv->ny * sizeof(GpTileStatus));

    //Нач. координаты файла
    if (x_cur >= x_num)  //Сдвиг вниз и на начало по горизонтали
    {
      priv->x = priv->from_x;
      priv->y -= step;
    }
    else
      priv->x += step;  //Сдвиг вправо

    //Конеч. координаты файла
    x_cur = (gint)((priv->x + step) / width);
    y_cur = (gint)((priv->y - step) / width);

    priv->x2 = (x_cur < x_num) ? priv->x + step : priv->to_x;
    priv->y2 = (y_cur > y_num) ? priv->y - step : priv->from_y;

    priv->started = TRUE;  //Буфер настроен
  }


  //Готовим плитки и пишем в файл.
  gp_streamer_save_one_image( streamer, priv->x, priv->y, priv->x2, priv->y2);

  return FALSE;
}


//Подготовить пиксельный буфер на заданное количество плиток, сделать запрос на получение каждой плитки,
//дождаться, пока все они будут готовы, и вызвать метод для записи буфера в файл.
//Возвращает TRUE, если попытка записи завершена -- удачно или нет.
//Возвращает FALSE, если продолжается подготовка каких-то плиток.
static gboolean gp_streamer_save_one_image( GpStreamer *streamer, gdouble x, gdouble y, gdouble x2, gdouble y2)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  GpTile tile;
  tile.l = priv->side;
  tile.type = priv->type;

  //Количество плиток
  gdouble width = priv->side / 100.0;
  gint x_tiles = (gint)(x2 / width) - (gint)(x / width) + 1;
  gint y_tiles = (gint)(y / width) - (gint)(y2 / width) + 1;

  gint i, j, tile_cnt;
  gboolean rval = TRUE;
  GpTileStatus status;

  //Для каждой плитки надо сделать запрос и проверить статус готовности
  for( i=0; i < y_tiles; i++)
    for( j=0; j < x_tiles; j++)
    {
      tile_cnt = i * x_tiles + j;

      if (priv->tile_status[ tile_cnt ] != GP_TILE_STATUS_ACTUAL)
      {
        tile.x = (gint)(x / width) + j;
        tile.y = (gint)(y / width) - i;

        status = (GpTileStatus)gp_tiler_get_tile(priv->tiler,
          (guint8 *)priv->tiles_buf + tile_cnt * GP_TILE_DATA_SIZE, &tile,
          GP_TILE_STATUS_ACTUAL - 1); //< Нужны актуальные данные, другие не подойдут!

        if (status == GP_TILE_STATUS_ACTUAL)
          priv->tile_status[ tile_cnt ] = GP_TILE_STATUS_ACTUAL;
        else
          rval = FALSE;
      }
    }

  //Проверить признак, все ли плитки готовы
  if (rval == FALSE) return FALSE;

/*
  //Все плитки готовы -- надо собрать общий буфер
  for( i=0; i < y_tiles; i++)
    for( j=0; j < x_tiles; j++)
      for( n=0; n < GP_TILE_SIDE; n++)
        memcpy( (uint8 *)priv->image_buf + (i * x_tiles) * GP_TILE_DATA_SIZE + (n * x_tiles + j) * GP_TILE_STRIDE,
                (uint8 *)priv->tiles_buf + (i * x_tiles + j) * GP_TILE_DATA_SIZE + n * GP_TILE_STRIDE,
                GP_TILE_STRIDE);
*/

  //ARGB32 -> ABGR
  for( i=0; i < x_tiles * y_tiles * GP_TILE_DATA_SIZE / 4; i++)
  {
    guint32 d = priv->tiles_buf[i];
    priv->tiles_buf[i] = (0xFF00FF00 & d) + (0xFF0000 & (d<<16)) + (0xFF & (d>>16));
    //priv->tiles_buf[i] |= 0xFF000000;
  }


  //Запись в файл
  gchar *fname = NULL;
  if (priv->single == TRUE)
    fname = g_strdup_printf("%s.%s", priv->file_name, priv->file_ext);
  else
    fname = g_strdup_printf("%s_%02d.%s", priv->file_name, priv->fcnt, priv->file_ext);

#if GEOTIFF_FOUND
  GError *err = NULL;

  gdouble cx = ((int)(x / width)) * width;  //utm-координаты для верхней левой точки буфера
  gdouble cy = ((int)(y / width) + 1) * width;

  gp_streamer_save_gtiff( streamer, priv->tiles_buf, x_tiles, y_tiles, cx, cy, fname, &err);

  if (err)
  {
    g_critical("%s", err->message);
    g_error_free( err);
  }
#else
  #warning "GEOTIFF_FOUND =  FALSE"
  g_warning("GpStreamer: GEOTIFF_FOUND =  FALSE");
#endif

  if (fname) g_free( fname);

  priv->fcnt++;
  priv->started = FALSE;

  priv->acnt += priv->frac;

  return TRUE;
}


#if GEOTIFF_FOUND
//Записать буфер на nx * ny плиток в gtiff-файл.
//cx, cy -- utm-координаты левой верхней точки буфера.
//Возвращает TRUE, если файл успешно записан.
static gboolean gp_streamer_save_gtiff( GpStreamer *streamer, void *buf, gint nx, gint ny, gdouble cx, gdouble cy, const gchar *filename, GError **error)
{
  GpStreamerPriv *priv = GP_STREAMER_GET_PRIVATE( streamer);

  g_return_val_if_fail(buf != NULL, FALSE);
  g_return_val_if_fail(nx > 0 && ny > 0, FALSE);
  g_return_val_if_fail(1 <= priv->zone && priv->zone <= 60, FALSE);

  TIFF *ftif = NULL;
  GTIF *gtif = NULL;

  //Открыть файл на запись
  if ((ftif = TIFFOpen(filename, "w")) == NULL)
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_OPEN, "%s: Error opening file %s.", G_STRFUNC, filename);
    return FALSE;
  }

  gint pix_x = nx * GP_TILE_SIDE;
  gint pix_y = ny * GP_TILE_SIDE;

  //Установить тэги для записи данных
  if (TIFFSetField( ftif, TIFFTAG_IMAGEWIDTH, pix_x) == 0 ||
      TIFFSetField( ftif, TIFFTAG_IMAGELENGTH, pix_y) == 0 ||
      TIFFSetField( ftif, TIFFTAG_TILEWIDTH, GP_TILE_SIDE) == 0 ||
      TIFFSetField( ftif, TIFFTAG_TILELENGTH, GP_TILE_SIDE) == 0 ||
      TIFFSetField( ftif, TIFFTAG_BITSPERSAMPLE, 8) == 0 ||
      TIFFSetField( ftif, TIFFTAG_SAMPLESPERPIXEL, 4) == 0 ||
      TIFFSetField( ftif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) == 0 ||
      TIFFSetField( ftif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB) == 0 ||
      TIFFSetField( ftif, TIFFTAG_COMPRESSION, COMPRESSION_NONE) == 0)
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_TIFF_TAG, "%s: Error setting tiff tags in %s.", G_STRFUNC, filename);
    goto failure;
  }

  //Записать массив пикселей
  gint i, j;
  uint8 *p = (uint8 *)buf;

  for( i=0; i < ny; i++)
    for( j=0; j < nx; j++)
    {
      if (TIFFWriteTile( ftif, p, j * GP_TILE_SIDE, i * GP_TILE_SIDE, 0, 0) != GP_TILE_DATA_SIZE)
      {
        g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_TIFF_WRITE, "%s: Error writing to %s.", G_STRFUNC, filename);
        goto failure;
      }

      p += GP_TILE_DATA_SIZE;
    }

  //Добавить дополнительные тэги
  const TIFFFieldInfo field_info[6] = {
    { TIFFTAG_GEOPIXELSCALE, -1, -1, TIFF_DOUBLE, (guint16)FIELD_CUSTOM, 1, 1, "GeoPixelScale" },
    { TIFFTAG_GEOTRANSMATRIX, -1, -1, TIFF_DOUBLE, (guint16)FIELD_CUSTOM, 1, 1, "GeoTransformationMatrix" },
    { TIFFTAG_GEOTIEPOINTS, -1, -1, TIFF_DOUBLE, (guint16)FIELD_CUSTOM, 1, 1, "GeoTiePoints" },
    { TIFFTAG_GEOKEYDIRECTORY, -1, -1, TIFF_SHORT, (guint16)FIELD_CUSTOM, 1, 1, "GeoKeyDirectory" },
    { TIFFTAG_GEODOUBLEPARAMS, -1, -1, TIFF_DOUBLE, (guint16)FIELD_CUSTOM, 1, 1, "GeoDoubleParams" },
    { TIFFTAG_GEOASCIIPARAMS, -1, -1, TIFF_ASCII, (guint16)FIELD_CUSTOM, 1, 1, "GeoASCIIParams" }
  };

  if (TIFFMergeFieldInfo( ftif, field_info, 6) != 0)
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_TIFF_MERGE, "%s: Error merging tiff tags in %s.", G_STRFUNC, filename);
    goto failure;
  }

  //Открыть файл как gtif
  if ((gtif = GTIFNew(ftif)) == NULL)
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_GTIFF_OPEN, "%s: Error opening gtif-descriptor for file %s.", G_STRFUNC, filename);
    goto failure;
  }

  //Установить ключи для utm-проекции
  guint zone_id = (cy < 0) ? PCS_WGS84_UTM_zone_1S - 1 + priv->zone : PCS_WGS84_UTM_zone_1N - 1 + priv->zone;

  if (GTIFKeySet( gtif, GTRasterTypeGeoKey, TYPE_SHORT, 1, RasterPixelIsPoint) == 0 ||
      GTIFKeySet( gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelProjected) == 0 ||
      GTIFKeySet( gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, zone_id) == 0 ||
      GTIFKeySet( gtif, ProjLinearUnitsGeoKey, TYPE_SHORT, 1, Linear_Meter) == 0)
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_GTIFF_KEYSET, "%s: Error setting gtif keys in %s.", G_STRFUNC, filename);
    goto failure;
  }

  //Привязать две точки к координатам
  gdouble scale = priv->side / (100.0 * GP_TILE_SIDE);

  gdouble tiepoint_tag[12] = { 0, 0, 0, cx, cy, 0,
    pix_x, pix_y, 0, cx + (gdouble)pix_x * scale, cx + (gdouble)pix_y * scale, 0 };

  if (tiepoint_tag[4] < 0) tiepoint_tag[4] += GP_UTM_N0;
  if (tiepoint_tag[10] < 0) tiepoint_tag[10] += GP_UTM_N0;

  if (TIFFSetField( ftif, TIFFTAG_GEOTIEPOINTS, 12, tiepoint_tag) == 0)
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_GTIFF_POINTS, "%s: Error setting gtif tiepoints in %s.", G_STRFUNC, filename);
    goto failure;
  }

  //Установить масштаб
  gdouble pixelscale_tag[3] = { scale, scale, 0 };

  if (TIFFSetField( ftif, TIFFTAG_GEOPIXELSCALE, 3, pixelscale_tag) == 0)
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_GTIFF_SCALE, "%s: Error setting gtif pixel scale in %s.", G_STRFUNC, filename);
    goto failure;
  }

  //Записать ключи
  if (GTIFWriteKeys(gtif) == 0)
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_GTIFF_WRKEYS, "%s: Error writing gtif keys scale in %s.", G_STRFUNC, filename);
    goto failure;
  }

//g_printf("%s: %s\n", G_STRFUNC, filename);

  GTIFFree(gtif);
  TIFFClose(ftif);

  return TRUE;

failure:

  if (gtif) GTIFFree(gtif);
  if (ftif) TIFFClose(ftif);

  return FALSE;
}
#endif

/*
//Записать буфер на pix_x * pix_y пикселей в tiff-файл.
//Возвращает TRUE, если файл успешно записан.
static gboolean gp_streamer_save_tiff( GpStreamer *streamer, void *buf, gint pix_x, gint pix_y, const gchar *filename, GError **error)
{
  g_return_if_fail( buf != NULL);
  g_return_if_fail( pix_x > 0 && pix_y > 0);

  TIFF *ftif = TIFFOpen(filename, "w");

  if (ftif == NULL)
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_OPEN, "%s: Error opening file %s.", G_STRFUNC, filename);
    return FALSE;
  }

  gboolean result = TRUE;

  if (TIFFSetField( ftif, TIFFTAG_IMAGEWIDTH, pix_x) != 0 &&
      TIFFSetField( ftif, TIFFTAG_IMAGELENGTH, pix_y) != 0 &&
      TIFFSetField( ftif, TIFFTAG_BITSPERSAMPLE, 8) != 0 &&
      TIFFSetField( ftif, TIFFTAG_SAMPLESPERPIXEL, 4) != 0 &&
      TIFFSetField( ftif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) != 0 &&
      TIFFSetField( ftif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB) != 0 &&
      TIFFSetField( ftif, TIFFTAG_COMPRESSION, COMPRESSION_NONE) != 0)
  {
    gint i;
    for( i=0; i < pix_y; i++)
    {
      if (TIFFWriteScanline( ftif, (uint8 *)buf + i * pix_x * 4, i, 0) != 1)
      {
        g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_TIFF_WRITE, "%s: Error writing to %s.", G_STRFUNC, filename);
        result = FALSE;
        break;
      }
    }
  }
  else
  {
    g_set_error( error, GP_STREAMER_ERROR, GP_STREAMER_ERROR_TIFF_TAG, "%s: Error setting tiff tag in %s.", G_STRFUNC, filename);
    result = FALSE;
  }

  TIFFClose(ftif);
  return result;
}
*/
