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



namespace Gp
{
  /**
   * Объект позволяет сбрасывать вызовом метода "dump" дампы в файлы вида dumpXXXXXX,
   * где XXXXX -- порядковый номер файла.
   *
   * Файлы пишутся в каталог dir_path_DATA,
   * где путь dir_path указан пользователем при создании объекта,
   * а DATA -- постфикс, содержащий дату и время создания объекта.
   * Если указанный каталог не существует, то он будет создан автоматически.
   *
   * Чтобы не переполнить диск можно указать максимальное количество файлов
   * для объекта Dumper (аргумет "max_files_num" при создании объекта).
   * При достижении максимального количества файлов самые старые начинают удаляться
   * (т.е. работает механизм аналогичный поведению кольцевого буфера).
   */
  public class Dumper : Object
  {
    public const string FILE_NAME_FORMAT = "dump%06u";

    public File dir           { public get; construct; }
    public uint max_files_num { public get; construct; }
    public uint current_num   { public get; private set; default = 0; }

    /**
    * Создание объекта для записи дампов.
    * @param dir_path Путь к каталогу, куда писать файлы (к пути будет добавлен постфикс с датой и временем).
    * @param max_files_num Максимальное количество файлов, либо 0, если ограничение не требуется.
    */
    public Dumper(string dir_path, uint max_files_num = 0)
    {
      Object(dir: File.new_for_path(dir_path + datetime_postfix()), max_files_num:max_files_num);
    }

    /**
    * Функция записи дампа.
    * @param data Данные для сброса в файл.
    * @param cancellable Опциональный объект для отмены операции.
    * @return Количество записанных байт.
    */
    public size_t dump(uint8[] data, Cancellable? cancellable = null) throws Error
    {
      size_t rval = 0;

      try
      {
        dir.make_directory_with_parents(cancellable);
      }
      catch(Error e)
      {
        if((e is IOError.EXISTS) == false)
          throw e;
      }

      if(max_files_num != 0 && current_num >= max_files_num)
      {
        var file_to_remove = File.new_for_path(GLib.Path.build_filename(
          dir.get_path(), FILE_NAME_FORMAT.printf(current_num - max_files_num)));
        file_to_remove.delete(cancellable);
      }

      var new_file = File.new_for_path(GLib.Path.build_filename(
        dir.get_path(), FILE_NAME_FORMAT.printf(current_num)));

      new_file.create(FileCreateFlags.NONE, cancellable).write_all(data, out rval, cancellable);

      current_num++;

      return rval;
    }
  }
}

