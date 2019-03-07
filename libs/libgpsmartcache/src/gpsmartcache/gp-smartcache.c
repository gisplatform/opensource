/*
 * GP_SMART_CACHE - data caching library, this library is part of GRTL.
 *
 * Copyright 2010 Zankov Peter
 *
 * This file is part of GRTL.
 *
 * GRTL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GRTL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GRTL. If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*!
 * \file gp-smartcache.c
 *
 * \author Zankov Peter
 * \date 27.09.2010
 *
 * \brief Исходный файл библиотеки кэширования данных.
 *
 * Все функции, описанные в этом файле, являются внутренними функциями
 * библиотеки \link gpsmartcache \endlink и не должны использоваться напрямую.
 *
 */

#include <glib.h>
#include <stdlib.h>
#include "gp-smartcache.h"
#include <string.h>

/*! \cond */

#ifdef GP_SMART_CACHE_ENABLE_GUI
  #include <glib/gi18n-lib.h>
  #include <gp-core-gui.h>
  static void gp_smart_cache_prefable_interface_init(GpPrefableIface *iface);

  /*
   * Функция выполняет действия, необходимые для обновления статистики в GUI.
   * Должна вызываться только под локом!
   */
  static void free_changed(GpSmartCache *cache);

  G_DEFINE_TYPE_EXTENDED(GpSmartCache, gp_smart_cache, G_TYPE_OBJECT, 0,
    G_IMPLEMENT_INTERFACE(GP_TYPE_PREFABLE, gp_smart_cache_prefable_interface_init));
#else
  #define free_changed(C) do {} while(0)
  G_DEFINE_TYPE(GpSmartCache, gp_smart_cache, G_TYPE_OBJECT);
#endif

typedef struct _GpSmartCachePriv GpSmartCachePriv;
#define GP_SMART_CACHE_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( ( obj ), GP_SMART_CACHE_TYPE, GpSmartCachePriv ) )

typedef struct _record_id
{
   guint group;
   guint index;
} RecordId;

typedef struct _record
{
   RecordId id;

   gpointer data;
   guint size;

   GList *access; // <-- соответствующий элемент из списка идентификаторов данных
} Record;

struct _GpSmartCachePriv
{
   GMutex mutex;
   GFreeFunc free_func; // <-- функция освобождения данных

   GArray *groups; // <-- выданные идентификаторы груп

   gsize size; // <-- максимальный размер кэша
   gsize free; // <-- счетчик свободной памяти

   Record *records; // <-- массив данных, отсортирован идентификатору
   guint records_count;

   GList *access; // <-- список идентификаторов данных, отсортирован по времени доступа

  #ifdef GP_SMART_CACHE_ENABLE_GUI
    GtkWidget *grid; // <-- виджет, содержащий настройки GpSmartCache
    gboolean mapped; // <-- флаг (под мьютексом), что виджет grid сейчас на экране
    gboolean refresh_gui_pending; // <-- флаг (под мьютексом), что создан и еще не отработал g_idle на обновление GUI со статистикой кэша
    GtkWidget *spin; // <--  unowned-ссылка на виджет, содержащий поле для ввода размера кеша
    GtkWidget *progress; // <-- unowned-ссылка на виджет, показывающий степень заполнения кеша
  #endif
};

static inline RecordId *record_id_copy( RecordId *id )
{
   RecordId *self;

   self = g_malloc( sizeof(RecordId) );

   self->group = id->group;
   self->index = id->index;

   return self;
}

static inline gint record_id_cmp( RecordId *a, RecordId *b )
{
   if (a->group > b->group)
      return 1;
   else
      if (a->group < b->group)
         return -1;
      else
      {
         if (a->index > b->index)
            return 1;
         else
            if (a->index < b->index)
               return -1;
            else
               return 0;
      }
}

/*
 * Поиск записи с указанным идентификатором.
 * Если запись существует, вернет TRUE и запишет в n ее номер.
 * Если запись не найдена, вернет FALSE и запишет в n номер элемента, рядом с которым она должны была находиться.
 */
static inline gboolean find_record( Record *records, guint records_count, guint group, guint index, guint *n )
{
   guint start = 0;
   guint end = records_count - 1;
   RecordId id =
   { .group = group, .index = index };

   if ( !records_count)
   {
      *n = 0;
      return FALSE;
   }

   while ( TRUE )
   {
      guint mid;
      gint cmp;

      mid = start + ( end - start ) / 2; // вычисление середины с защитой от переполнения
      cmp = record_id_cmp( &id, &records[mid].id );

      // Найдена запись с указанным идентификатором
      if (cmp == 0)
      {
         *n = mid;
         return TRUE;
      }

      // Запись с указанным идентификатором не найдена
      if (start == end)
      {
         *n = start;
         return FALSE;
      }

      // Делим отрезок пополам
      if (cmp > 0)
         start = mid + 1;
      else
         if (cmp < 0)
            end = mid;
   }
}

/*
 * Освобождение памяти кэша.
 * Вернет количество особожденных байт.
 */
static inline guint free_space( Record **records, guint *records_count, GFreeFunc free_func, GList **access,
                                gsize space )
{
   guint rec_n;
   gsize freed = 0;
   guint initial_count = *records_count;

   if ( ! *records_count)
      return 0;

   // Для ускорения работы изменим направление списка
   *access = g_list_reverse( *access );

   // Удаляем старые данные
   while ( freed < space && *access )
   {
      GList *link;
      RecordId *link_id;

      // Берем звено с последним временем доступа
      link = g_list_first( *access );
      link_id = (RecordId*) link->data;

      // Найдем данные, соответствующие звену
      g_assert( find_record( *records, *records_count, link_id->group, link_id->index, &rec_n ) );

      // Удалим данные
      free_func( ( *records )[rec_n].data );
      freed += ( *records )[rec_n].size;

      // Сместим данные
      if ( *records_count > rec_n + 1)
         memmove( &( *records )[rec_n], &( *records )[rec_n + 1], ( *records_count - ( rec_n + 1 ) ) * sizeof(Record) );

      *records_count -= 1;

      // Если размер массива данных сильно уменьшился, освободим его память
      if ( *records_count <= initial_count / 2)
      {
         *records = g_realloc( *records, *records_count * sizeof(Record) );
         initial_count = *records_count;
      }

      // Удалим звено
      g_free( link_id );
      *access = g_list_delete_link( *access, link );
   }

   // Вернем направление списка
   *access = g_list_reverse( *access );

   return freed;
}

/*
 * Находит элемент, расположенный слева или справа от номера n и относящийся к группе group.
 * Если такой элемент существует, вернет TRUE и сохранит его номер в n.
 */
static inline gboolean find_neighbour_record( Record *records, guint records_count, guint group, guint *n )
{
   if ( *n >= 0 && *n < records_count)
      if (records[ *n].id.group == group)
         return TRUE;

   if ( *n > 0 && *n < records_count)
      if (records[ *n - 1].id.group == group)
      {
         *n -= 1;
         return TRUE;
      }

   if ( *n >= 0 && *n < records_count - 1)
      if (records[ *n + 1].id.group == group)
      {
         *n += 1;
         return TRUE;
      }

   return FALSE;
}

/*
 * Перемещает звено в начало списка.
 */
static inline void make_first( GList **access, GList *link )
{
   *access = g_list_remove_link( *access, link );

   if ( *access)
      ( *access )->prev = link;

   link->next = *access;

   *access = link;
}

guint gp_smart_cache_reg_group (GpSmartCache *self)
{
   GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );
   guint group;
   gboolean generate = TRUE;

   g_mutex_lock( &priv->mutex );

   // Сгенерируем случайный номер
   group = rand();

   // Посмотрим, нет ли уже такой группы
   while ( generate )
   {
      guint n;

      generate = FALSE;

      for ( n = 0; n < priv->groups->len; n++ )
         if (g_array_index( priv->groups, guint, n ) == group)
         {
            // Возьмем следующий номер
            group++ ;

            generate = TRUE;
         }
   }

   g_array_append_val( priv->groups, group );

   g_mutex_unlock( &priv->mutex );

   return group;
}



GpSmartCache *gp_smart_cache_new ()
{
  return g_object_new(GP_SMART_CACHE_TYPE, NULL);
}


void gp_smart_cache_unreg_group (GpSmartCache *self, guint group)
{
   GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );
   guint n;

   g_mutex_lock( &priv->mutex );

   for ( n = 0; n < priv->groups->len; n++ )
      if (g_array_index( priv->groups, guint, n ) == group)
      {
         g_array_remove_index( priv->groups, n );
         break;
      }

   g_mutex_unlock( &priv->mutex );
}

void gp_smart_cache_set_size (GpSmartCache *self, gsize size)
{
   GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );

   g_mutex_lock( &priv->mutex );

   // Очистим кэш
   priv->free += free_space( &priv->records, &priv->records_count, priv->free_func, &priv->access, priv->size );
   g_assert( priv->free == priv->size );

   priv->size = size;
   priv->free = size;
   free_changed(self);

   g_mutex_unlock( &priv->mutex );
}

gsize gp_smart_cache_get_size (GpSmartCache *self)
{
   GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );
   gsize size;

   g_mutex_lock( &priv->mutex );

   size = priv->size;

   g_mutex_unlock( &priv->mutex );

   return size;
}

gsize gp_smart_cache_get_free (GpSmartCache *self)
{
   GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );
   gsize free;

   g_mutex_lock( &priv->mutex );

   free = priv->free;

   g_mutex_unlock( &priv->mutex );

   return free;
}


void gp_smart_cache_set_unowned(GpSmartCache *self, guint group, guint index, gpointer data, guint size)
{
  gp_smart_cache_set(self, group, index, g_memdup(data, size), size);
}


void gp_smart_cache_set (GpSmartCache *self, guint group, guint index, gpointer data, guint size)
{
   GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );
   RecordId id =
   { .group = group, .index = index };
   guint rec_n;

   g_mutex_lock( &priv->mutex );

   if (priv->size < size)
   {
      priv->free_func( data );
      g_mutex_unlock( &priv->mutex );
      return;
   }

   gsize old_free = priv->free;

   // Освободим место
   if (priv->free < size)
   {
      priv->free += free_space( &priv->records, &priv->records_count, priv->free_func, &priv->access, size
            * GP_SMART_CACHE_FREE_K );
      g_assert( priv->free >= size );
   }

   // Поищем данные с таким-же идентификатором в кэше
   if (find_record( priv->records, priv->records_count, group, index, &rec_n ))
   {
      /*
       * Найдена запись с указанным идентификатором.
       * Заменим ее данные.
       */

      // Переместим звено в начало списка
      make_first( &priv->access, priv->records[rec_n].access );

      // Удалим старые данные
      priv->free_func( priv->records[rec_n].data );
      priv->free += priv->records[rec_n].size;

      // Сохраним данные
      priv->records[rec_n].data = data;
      priv->records[rec_n].size = size;
      priv->free -= size;
   }
   else
   {
      guint new_rec_n = 0; // номер новой записи в массиве

      /*
       * Запись с указанным идентификатором не найдена.
       * Добавим новую запись.
       */

      // Найдем куда вставить новый элемент
      if (priv->records_count)
      {
         if (record_id_cmp( &id, &priv->records[rec_n].id ) < 0)
            new_rec_n = rec_n;
         else
            new_rec_n = rec_n + 1;
      }

      priv->records = g_realloc( priv->records, ( priv->records_count + 1 ) * sizeof(Record) );

      // Сместим данные, выделив место под новый элемент
      if (priv->records_count > new_rec_n)
         memmove( &priv->records[new_rec_n + 1], &priv->records[new_rec_n], ( priv->records_count - new_rec_n )
               * sizeof(Record) );

      // Добавим звено в начало списка
      priv->access = g_list_prepend( priv->access, record_id_copy( &id ) );

      // Сохраним данные
      priv->records[new_rec_n].id = id;
      priv->records[new_rec_n].data = data;
      priv->records[new_rec_n].size = size;
      priv->records[new_rec_n].access = g_list_first( priv->access );

      priv->free -= size;
      priv->records_count++ ;
   }

   if(priv->free != old_free)
     free_changed(self);

   g_mutex_unlock( &priv->mutex );
}

gboolean gp_smart_cache_get (GpSmartCache *self, guint group, guint index, guint *size, gpointer buff, guint buff_size)
{
  return gp_smart_cache_get2 (self, group, index, size, buff, buff_size, NULL, 0);
}

gboolean gp_smart_cache_get2 (GpSmartCache *self, guint group, guint index, guint *size, gpointer buff1,
                              guint buff1_size, gpointer buff2, guint buff2_size)
{
  g_return_val_if_fail(self, FALSE);
  GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );
  guint rec_n;

  g_mutex_lock( &priv->mutex );

  // Найдем элемент
  if ( !find_record( priv->records, priv->records_count, group, index, &rec_n ))
  {
    g_mutex_unlock( &priv->mutex );
    return FALSE;
  }

  Record *record = priv->records + rec_n;

  // Переместим звено в начало списка
  make_first( &priv->access, record->access );

  // Вернем данные -->
    if(size)
      *size = record->size;

    if(buff1 && buff1_size)
      memcpy(buff1, record->data, MIN(buff1_size, record->size));

    if(G_UNLIKELY(buff2))
      if(buff2_size && buff1_size < record->size)
        memcpy(buff2, record->data + buff1_size,
          MIN(buff2_size, record->size - buff1_size));
  // Вернем данные <--

  g_mutex_unlock( &priv->mutex );

  return TRUE;
}


void gp_smart_cache_modify (GpSmartCache *self, guint group,
                            GpSmartCacheAccessFunc condition, gpointer condition_user_data,
                            GpSmartCacheAccessFunc modifier, gpointer modifier_user_data)
{
  //g_return_if_fail( condition );
  g_return_if_fail( modifier );

  GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );
  guint left, right;
  guint n;

  g_mutex_lock( &priv->mutex );

  if ( !priv->records_count )
  {
    g_mutex_unlock( &priv->mutex );
    return;
  }

  find_record( priv->records, priv->records_count, group, 0, &left );
  if ( !find_neighbour_record( priv->records, priv->records_count, group, &left ))
  {
    g_mutex_unlock( &priv->mutex );
    return;
  }

  find_record( priv->records, priv->records_count, group, G_MAXUINT, &right );
  if ( !find_neighbour_record( priv->records, priv->records_count, group, &right ))
  {
    g_mutex_unlock( &priv->mutex );
    return;
  }

  if(!condition)
  {
    for ( n = left; n <= right; n++ )
      modifier( priv->records[n].data, priv->records[n].size, modifier_user_data );
  }
  else
  {
    for ( n = left; n <= right; n++ )
      if( condition(priv->records[n].data, priv->records[n].size, condition_user_data ))
        modifier( priv->records[n].data, priv->records[n].size, modifier_user_data );
  }

  g_mutex_unlock( &priv->mutex );
}


void gp_smart_cache_clean (GpSmartCache *self, guint group)
{
   gp_smart_cache_clean_by_condition (self, group, NULL, NULL);
}


void gp_smart_cache_clean_by_condition (GpSmartCache *self, guint group, GpSmartCacheAccessFunc condition,
                                        gpointer user_data)
{
  GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );
  guint left, right;
  guint n;
  guint deleted_records_num = 0;

  g_mutex_lock( &priv->mutex );

  if ( !priv->records_count)
  {
    g_mutex_unlock( &priv->mutex );
    return;
  }

  find_record( priv->records, priv->records_count, group, 0, &left );
  if ( !find_neighbour_record( priv->records, priv->records_count, group, &left ))
  {
    g_mutex_unlock( &priv->mutex );
    return;
  }

  find_record( priv->records, priv->records_count, group, G_MAXUINT, &right );
  if ( !find_neighbour_record( priv->records, priv->records_count, group, &right ))
  {
    g_mutex_unlock( &priv->mutex );
    return;
  }

  if(!condition)
    for ( n = left; n <= right; n++ ) // Удалим все данные, без условий.
    {
      // Удалим данные
      priv->free_func( priv->records[n].data );
      priv->free += priv->records[n].size;
      free_changed(self);

      // Удалим звено
      g_free( priv->records[n].access->data );
      priv->access = g_list_delete_link( priv->access, priv->records[n].access );

      deleted_records_num++;
    }
  else
    for ( n = left; n <= right; n++ ) // Удалим данные по условию condition.
    {
      if(condition(priv->records[n].data, priv->records[n].size, user_data))
      {
        // Удалим данные
        priv->free_func( priv->records[n].data );
        priv->free += priv->records[n].size;
        free_changed(self);

        // Удалим звено
        g_free( priv->records[n].access->data );
        priv->access = g_list_delete_link( priv->access, priv->records[n].access );

        deleted_records_num++;
      }
      else
      {
        // Элемент не удален, сместим его в массиве ближе к началу,
        // к последнему неудаленному элементу слева от него.
        priv->records[n - deleted_records_num] = priv->records[n];
      }
    }

  // Сместим данные
  if (priv->records_count > right + 1)
    memmove( &priv->records[right + 1 - deleted_records_num], &priv->records[right + 1],
      ( priv->records_count - ( right + 1 ) ) * sizeof(Record) );

  priv->records_count -= deleted_records_num;

  g_mutex_unlock( &priv->mutex );
}


static void gp_smart_cache_init (GpSmartCache *self)
{
   GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );

   g_mutex_init(&priv->mutex);
   priv->free_func = g_free;

   priv->groups = g_array_new( FALSE, FALSE, sizeof(guint) );
   priv->size = 0;
   priv->free = 0;

   priv->access = NULL;
   priv->records = NULL;
   priv->records_count = 0;
}

static void gp_smart_cache_finalize (GpSmartCache *self)
{
   GObjectClass *parent_class = g_type_class_peek_parent( g_type_class_peek(GP_SMART_CACHE_TYPE ) );
   GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE( self );

   #ifdef GP_SMART_CACHE_ENABLE_GUI
     g_clear_object(&priv->grid);
   #endif

   g_mutex_lock( &priv->mutex );

     // Очистим кэш
     priv->free += free_space( &priv->records, &priv->records_count, priv->free_func, &priv->access, priv->size );
     g_assert( priv->free == priv->size );

     g_array_free( priv->groups, TRUE );

   g_mutex_unlock( &priv->mutex );

   g_mutex_clear(&priv->mutex);

   parent_class->finalize( (GObject*) self );
}

static void gp_smart_cache_class_init(GpSmartCacheClass *self)
{
   GObjectClass *this_class = G_OBJECT_CLASS( self );
   this_class->finalize = (void*) gp_smart_cache_finalize;

   g_type_class_add_private( self, sizeof(GpSmartCachePriv) );
}

#ifdef GP_SMART_CACHE_ENABLE_GUI
  static void config_load(GpPrefable* self, GKeyFile* kf, GError** error)
  {
    g_return_if_fail(error == NULL || *error == NULL);

    GError *internal_error = NULL;
    guint64 size = g_key_file_get_uint64(kf, gp_prefable_config_group(self), "size", &internal_error);

    if(internal_error == NULL)
      gp_smart_cache_set_size(GP_SMART_CACHE(self), (gsize)size);
    else
      g_propagate_prefixed_error(error, internal_error, _("Failed to set cache size: "));
  }

  static void config_save(GpPrefable* self, GKeyFile* kf)
  {
    g_key_file_set_uint64(kf, gp_prefable_config_group(self), "size",
      (guint64)gp_smart_cache_get_size(GP_SMART_CACHE(self)));
  }

  static void refresh_gui(GpSmartCache *cache)
  {
    GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE(cache);
    g_return_if_fail(priv);

    g_mutex_lock(&priv->mutex);
      gsize size = priv->size;
      gsize free = priv->free;
      priv->refresh_gui_pending = FALSE;
    g_mutex_unlock(&priv->mutex);

    gchar str[16];
    g_snprintf(str, sizeof(str), "%zu", (size - free) / (1024 * 1024));
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(priv->progress), str);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->progress), (gdouble)(size - free) / size);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->spin), (gdouble)(size / (1024 * 1024)));
  }

  static void spin_value_changed(GtkSpinButton *spin_button, GpSmartCache *cache)
  {
    GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE(cache);
    g_return_if_fail(priv);

    gp_smart_cache_set_size(cache, (gsize)(1024 * 1024) * gtk_spin_button_get_value(GTK_SPIN_BUTTON(priv->spin)));
  }

  static void grid_map(GtkWidget *grid, GpSmartCache *cache)
  {
    GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE(cache);
    g_return_if_fail(priv);

    g_mutex_lock(&priv->mutex);
      priv->mapped = TRUE;
    g_mutex_unlock(&priv->mutex);

    refresh_gui(cache);
  }

  static void grid_unmap(GtkWidget *grid, GpSmartCache *cache)
  {
    GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE(cache);
    g_return_if_fail(priv);

    priv->mapped = FALSE;
  }

  static void free_changed(GpSmartCache *cache)
  {
    GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE(cache);
    g_return_if_fail(priv);

    if(priv->mapped && !priv->refresh_gui_pending)
    {
      g_idle_add_full(G_PRIORITY_LOW, (GSourceFunc)refresh_gui, g_object_ref(cache), g_object_unref);
      priv->refresh_gui_pending = TRUE;
    }
  }

  static GtkWidget* get_pref_widget(GpPrefable* self)
  {
    GpSmartCache *cache = GP_SMART_CACHE(self);
    g_return_val_if_fail(cache, NULL);

    GpSmartCachePriv *priv = GP_SMART_CACHE_GET_PRIVATE(cache);

    if(priv->grid == NULL)
    {
      priv->grid = gtk_grid_new();
      g_signal_connect(priv->grid, "map", G_CALLBACK(grid_map), self);
      g_signal_connect(priv->grid, "unmap", G_CALLBACK(grid_unmap), self);
      gtk_widget_set_halign(priv->grid, GTK_ALIGN_CENTER);
      gtk_widget_set_valign(priv->grid, GTK_ALIGN_CENTER);
      gtk_grid_set_row_spacing(GTK_GRID(priv->grid), 5);

      GtkWidget *label;
      label = gtk_label_new(_("Data in cache (Mb):"));
      gtk_grid_attach(GTK_GRID(priv->grid), label, 0, 0, 1, 1);

      priv->progress = gtk_progress_bar_new();
      gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(priv->progress), TRUE);
      gtk_grid_attach(GTK_GRID(priv->grid), priv->progress, 0, 1, 1, 1);

      label = gtk_label_new(_("Total cache size (Mb):"));
      gtk_grid_attach(GTK_GRID(priv->grid), label, 0, 2, 1, 1);

      priv->spin = gtk_spin_button_new_with_range(0, 128000, 50);
      refresh_gui(cache); //< Нужно выставить актуальные значения до установки сигнала "value-changed".
      g_signal_connect(priv->spin, "value-changed", G_CALLBACK(spin_value_changed), self);
      gtk_grid_attach(GTK_GRID(priv->grid), priv->spin, 0, 3, 1, 1);

      gtk_widget_show_all(GTK_WIDGET(priv->grid));
    }

    return GTK_WIDGET(priv->grid);
  }

  static void gp_smart_cache_prefable_interface_init(GpPrefableIface *iface)
  {
    iface->config_load = config_load;
    iface->config_save = config_save;
    iface->get_pref_widget = get_pref_widget;
  }
#endif

/*! \endcond */
