
#include "weigher.h"
#include "gp-core-gui.h"
#include <string.h>
#include <unistd.h> //< Для usleep.

static const double MIN_DEPTH = 5;
static const double MAX_DEPTH = 25;

struct _WeigherPriv
{
  gint cur_type;    /*!< Тип плиток для данного слоя.*/
  gint bottom_type; /// Тип глубины 0 - наклон по оси X, 1 - наклон по оси Y
  gint data_type;   /// Тип данных 0 - цвет, 1 - веса
  double x;
  double y;
  double w;
  double h;
  double step;
  GpPalette *plt;
};

G_DEFINE_TYPE(Weigher, weigher, GP_TYPE_TILER );

// Статические функции -->
  /// Функция просто вычищает из кеша ряд плиток.
  static gboolean check_tile(GpTile *tile, gpointer user_data);
// Статические функции <--

static GpMemTile *weigher_get_tile_from_source(GpTiler *tiler, GpTile* tile, GpTileStatus status);
gboolean weigher_get_area(GpTiler *weigher, gint type, gdouble* from_x, gdouble* to_x, gdouble* from_y, gdouble* to_y);

gboolean check_tile(GpTile *tile, gpointer user_data)
{
  return (1
    && tile->x <=  1
    && tile->x >= -1
    && tile->y <=  1
    && tile->y >= -1
  );
}

// Настройка класса.
static void weigher_class_init(WeigherClass *klass)
{
  GpTilerClass *parent_class = GP_TILER_CLASS(klass);

  g_type_class_add_private(klass, sizeof(WeigherPriv));

  parent_class->get_tile_from_source = weigher_get_tile_from_source;
  parent_class->get_area = weigher_get_area;
}

gint weigher_get_tiles_type(Weigher *self)
{
  return WEIGHER(self)->priv->cur_type;
}

// Инициализация объекта.
static void weigher_init(Weigher *weigher)
{
  weigher->priv = G_TYPE_INSTANCE_GET_PRIVATE(weigher, WEIGHER_TYPE, WeigherPriv);
}

// Функция создания объекта Weigher.
Weigher *weigher_new(GpSmartCache *cache)
{
  static guint STATIC_TEST_ID = 1;

  gchar *name = g_strdup_printf("Weigher [%d]", STATIC_TEST_ID);

  Weigher *weigher = g_object_new(WEIGHER_TYPE,
    "cache", cache,
    "name", name,
    "tile-types-num", 1,
  NULL);

  g_free(name);

  WeigherPriv *priv = weigher->priv;

  priv->cur_type = 0;

  priv->x = 0;
  priv->y = 0;
  priv->w = 0;
  priv->h = 0;
  priv->step = 0;
  priv->bottom_type = 0;
  priv->data_type = 0;

  STATIC_TEST_ID++;
  return weigher;
}

void weigher_set_bottom_type(Weigher *weigher, gint bottom_type)
{
  WeigherPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(weigher, WEIGHER_TYPE, WeigherPriv);
  priv->bottom_type = CLAMP(bottom_type, 0, 1);
}

void weigher_set_data_type(Weigher *weigher, gint data_type)
{
  WeigherPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(weigher, WEIGHER_TYPE, WeigherPriv);
  priv->data_type = CLAMP(data_type, 0, 1);
}

void weigher_set_area(Weigher *weigher, gdouble x, gdouble y, gdouble w, gdouble h, gdouble step)
{
  WeigherPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(weigher, WEIGHER_TYPE, WeigherPriv);
  priv->x = x;
  priv->y = y;
  priv->w = w;
  priv->h = h;
  priv->step = step;
}

gboolean weigher_get_area(GpTiler *weigher, gint type, gdouble* from_x, gdouble* to_x, gdouble* from_y, gdouble* to_y)
{
  WeigherPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(weigher, WEIGHER_TYPE, WeigherPriv);
  *from_x = priv->x;
  *from_y = priv->y;
  *to_x = *from_x + priv->w;
  *to_y = *from_y + priv->h;
  return TRUE;
}

gboolean is_graphical(Weigher *weigher, gint type)
{
  //~ if(type < 1)
  //~ return TRUE;
  //~ else
    return FALSE;
}

gdouble weigher_depth_generator(Weigher *weigher, gdouble x, gdouble y)
{
  WeigherPriv *priv = weigher->priv;
  gdouble res = MIN_DEPTH;

  if(priv->bottom_type == 0)
    res += (MAX_DEPTH - MIN_DEPTH) * (x - priv->x) / priv->w;
  else if(priv->bottom_type == 1)
        res += (MAX_DEPTH - MIN_DEPTH) * (y - priv->y) / priv->h;

  return res;
}

gdouble weigher_calc_weight(Weigher *weigher, gdouble x, gdouble y)
{
  WeigherPriv *priv = weigher->priv;

  gdouble res = 0.0;
  if(priv->bottom_type == 0)
    res = 1.0 - (x - priv->x) / priv->w;
  else if(priv->bottom_type == 1)
    res = 1.0 - (y - priv->y) / priv->h;

  return res;
}

GpMemTile *weigher_get_tile_from_source(GpTiler *tiler, GpTile* tile, GpTileStatus status)
{
  GpMemTile *memtile = gp_mem_tile_new_with_tile(tile, GP_TILE_STATUS_ACTUAL);
  guint8 *buf = gp_mem_tile_get_buf(memtile);

  WeigherPriv *priv = WEIGHER(tiler)->priv;

  guint i = 0, j;

  // В Weigher только один тип плиток, перепроверем это дело.
  g_assert(tile->type == 0);

  //~ g_print("GENERATE SS: tiler = %p\n", tiler);

  memset(buf, '\0', GP_TILE_DATA_SIZE);

  gdouble x_min_tile, x_max_tile, y_min_tile, y_max_tile;
  gp_tile_get_limits_in_meters(tile, &x_min_tile, &x_max_tile, &y_min_tile, &y_max_tile);

  gdouble point_x = priv->x;
  gdouble point_y = priv->y;
  gint n_x = priv->w / priv->step + 1;
  gint n_y = priv->h / priv->step + 1;

  guint8 pixel[4] = {0};

  for(i = 0; i < n_y; i++)
  {
    point_y = priv->y + priv->step * i;
    for(j = 0; j < n_x; j++)
    {
      point_x = priv->x + priv->step * j;

      if((point_x > x_min_tile - priv->step) && (point_x < x_max_tile + priv->step) && (point_y > y_min_tile - priv->step)  && (point_y < y_max_tile + priv->step))
      {
        double delta = (tile->l / 100.0) / GP_TILE_SIDE;
        int index_x = (int)((point_x - x_min_tile) / delta + 0.5);
        int index_y = GP_TILE_SIDE - (int)((point_y - y_min_tile) / delta + 0.5);

        int cur_index = (index_y * GP_TILE_SIDE + index_x) * 4;

        //~ gfloat depth = (gfloat) g_random_double_range((gdouble)MIN_DEPTH, (gdouble)MAX_DEPTH);
        gfloat depth = (gfloat) weigher_depth_generator(WEIGHER(tiler), point_x, point_y);

        if(cur_index < GP_TILE_DATA_SIZE - 3 && cur_index >= 0)
        {
          if(priv->data_type == 0)
          {
            gp_palette_get_pixel(priv->plt, 0, depth, pixel, MIN_DEPTH, MAX_DEPTH);
            if(priv->bottom_type == 0)
            {
              *(buf + cur_index + 0) = pixel[0];
              *(buf + cur_index + 1) = pixel[1];
              *(buf + cur_index + 2) = pixel[2];
            }
            else
            {
              *(buf + cur_index + 0) = pixel[2];
              *(buf + cur_index + 1) = pixel[1];
              *(buf + cur_index + 2) = pixel[0];
            }
            double coef = weigher_calc_weight(WEIGHER(tiler), point_x, point_y);
            *(buf + cur_index + 3) = (guint8)(256 * coef);//0xFF;
          }
          else if(priv->data_type == 1)
               {
                  //~ *(buf + cur_index + 0) = 0;//(guint8)(depth / 100.0);
                  //~ *(buf + cur_index + 1) = 0;//(guint8)(depth - 100 * (gint)(depth / 100.0));
                  //~ *(buf + cur_index + 2) = 0;//(guint8)((depth - (gint)depth) * 100.0);
                  *(buf + cur_index + 0) = (guint8)(depth / 100.0);
                  *(buf + cur_index + 1) = (guint8)(depth - 100 * (gint)(depth / 100.0));
                  *(buf + cur_index + 2) = (guint8)((depth - (gint)depth) * 100.0);
                  *(buf + cur_index + 3) = (guint8)(weigher_calc_weight(WEIGHER(tiler), point_x, point_y) * 100);
                  //~ *(buf + cur_index + 0) = 0;
                  //~ *(buf + cur_index + 1) = (guint8)depth;
                  //~ *(buf + cur_index + 2) = 0;
                  //~ *(buf + cur_index + 3) = (guint8)(weigher_calc_weight(WEIGHER(tiler), point_x, point_y) * 100);
               }
        }
      }
    }
  }

  return memtile;
}

