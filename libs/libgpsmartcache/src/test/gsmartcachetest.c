/*! \file gsmartcachetest.c
 *
 *  Created on: 27.09.2011
 *      Author: zankov.p
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "gp-smartcache.h"

typedef struct _rec
{
   guint group;
   guint index;

   gpointer data;
   guint size;
} rec;

#define COUNT 100000 // количество добавляемых индексов
#define CACHE_SIZE ( COUNT / 10 ) // количество хранимых в кэше индексов
#define GROUP_SIZE ( CACHE_SIZE / 10 ) // количество индексов в группе
#define DATA_SIZE 1024 // размер индекса
int main( int argc, char **argv )
{
   GpSmartCache *cache;
   GTimeVal time;
   GTimer *timer;
   rec *data;
   gint n, g;

   #if !GLIB_CHECK_VERSION(2,36,0)
     g_type_init();
   #endif

   cache = gp_smart_cache_new ();
   gp_smart_cache_set_size (cache, CACHE_SIZE * DATA_SIZE);

   g_get_current_time( &time );
   srand( time.tv_sec + time.tv_usec );
   timer = g_timer_new();

   /*
    * Формируем набор данных
    */

   g = 0;
   data = g_malloc0( COUNT * sizeof(rec) );

   for ( n = 0; n < COUNT; n++ )
   {
      static guint group = 0;

      if ( ++g == GROUP_SIZE)
      {
         group = gp_smart_cache_reg_group (cache);
         g = 0;
      }

      data[n].group = group;
      data[n].index = rand() * 100.0 / RAND_MAX;

      data[n].data = g_malloc( DATA_SIZE );
      data[n].size = DATA_SIZE;

      memset( data[n].data, data[n].index, DATA_SIZE );
   }

   /*
    * Добавляем элементы
    */

   g_timer_start( timer );

   for ( n = 0; n < COUNT; n++ )
      gp_smart_cache_set (cache, data[n].group, data[n].index, data[n].data, data[n].size);

   printf( "%i elements added in %.3f ms\n", COUNT, g_timer_elapsed( timer, NULL ) * 1000 );

   /*
    * Считываем последние добавленные элементы
    */

   g_timer_start( timer );

   for ( n = COUNT - ( CACHE_SIZE - GP_SMART_CACHE_FREE_K + 1 ); n < COUNT; n++ )
      g_assert(gp_smart_cache_get (cache, data[n].group, data[n].index, NULL, NULL, 0) );

   printf( "%zi elements found in %.3f ms\n", CACHE_SIZE - GP_SMART_CACHE_FREE_K + 1, g_timer_elapsed( timer, NULL ) * 1000 );

   /*
    * Проверим содержимое последних добавленных элементов
    */

   for ( n = COUNT - 1; n >= COUNT - ( CACHE_SIZE - GP_SMART_CACHE_FREE_K + 1 ); n-- )
   {
      gboolean has_prev = FALSE;
      guchar buff[DATA_SIZE];
      guint data_size;
      gint m;

      for ( m = n + 1; m < COUNT; m++ )
         if (data[m].group == data[n].group && data[m].index == data[n].index)
            has_prev = TRUE;

      if (has_prev)
         continue;

      g_assert(gp_smart_cache_get (cache, data[n].group, data[n].index, &data_size, buff, DATA_SIZE) );
      g_assert( data_size == DATA_SIZE );
      g_assert( memcmp( buff, data[n].data, DATA_SIZE ) == 0 );
   }

   /*
    * Очистка группы
    */

   printf( "Initial free:       %9zu bytes\n", gp_smart_cache_get_free (cache) );
   gp_smart_cache_clean (cache, data[COUNT - 1].group);
   printf( "Group removed free: %9zu bytes\n", gp_smart_cache_get_free (cache) );

   gp_smart_cache_unreg_group (cache, data[COUNT - 1].group);

   // Проверим, что данные группы стерлись
   for ( n = 0; n < COUNT; n++ )
      if (data[n].group == data[COUNT - 1].group)
         g_assert( !gp_smart_cache_get (cache, data[n].group, data[n].index, NULL, NULL, 0) );

   // Проверим, что остальные данные не тронуты
   for ( n = COUNT - 1; n >= COUNT - ( CACHE_SIZE - GP_SMART_CACHE_FREE_K + 1 ); n-- )
      if (data[n].group != data[COUNT - 1].group)
      {
         gboolean has_prev = FALSE;
         guchar buff[DATA_SIZE];
         guint data_size;
         gint m;

         for ( m = n + 1; m < COUNT; m++ )
            if (data[m].group == data[n].group && data[m].index == data[n].index)
               has_prev = TRUE;

         if (has_prev)
            continue;

         g_assert(gp_smart_cache_get (cache, data[n].group, data[n].index, &data_size, buff, DATA_SIZE) );
         g_assert( data_size == DATA_SIZE );
         g_assert( memcmp( buff, data[n].data, DATA_SIZE ) == 0 );
      }

   /*
    * Изменение размера кэша
    */

   printf( "Initial size: %9zu bytes, free: %9zu bytes\n", gp_smart_cache_get_size (cache),
           gp_smart_cache_get_free (cache) );
   gp_smart_cache_set_size (cache, CACHE_SIZE * DATA_SIZE / 2);
   printf( "New size:     %9zu bytes, free: %9zu bytes\n", gp_smart_cache_get_size (cache),
           gp_smart_cache_get_free (cache) );

   g_free( data );

   #if 1
     g_object_unref( cache );
   #else
    GThread *thread = g_thread_new("Cache_unref_thread", (GThreadFunc)g_object_unref, cache);

    printf("Unreffing GpSmartCache object in separate thread.\n");

    g_thread_join(thread);

    printf("GpSmartCache object unreffed.\n");
   #endif

   return 0;
}
