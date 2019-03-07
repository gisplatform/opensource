/*
 * GpDumper is an object for data dumping (messages for example) in sequential files.
 *
 * Copyright (C) 2017 Sergey Volkhin.
 *
 * GpDumper is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpDumper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpDumper. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include <gp-core.h>

int main(int argc, char **argv)
{
  #if !GLIB_CHECK_VERSION(2,36,0)
    g_type_init();
  #endif

  static const int MAX_FILES_NUM = 3;
  static const int DUMPS_CALLS = 5;

  GError *err = NULL;
  GFile *dir_with_dumps = NULL;
  GFile *target_dir = g_file_new_for_path("/tmp/");

  { // На случай, если /tmp/ не доступен (к примеру, для винды) -->
    GFileInfo *target_dir_info = g_file_query_info(target_dir, G_FILE_ATTRIBUTE_STANDARD_TYPE, G_FILE_QUERY_INFO_NONE, NULL, NULL);

    if(target_dir_info == NULL ||
      (g_file_info_get_file_type(target_dir_info) != G_FILE_TYPE_DIRECTORY))
    {
      g_warning("Setting home as target_dir");
      g_clear_object(&target_dir);
      target_dir = g_file_new_for_path(g_get_home_dir());
    }

    g_clear_object(&target_dir_info);
  } // На случай, если /tmp/ не доступен (к примеру, для винды) <--

  {
    gchar *target_path_pattern = g_build_filename(g_file_get_path(target_dir), "DUMPER_TEST", NULL);

    GpDumper *dumper = gp_dumper_new(target_path_pattern , MAX_FILES_NUM);
    dir_with_dumps = g_object_ref(gp_dumper_get_dir(dumper));

    int i;
    for(i = 0; i < DUMPS_CALLS; i++)
    {
      gp_dumper_dump(dumper, (guint8*)&i, sizeof(i), NULL, &err); //< Пишем в дамп i.

      if(err != NULL)
        g_error("Failed to create dump #%d: %s", i, err->message);
    }

    g_clear_object(&dumper);
    g_free(target_path_pattern);
  }


  {
    const gchar *name;
    GDir *dir = g_dir_open(g_file_get_path(dir_with_dumps), 0, &err);
    GPtrArray *dumps = g_ptr_array_new_with_free_func(g_free);

    if(err != NULL)
      g_error("Failed to list target_dir: %s", err->message);

    while((name = g_dir_read_name(dir)) != NULL)
      g_ptr_array_add(dumps, g_strdup(name));

    gp_filenames_sort((gchar **)dumps->pdata, dumps->len, FALSE);

    int i;
    for(i = 0; i < dumps->len; i++)
    {
      name = g_ptr_array_index(dumps, i);

      gchar *path = g_build_filename(g_file_get_path(dir_with_dumps), name, NULL);
      GFile *file = g_file_new_for_path(path);
      GFileInputStream *fis = g_file_read(file, NULL, &err);

      if(err != NULL)
        g_error("Failed to File.read dump #%d '%s': %s", i, path, err->message);

      gsize bytes_read;
      int dump_content; //< Число i, выше записанное, считаем из дампа.
      g_input_stream_read_all(G_INPUT_STREAM(fis), &dump_content, sizeof(dump_content), &bytes_read, NULL, &err);

      if(err != NULL)
        g_error("Failed to read_all dump #%d: %s", i, err->message);

      g_print("> %s with content: %d\n", name, dump_content);

      g_assert_cmpint(dump_content, ==, (DUMPS_CALLS - MAX_FILES_NUM) + i);

      g_clear_object(&file);
      g_clear_object(&fis);
      g_free(path);
    }

    g_assert_cmpuint(dumps->len, ==, MAX_FILES_NUM);
    g_assert_cmpstr(g_ptr_array_index(dumps, 0), ==, "dump000002");
    g_assert_cmpstr(g_ptr_array_index(dumps, 1), ==, "dump000003");
    g_assert_cmpstr(g_ptr_array_index(dumps, 2), ==, "dump000004");

    g_ptr_array_unref(dumps);
    g_dir_close(dir);
  }

  g_message("Cleaning: removing '%s'", g_file_get_path(dir_with_dumps));
  gp_recursive_delete(dir_with_dumps, NULL, &err);

  if(err != NULL)
    g_error("Failed to delete dir_with_dumps: %s", err->message);

  g_clear_object(&target_dir);
  g_clear_object(&dir_with_dumps);

  return 0;
}

