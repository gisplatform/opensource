/*
 * GpChunk is a data storage with built-in header and optional compression.
 *
 * Copyright (C) 2018 Sergey Volkhin.
 *
 * GpChunk is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpChunk is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpChunk. If not, see <http://www.gnu.org/licenses/>.
 *
*/


#include <gp-core.h>

static const gsize N_BYTES = 300;
static const gsize BYTES_PER_LINE = 20;

#if !GLIB_CHECK_VERSION(2, 38, 0)
  #warning Glib version is so old, that we will define g_assert_true/false ourself

  #define g_assert_true(expr) G_STMT_START \
  { \
    if(G_LIKELY(expr)) \
      ; \
    else \
      g_assertion_message(G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, "'" #expr "' should be TRUE");\
  } G_STMT_END

  #define g_assert_false(expr) G_STMT_START \
  { \
    if(G_LIKELY(!(expr))) \
      ; \
    else \
      g_assertion_message (G_LOG_DOMAIN, __FILE__, __LINE__, G_STRFUNC, "'" #expr "' should be FALSE"); \
  } G_STMT_END

#endif

static void catch(GError *e)
{
  if(e != NULL)
    g_error("Error: %s", e->message);
}

int main(int argc, char **argv)
{
  #if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
  #endif

  guint i;
  GError *err = NULL;
  GpChunk *chunk = NULL;

  { // Заполнение чанка рандомными данными -->
    GTimeVal time;
    g_get_current_time( &time );

    #if 0
      GRand *rand = g_rand_new_with_seed(1); //< Вариант с повторением данных от запуска к запуску.
    #else
      GRand *rand = g_rand_new_with_seed(time.tv_sec + time.tv_usec);
    #endif

    guint8 *user_buffer = g_malloc(N_BYTES);

    for(i = 0; i < N_BYTES; i++)
      #if 0
        user_buffer[i] = g_rand_int_range(rand, 0, G_MAXUINT8);
      #else
        user_buffer[i] = g_rand_int_range(rand, 0, 16); //< Вариант, который хорошечно сжимается.
      #endif

    chunk = gp_chunk_new_with_data(user_buffer, N_BYTES);

    g_rand_free(rand);
  } // Заполнение чанка рандомными данными <--

  // Оригинал -->
    g_print("---- ORIGINAL CHUNK:\n");
    gp_chunk_hexdump(chunk, BYTES_PER_LINE);

    gchar *orig_md5sum = gp_chunk_get_user_data_md5sum(chunk);

    g_assert_cmpuint(gp_chunk_get_user_data_size_from_header(chunk), ==, N_BYTES);
    g_assert_false(gp_chunk_get_compress_flag_from_header(chunk));
  // Оригинал <--

  // Сжатие -->
    gp_chunk_deflate(chunk, TRUE, &err);
    catch(err);

    g_print("---- COMPRESSED CHUNK:\n");
    gp_chunk_hexdump(chunk, BYTES_PER_LINE);

    gchar *compressed_md5sum = gp_chunk_get_user_data_md5sum(chunk);

    g_assert_cmpstr(compressed_md5sum, !=, orig_md5sum);
    g_assert_true(gp_chunk_get_compress_flag_from_header(chunk));
  // Сжатие <--

  // Попытка повторного сжатия -->
    gp_chunk_deflate(chunk, TRUE, &err);
    catch(err);

    gchar *double_compressed_md5sum = gp_chunk_get_user_data_md5sum(chunk);

    g_assert_cmpstr(double_compressed_md5sum, ==, compressed_md5sum);
    g_assert_true(gp_chunk_get_compress_flag_from_header(chunk));
  // Попытка повторного сжатия <--

  // Распаковка -->
    gp_chunk_deflate(chunk, FALSE, &err);
    catch(err);

    gchar *decompressed_md5sum = gp_chunk_get_user_data_md5sum(chunk);

    g_print("---- DECOMPRESSED CHUNK:\n");
    gp_chunk_hexdump(chunk, BYTES_PER_LINE);

    g_assert_cmpstr(decompressed_md5sum, ==, orig_md5sum);
    g_assert_cmpuint(gp_chunk_get_user_data_size_from_header(chunk), ==, N_BYTES);
    g_assert_false(gp_chunk_get_compress_flag_from_header(chunk));
  // Распаковка <--

  // Попытка повторной распаковки -->
    gp_chunk_deflate(chunk, FALSE, &err);
    catch(err);

    gchar *double_decompressed_md5sum = gp_chunk_get_user_data_md5sum(chunk);

    g_assert_cmpstr(double_decompressed_md5sum, ==, orig_md5sum);
    g_assert_false(gp_chunk_get_compress_flag_from_header(chunk));
  // Попытка повторной распаковки <--

  g_free(orig_md5sum);
  g_free(compressed_md5sum);
  g_free(double_compressed_md5sum);
  g_free(decompressed_md5sum);
  g_free(double_decompressed_md5sum);

  gp_chunk_free(chunk);

  return 0;
}

