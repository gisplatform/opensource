/*
 * GpBookmark is a bookmarks storage library.
 *
 * Copyright (C) 2015 Alexey Pankratov, Sergey Volkhin.
 *
 * GpBookmark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpBookmark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpBookmark. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using GLib;

namespace Gp
{
  #if GLIB_2_40
  /**
   * Класс, реализующий сохранение заметок и недавних чего-нибудь (например адресов серверов)
   * в указанный файл в указанной папке.
   */
  public class Bookmark
  {
    KeyFile file;
    string config_path;

    /**
     * Конструктор.
     *
     * @param config_path Папка, в которой будет храниться сохранённый файл.
     * @param filename Имя файла, в который будут сохранены записаные в объект данные.
     */
    public Bookmark(string config_path, string filename) throws FileError
    {
      if(!FileUtils.test(config_path, FileTest.EXISTS))
      {
        DirUtils.create_with_parents(config_path, 511);
      }
      this.config_path = Path.build_filename(config_path,
                                        filename,
                                        null );
      try
      {
        file = new KeyFile();
        if(FileUtils.test(this.config_path, FileTest.EXISTS))
        {
          file.load_from_file(this.config_path, KeyFileFlags.NONE);
        }
      }
      catch(KeyFileError error)
      {
        warning("KeyFile error: %s \n", error.message);
      }
    }

    /**
     * Функция для получения списка недавних.
     */
    public string[] get_recent()
    {
      string[] recent;
      try
      {
        recent = file.get_string_list("Bookmark", "recent");
      }
      catch(KeyFileError error)
      {
        return {};
      }
      if(recent==null)
      {
        return {};
      }
      return recent;
    }

    /**
     * Функция для получения списка сохранённых.
     */
    public string[] get_saved()
    {
      string[] saved;
      try
      {
        saved = file.get_string_list("Bookmark", "saved");
      }
      catch(KeyFileError error)
      {
        return {};
      }
      return saved;
    }

    /**
     * Функция для получения списка описаний к сохранённым.
     */
    public string[] get_saved_description()
    {
      string[] saved_description;
      try
      {
        saved_description = file.get_string_list("Bookmark", "saved_description");
      }
      catch(KeyFileError error)
      {
        return {};
      }
      return saved_description;
    }

    /**
     * Функция для добавления в список недавних нового значения.
     * В списке недавних хранится не более 10 последних значений.
     *
     * @param address Значение, которое будет добавлено в список.
     */
    public void add_recent(string address) throws FileError
    {
      string[] recent;
      try
      {
        recent = file.get_string_list("Bookmark", "recent");
        if(!(address in recent) && recent.length < 10)
        {
          recent += address;
        }
        else if(!(address in recent) && recent.length >= 10)
        {
          string[] tmp = {};
          tmp += address;
          for(int i = 0; i < recent.length - 1; i++)
          {
            tmp += recent[i];
          }
          recent = tmp;
        }
      }
      catch(KeyFileError error)
      {
        recent = {address};
      }
      file.set_string_list("Bookmark", "recent", recent);
      file.save_to_file(config_path);
    }

    /**
     * Функция для добавления в список сохранённых нового значения.
     *
     * @param address Значение, которое будет добавлено в список.
     */
    public void add_saved(string address) throws FileError
    {
      string[] saved;
      string[] saved_description = {};
      try
      {
        saved = file.get_string_list("Bookmark", "saved");
        if(!(address in saved))
        {
          saved += address;
          saved_description = file.get_string_list("Bookmark", "saved_description");
          saved_description += "";
        }
      }
      catch(KeyFileError error)
      {
        saved = {address};
        saved_description = {""};
      }
      file.set_string_list("Bookmark", "saved", saved);
      file.set_string_list("Bookmark", "saved_description", saved_description);
      file.save_to_file(config_path);
    }

    /**
     * Функция для удаления из списка сохранённых значения.
     *
     * @param address Значение, которое будет удалено из списка.
     */
    public void remove_saved(string address) throws FileError
    {
      string[] saved;
      string[] saved_description;
      try
      {
        saved = file.get_string_list("Bookmark", "saved");
        saved_description = file.get_string_list("Bookmark", "saved_description");
        if(address in saved)
        {
          string[] tmp_s = {};
          string[] tmp_sd = {};
          for(int i = 0; i < saved.length; i++)
          {
            if(saved[i] != address)
            {
              tmp_s += saved[i];
              tmp_sd += saved_description[i];
            }
          }
          saved = tmp_s;
          saved_description = tmp_sd;
        }
      }
      catch(KeyFileError error)
      {
        saved = {};
        saved_description = {};
      }
      file.set_string_list("Bookmark", "saved", saved);
      file.set_string_list("Bookmark", "saved_description", saved_description);
      file.save_to_file(config_path);
    }

    /**
     * Функция для установки описания к значению в списке сохранённых.
     *
     * @param address Значение, к которому будет установлено описание.
     * @param description Описание.
     */
    public void set_saved_description(string address, string description) throws FileError
    {
      string[] saved;
      string[] saved_description;
      try
      {
        saved = file.get_string_list("Bookmark", "saved");
        saved_description = file.get_string_list("Bookmark", "saved_description");
        for(int i = 0; i < saved.length; i++)
        {
          if(saved[i] == address)
          {
            saved_description[i] = description;
            break;
          }
        }
        file.set_string_list("Bookmark", "saved_description", saved_description);
        file.save_to_file(config_path);
      }
      catch(KeyFileError error)
      {
        warning("KeyFile error: %s \n", error.message);
      }
    }
  }
  #endif
}

