
#include "duller.h"
#include <string.h>
#include <unistd.h> //< Для usleep.



struct _DullerPriv
{
  gint cur_type; /*!< Тип плиток для данного слоя.*/
  guint id; /*!< Идентификатор (номер) GpAsyncTiler'а.*/
};



G_DEFINE_TYPE(Duller, duller, GP_TYPE_ASYNC_TILER );


// Статические функции -->
  static GObject *immut_generate_imp(GpAsyncTiler *tiler, GObject *old_immut);

  static void tile_generate_imp(GpAsyncTiler* tiler, GObject* immut, GpTile* tile, guint8* buf);

  /// Функция просто вычищает из кеша ряд плиток.
  static gboolean check_tile(GpTile *tile, gpointer user_data);
// Статические функции <--



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
static void duller_class_init(DullerClass *klass)
{
  GpAsyncTilerClass *async_tiler_class = GP_ASYNC_TILER_CLASS(klass);

  g_type_class_add_private(klass, sizeof(DullerPriv));

  async_tiler_class->immut_generate = immut_generate_imp;
  async_tiler_class->tile_generate = tile_generate_imp;
}



gint duller_get_tiles_type(Duller *self)
{
  return DULLER(self)->priv->cur_type;
}



// Инициализация объекта.
static void duller_init(Duller *duller)
{
  duller->priv = G_TYPE_INSTANCE_GET_PRIVATE(duller, DULLER_TYPE, DullerPriv);
}



// Функция создания объекта Duller.
Duller *duller_new(GpSmartCache *cache, GpDispatcher *dispatcher)
{
  static guint STATIC_TEST_ID = 1;

  gchar *name = g_strdup_printf("Duller [%d]", STATIC_TEST_ID);

  Duller *duller = g_object_new(DULLER_TYPE,
    "cache", cache,
    "dispatcher", dispatcher,
    "name", name,
    "tile-types-num", 1,
  NULL);

  g_free(name);

  DullerPriv *priv = duller->priv;

  priv->cur_type = 0;
  priv->id = STATIC_TEST_ID;

  STATIC_TEST_ID++;

  return duller;
}



GObject *immut_generate_imp(GpAsyncTiler *tiler, GObject *old_immut)
{
  DullerPriv *priv = DULLER(tiler)->priv;

                                gp_async_tiler_data_updating_emit(tiler, 0.00);
  //~ g_usleep(G_USEC_PER_SEC / 2); gp_async_tiler_data_updating_emit(tiler, 0.25);
  //~ g_usleep(G_USEC_PER_SEC / 2); gp_async_tiler_data_updating_emit(tiler, 0.50);
  //~ g_usleep(G_USEC_PER_SEC / 2); gp_async_tiler_data_updating_emit(tiler, 0.75);
  //~ g_usleep(G_USEC_PER_SEC / 2); gp_async_tiler_data_updating_emit(tiler, 1.00);

  #if 0
    // С промаргиванием.
    g_tiler_cache_clean_by_condition(GP_TILER(tiler), priv->cur_type, check_tile, NULL);
  #else
    // Без промаргивания.
    gp_tiler_cache_mark_notactual(GP_TILER(tiler), priv->cur_type, check_tile, NULL);
  #endif

  // Пустой объект, просто чтобы не NULL вернуть.
  return g_object_new(G_TYPE_OBJECT, NULL);
}



void tile_generate_imp(GpAsyncTiler* tiler, GObject* immut, GpTile* tile, guint8* buf)
{
  DullerPriv *priv = DULLER(tiler)->priv;

  guint i, j;

  //~ usleep(1000000);
  usleep(1000000);

  static guint STATIC_TEST_ID = 0;

  // В Duller только один тип плиток, перепроверем это дело.
  g_assert(tile->type == 0);

  g_print("GENERATE: id = %d, tiler = %p\n", priv->id, tiler);

  switch(priv->id)
  {
    case 1:
      for(i = 0; i < GP_TILE_SIDE; i++)
        for(j = 0; j < GP_TILE_SIDE; j++)
          if(
            j < (GP_TILE_SIDE / 4) || j > (3 * GP_TILE_SIDE / 4) || i < (GP_TILE_SIDE / 4) || i > (3 * GP_TILE_SIDE / 4)
            )
            {
              *(buf + 4 * (i * GP_TILE_SIDE + j) + 0) = i/3;
              *(buf + 4 * (i * GP_TILE_SIDE + j) + 1) = STATIC_TEST_ID % 0xFF;
              *(buf + 4 * (i * GP_TILE_SIDE + j) + 2) = j/3;
              *(buf + 4 * (i * GP_TILE_SIDE + j) + 3) = 0xFF;

              STATIC_TEST_ID++;
            }
    break;

    case 2:
      for(i = 4 * GP_TILE_SIDE / 10; i < 6 * GP_TILE_SIDE / 10; i++)
        for(j = 0; j < GP_TILE_SIDE; j++)
          {
            *(buf + 4 * (i * GP_TILE_SIDE + j) + 0) = 0xFF;
            *(buf + 4 * (i * GP_TILE_SIDE + j) + 1) = j/3;
            *(buf + 4 * (i * GP_TILE_SIDE + j) + 2) = STATIC_TEST_ID % 0xFF;
            *(buf + 4 * (i * GP_TILE_SIDE + j) + 3) = 0xFF;

            STATIC_TEST_ID++;
          }
    break;

    default:
      g_critical("priv->id = %d", priv->id);
      g_assert_not_reached();
    break;
  }
}

