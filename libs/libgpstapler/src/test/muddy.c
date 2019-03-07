
#include <gp-core.h>
#include "gp-core-gui.h"
#include "muddy.h"
#include "weigher.h"
#include <string.h>
#include <unistd.h> //< Для usleep.

static const double MIN_DEPTH = 5;
static const double MAX_DEPTH = 25;

struct _MuddyPriv
{
  gint cur_type;    /*!< Тип плиток для данного слоя.*/
  double x;
  double y;
  double w;
  double h;
  double step;
  GpPalette *plt;
  GtkTreeModel *model;
  gint tiler_col;
  gint visible_col;
  gint sum_type; /// Тип суммирования 0-усреднение, 1-по весам

  GpTiler *tiler_w[2];
};

G_DEFINE_TYPE(Muddy, muddy, GP_TYPE_TILER );


// Статические функции -->
  /// Функция просто вычищает из кеша ряд плиток.
  static gboolean check_tile(GpTile *tile, gpointer user_data);
// Статические функции <--

static GpMemTile *muddy_get_tile_from_source(GpTiler *tiler, GpTile* tile, GpTileStatus status);

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
static void muddy_class_init(MuddyClass *klass)
{
  GpTilerClass *parent_class = GP_TILER_CLASS(klass);

  g_type_class_add_private(klass, sizeof(MuddyPriv));

  parent_class->get_tile_from_source = muddy_get_tile_from_source;
}

gint muddy_get_tiles_type(Muddy *self)
{
  return MUDDY(self)->priv->cur_type;
}

// Инициализация объекта.
static void muddy_init(Muddy *muddy)
{
  muddy->priv = G_TYPE_INSTANCE_GET_PRIVATE(muddy, MUDDY_TYPE, MuddyPriv);
}

// Функция создания объекта Muddy.
Muddy *muddy_new(GpSmartCache *cache)
{
  gchar *name = g_strdup_printf("Muddy");

  Muddy *muddy = g_object_new(MUDDY_TYPE,
    "cache", cache,
    "name", name,
    "tile-types-num", 1,
  NULL);

  g_free(name);

  MuddyPriv *priv = muddy->priv;

  priv->cur_type = 0;

  priv->x = 0;
  priv->y = 0;
  priv->w = 0;
  priv->h = 0;
  priv->step = 0.1;
  priv->tiler_w[0] = NULL;
  priv->tiler_w[1] = NULL;
  priv->sum_type = 0;

  return muddy;
}

gboolean determ_area (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
  MuddyPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(MUDDY(data), MUDDY_TYPE, MuddyPriv);
  GpTiler *tl = NULL;
  gchar *str = NULL;
  gtk_tree_model_get(model, iter, priv->tiler_col, &tl, 1, &str, -1);

  gint n = 0;

  if(WEIGHER_IS(tl) == TRUE)
  {
    for(n = 0; n < 2; n++)
    {
      if(priv->tiler_w[n] == NULL)
      {
        priv->tiler_w[n] = tl;
        printf("[%d] %s\n", n, str);
        return FALSE;
      }
    }
  }

  return FALSE;
}

void muddy_set_area(Muddy *muddy, gdouble x, gdouble y, gdouble w, gdouble h, gdouble step)
{
  MuddyPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(muddy, MUDDY_TYPE, MuddyPriv);

  priv->x = x;
  priv->y = y;
  priv->w = w;
  priv->h = h;
  priv->step = step;
}

void muddy_set_sum_type(Muddy *muddy, gint sum_type)
{
  MuddyPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(muddy, MUDDY_TYPE, MuddyPriv);
  priv->sum_type = CLAMP(sum_type, 0, 1);
}

void muddy_set_model(Muddy *muddy, GtkTreeModel *model, gint tiler_col, gint visible_col)
{
  MuddyPriv *priv = G_TYPE_INSTANCE_GET_PRIVATE(muddy, MUDDY_TYPE, MuddyPriv);
  priv->model = model;
  priv->tiler_col = tiler_col;
  priv->visible_col = visible_col;

  gtk_tree_model_foreach(priv->model, (GtkTreeModelForeachFunc)determ_area, muddy);
}

GpMemTile *muddy_get_tile_from_source(GpTiler *tiler, GpTile* tile, GpTileStatus status)
{
  GpMemTile *memtile = gp_mem_tile_new_with_tile(tile, GP_TILE_STATUS_ACTUAL);
  guint8 *buf = gp_mem_tile_get_buf(memtile);

  MuddyPriv *priv = MUDDY(tiler)->priv;

  guint i;

  // В Muddy только один тип плиток, перепроверем это дело.
  g_assert(tile->type == 0);

  //~ g_print("GENERATE: tiler = %p\n", tiler);

  guint8 pixel[4] = {0};
  guint8 data_buf_1[GP_TILE_DATA_SIZE];
  guint8 data_buf_2[GP_TILE_DATA_SIZE];

  memset(data_buf_1, 0, GP_TILE_DATA_SIZE);
  memset(data_buf_2, 0, GP_TILE_DATA_SIZE);

  gfloat depth = 0, depth1, depth2, w1, w2;

  gfloat del = 2.0;

  // TODO FIXME: ниже требует только актуальные плитки. Это должно работать в офлайн режиме,
  // но в онлайн режиме нужно не брезговать и плитками со статусами, отличными от актуального.
  // Нужно сохранять возвращаемые значения get_tile, как-то рассчитывать и возвращать
  // текущий статус получившейся на основе других плиток новой плитки,
  // либо учитывать новый параметр status метода get_tile_from_source.
  gp_tiler_get_tile (priv->tiler_w[0], data_buf_1, tile, GP_TILE_STATUS_ACTUAL - 1);
  gp_tiler_get_tile (priv->tiler_w[1], data_buf_2, tile, GP_TILE_STATUS_ACTUAL - 1);

  for(i = 0; i < GP_TILE_DATA_SIZE - 4; i += 4)
  {
    depth1 = (gfloat)data_buf_1[i + 0] * 100.0 + (gfloat)data_buf_1[i + 1] + (gfloat)data_buf_1[i + 2] / 100.0;
    depth2 = (gfloat)data_buf_2[i + 0] * 100.0 + (gfloat)data_buf_2[i + 1] + (gfloat)data_buf_2[i + 2] / 100.0;
    del = 2.0;

    if(priv->sum_type == 1)
    {
      w1 = (gfloat)data_buf_1[i + 3];
      w2 = (gfloat)data_buf_2[i + 3];

      depth = depth1;

      if(w2 > w1)
        depth = depth2;
    }
    else if(priv->sum_type == 0)
    {
      if(depth2 < MIN_DEPTH || depth1 < MIN_DEPTH)
        del = 1.0;

       depth = (depth1 + depth2) / del;
    }


    //~ gfloat depth = (gfloat) g_random_double_range( MIN_DEPTH, MAX_DEPTH);
    gp_palette_get_pixel(priv->plt, 1, depth, pixel, MIN_DEPTH, MAX_DEPTH);

    if(depth > 1.0)
    {
      buf[i + 0] = pixel[0];
      buf[i + 1] = pixel[1];
      buf[i + 2] = pixel[2];
      buf[i + 3] = pixel[3];
    }
  }

  return memtile;
}

