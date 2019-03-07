/*!
 * \file gp-streamer.h
 *
 * \author Gennady Nefediev
 * \date 17.08.2017
 *
 * Объект Streamer формирует несколько плиток и экспортирует изображение в файл.
 *
*/

#ifndef _gp_streamer_h
#define _gp_streamer_h

#include <glib.h>
#include "gp-tiler.h"


G_BEGIN_DECLS


#define GP_STREAMER_TYPE       gp_streamer_get_type()
#define GP_STREAMER(obj)       (G_TYPE_CHECK_INSTANCE_CAST((obj), GP_STREAMER_TYPE, GpStreamer))
#define GP_STREAMER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass), GP_STREAMER_TYPE, GpStreamerClass))
#define GP_STREAMER_IS(obj)      (G_TYPE_CHECK_INSTANCE_TYPE((obj), GP_STREAMER_TYPE))
#define GP_STREAMER_CLASS_IS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), GP_STREAMER_TYPE))
#define GP_STREAMER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_INTERFACE((obj), GP_STREAMER_TYPE, GpStreamerClass))

typedef struct _GpStreamer GpStreamer;
typedef struct _GpStreamerClass GpStreamerClass;
typedef struct _GpStreamerPriv GpStreamerPriv;

GType gp_streamer_get_type(void);

struct _GpStreamer
{
  GInitiallyUnowned parent;
  GpStreamerPriv *priv;
};

struct _GpStreamerClass
{
  GInitiallyUnownedClass parent_class;
};


/**
* GpStreamerError:
* @GP_STREAMER_ERROR_OPEN: Невозможно открыть файл на запись.
* @GP_STREAMER_ERROR_TIFF_TAG: Ошибка записи тэгов tiff-файла.
* @GP_STREAMER_ERROR_TIFF_WRITE: Ошибка записи данных в tiff-файл.
* @GP_STREAMER_ERROR_TIFF_MERGE: Ошибка добавления тэгов в tiff-файл.
* @GP_STREAMER_ERROR_GTIFF_OPEN: Невозможно открыть файл как gtif.
* @GP_STREAMER_ERROR_GTIFF_KEYSET: Ошибка установки ключей.
* @GP_STREAMER_ERROR_GTIFF_POINTS: Ошибка установки координат.
* @GP_STREAMER_ERROR_GTIFF_SCALE: Ошибка установки шкалы.
* @GP_STREAMER_ERROR_GTIFF_WRKEYS: Ошибка записи ключей.
*
* Виды исключений GpStreamer.
*/
typedef enum
{
  GP_STREAMER_ERROR_OPEN,
  GP_STREAMER_ERROR_TIFF_TAG,
  GP_STREAMER_ERROR_TIFF_WRITE,
  GP_STREAMER_ERROR_TIFF_MERGE,
  GP_STREAMER_ERROR_GTIFF_OPEN,
  GP_STREAMER_ERROR_GTIFF_KEYSET,
  GP_STREAMER_ERROR_GTIFF_POINTS,
  GP_STREAMER_ERROR_GTIFF_SCALE,
  GP_STREAMER_ERROR_GTIFF_WRKEYS
}
GpStreamerError;

/**
* GP_STREAMER_ERROR:
*
* Макрос для получения #GError quark для ошибок объекта GpStreamer.
*/
#define GP_STREAMER_ERROR (gp_streamer_error_quark())
GQuark gp_streamer_error_quark(void) G_GNUC_CONST;


/**
 * gp_streamer_new:
 *
 * Создание объекта.
 *
 * Returns: (type GpStreamer): указатель на объект.
 */
GpStreamer *gp_streamer_new( void);


/**
 * gp_streamer_get_fraction:
 *
 * Returns: значение счетчика активности.
 */
gdouble gp_streamer_get_fraction( GpStreamer *streamer);


/**
 * gp_streamer_get_cancellable:
 *
 * Returns: (type GCancellable): указатель на объект для отмены активности.
 */
const GCancellable* gp_streamer_get_cancellable( GpStreamer *streamer);


/**
 * gp_streamer_get_act_name:
 *
 * Получить имя файла активности.
 */
const char* gp_streamer_get_act_name( GpStreamer *streamer);


/**
 * gp_streamer_set_act_name:
 * @name: имя активности.
 *
 * Указать имя файла активности.
 */
void gp_streamer_set_act_name( GpStreamer *streamer, gchar *name);


/**
 * gp_streamer_set_tiler:
 * @tiler: указатель на объект Tiler, предназначенный для формирования плиток.
 *
 * Указать Tiler.
 */
void gp_streamer_set_tiler( GpStreamer *streamer, GpTiler *tiler);


/**
 * gp_streamer_set_file_name:
 * @name: полный путь и имя файла без расширения.
 * @ext: расширение файла.
 *
 * Указать имя файла для экспорта.
 */
void gp_streamer_set_file_name( GpStreamer *streamer, gchar *name, gchar *ext);


/**
 * gp_streamer_set_tile_side:
 * @value: размер стороны плитки.
 *
 * Указать размер стороны плитки (в сантиметрах).
 */
void gp_streamer_set_tile_side( GpStreamer *streamer, gint value);


/**
 * gp_streamer_set_view_type:
 * @tiles_type: тип плиток.
 *
 * Указать тип плиток.
 */
void gp_streamer_set_view_type( GpStreamer *streamer, gint tiles_type);


/**
 * gp_streamer_set_utm_zone:
 * @zone: номер utm-зоны.
 *
 * Установить utm-зону для записи координат в файл.
 */
void gp_streamer_set_utm_zone( GpStreamer *streamer, guint zone);


/**
 * gp_streamer_set_area:
 * @from_x: левая х-координата изображения.
 * @to_x: правая х-координата изображения.
 * @from_y: нижняя y-координата изображения.
 * @to_y: верхняя y-координата изображения.
 *
 * Установить область отображения.
 */
void gp_streamer_set_area( GpStreamer *streamer, gdouble from_x, gdouble to_x, gdouble from_y, gdouble to_y);


/**
 * gp_streamer_set_all_tiles:
 * @streamer: указатель на объект #GpStreamer.
 *
 * Создает пиксельный буфер для всей выбранной области отображения и сбрасывает его в файлы.
 * 
 * Returns: TRUE в случае успеха или наоборот -- полного эпик-фейла. FALSE в случае если еще не все плитки готовы.
 */
gboolean gp_streamer_save( GpStreamer *streamer);


G_END_DECLS

#endif // _gp_streamer_h
