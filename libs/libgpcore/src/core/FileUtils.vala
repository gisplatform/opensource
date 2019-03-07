/*
 * GpFileUtils is a file utils library.
 *
 * Copyright (C) 2016 Sergey Volkhin, Andrey Vodilov.
 *
 * GpFileUtils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpFileUtils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpFileUtils. If not, see <http://www.gnu.org/licenses/>.
 *
*/

namespace Gp
{
  /*
  * Возвращает строку-постфикс вида "_текущая_дата_текущее_время" для использвания в именах файлов.
  */
  public string datetime_postfix()
  {
    return (new DateTime.now_local()).format("_%Y%m%d_%H%M%S");
  }

  /**
   * Сортировка массива строк с помощью g_utf8_collate_key_for_filename.
   *
   * Строки могут быть выделены статически.
   * Элементы массива равные NULL попадают в конец отсортированного списка.
   * @param names Массив для сортировки.
   * @param debug Выводить отладочные сообщения (с помощью g_message).
  */
  public void filenames_sort(string[] names, bool debug = false)
  {
    var htable = new HashTable<string, GenericArray<string>>(str_hash, str_equal);
    var keys = new Array<string>();

    if(unlikely(debug)) message("* filenames_sort * original names and keys:");

    for(int i = 0; i < names.length; i++)
    {
      if(names[i] == null)
      {
        if(unlikely(debug)) message("%s at [%p] has no key", names[i], names[i]);

        continue;
      }

      var key = names[i].collate_key_for_filename();
      var elem = htable.lookup(key); // Элемент в hash-таблице -- массив одинаковых строк.

      if(unlikely(debug)) message("%s at [%p] has key\t%s", names[i], names[i], key);

      if(elem == null)
      {
        keys.append_val(key);
        elem = new GenericArray<string>();
        htable.insert((owned)key, elem);
      }

      elem.add((owned)names[i]);
    }

    keys.sort((pa, pb) =>
    {
      char *a = *(char**)pa;
      char *b = *(char**)pb;
      return strcmp((string)a, (string)b);
    });

    if(unlikely(debug)) message("* filenames_sort * sorted names:");

    int names_i = 0;
    for(int keys_i = 0; keys_i < keys.length; keys_i++)
    {
      var elem = htable.lookup(keys.index(keys_i));
      return_if_fail(elem != null);

      for(int elem_i = 0; elem_i < elem.length; elem_i++)
      {
        return_if_fail(names_i < names.length);
        names[names_i] = (owned)elem.data[elem_i];

        names_i++;
      }
    }

    if(unlikely(debug)) message("* filenames_sort * done");
  }

  /**
   * Рекурсивно копирует директорию.
   * @param src Директория-источник.
   * @param dest Путь для копии.
   * @param flags Флаги.
   * @param cancellable Объект для отмены.
   * @return В случае успеха -- true, в случае ошибки -- false.
   */
  public bool recursive_copy(GLib.File src, GLib.File dest, GLib.FileCopyFlags flags = GLib.FileCopyFlags.NONE, GLib.Cancellable? cancellable = null) throws GLib.Error
  {
    GLib.FileType src_type = src.query_file_type(GLib.FileQueryInfoFlags.NONE, cancellable);

    if(src_type == GLib.FileType.DIRECTORY)
    {
      dest.make_directory(cancellable);
      src.copy_attributes(dest, flags, cancellable);

      string src_path = src.get_path();
      string dest_path = dest.get_path();

      GLib.FileEnumerator enumerator = src.enumerate_children(GLib.FileAttribute.STANDARD_NAME, GLib.FileQueryInfoFlags.NONE, cancellable);
      for(GLib.FileInfo? info = enumerator.next_file(cancellable); info != null ; info = enumerator.next_file(cancellable))
      {
        recursive_copy(
          GLib.File.new_for_path(GLib.Path.build_filename(src_path, info.get_name())),
          GLib.File.new_for_path(GLib.Path.build_filename(dest_path, info.get_name())),
          flags,
          cancellable);
      }
    }
    else
      src.copy(dest, flags, cancellable);

    return true;
  }

  /**
   * Рекурсивно удаляет директорию и ее содержимое.
   * @param target Директория/файл для удаления.
   * @param cancellable Объект для отмены.
   * @return В случае успеха -- true, в случае ошибки -- false.
   */
  public bool recursive_delete(GLib.File target, GLib.Cancellable? cancellable = null) throws GLib.Error
  {
    GLib.FileType target_type = target.query_file_type(GLib.FileQueryInfoFlags.NOFOLLOW_SYMLINKS, cancellable);

    if(target_type == GLib.FileType.DIRECTORY)
    {
      GLib.FileEnumerator enumerator = target.enumerate_children(
        GLib.FileAttribute.STANDARD_NAME,
        GLib.FileQueryInfoFlags.NOFOLLOW_SYMLINKS,
        cancellable);

      for(GLib.FileInfo? info = enumerator.next_file(cancellable); info != null; info = enumerator.next_file(cancellable))
        recursive_delete(
          GLib.File.new_for_path(GLib.Path.build_filename(target.get_path(), info.get_name())),
          cancellable);
    }

    target.delete(cancellable);

    return true;
  }

  /**
   * Возвращает имя файла без расширения.
   * @param basename имя файла с раширением без полного пути (например: file.txt).
   * @return В случае успеха имя файла без расширения, в случае ошибки - входную строку.
   */
  public string ? get_base( string basename)
  {
    if (basename.length == 0) return null;
    int index = basename.last_index_of_char ('.');
    return (index == -1) ? basename : basename[0 : index];
  }

    /**
   * Возвращает расширение файла без точки.
   * @param basename имя файла с раширением без полного пути (например: file.txt).
   * @return В случае успеха расширение файла без точки, в случае ошибки - null.
   */
  public string ? get_extension( string basename)
  {
    if (basename.length == 0) return null;
    int index = basename.last_index_of_char ('.');
    return (index == -1) ? null : basename[index + 1 : basename.length];
  }

  /**
  * Производит компактную (без выравниваний) сериализацию данных в GVariantе/массиве GVariantов.
  * Т.е. массив, состоящий из объектов типов VariantType.BYTE, VariantType.UINT32 и
  * VariantType.BYTE, будет гарантированно сериализован в 6 байт.
  */
  public Bytes variant_compact_serialise(Variant[] input_array)
  {
    var output_array = new ByteArray();

    foreach(unowned Variant v in input_array)
      output_array.append(v.get_data_as_bytes().get_data());

    return ByteArray.free_to_bytes(output_array);
  }
}

