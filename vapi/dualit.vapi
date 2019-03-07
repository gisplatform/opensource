/*
 * dualit.vapi is a usefull common vapi-file.
 *
 * Copyright (C) 2016 Sergey Volkhin.
 *
 * dualit.vapi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dualit.vapi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with dualit.vapi. If not, see <http://www.gnu.org/licenses/>.
 *
*/

namespace Dualit
{
  namespace Version
  {
    /**
     * Мажорная версия ПО. Прописана в Version.cmake.
    */
    const int MAJOR;
    /**
     * Минорная версия ПО. Прописана в Version.cmake.
    */
    const int MINOR;
    /**
     * Патч-версия ПО. Прописана в Version.cmake.
    */
    const int PATCH;
  }

  /**
   * Гит-хэш репозитория. Формируется в Version.cmake.
  */
  const string GIT_HASH;
  /**
   * Строка с именем ветки в git. Формируется в Version.cmake.
  */
  const string GIT_BRANCH;
  /**
   * Информация о версии ПО и гит-хэше в виде строки. Формируется в Version.cmake.
  */
  const string INFO;

  /**
  * Хак, чтобы собирать программы, работающие под старыми версиями GLib,
  * требующими ручного вызова g_type_init.
  */
  [CCode (cname = "g_type_init")]
  public void legacy_type_init();

  namespace Gettext
  {
    /**
    * Текущий домен gettext, установленный с помощью -DGETTEXT_PACKAGE.
    */
    [CCode (cname = "GETTEXT_PACKAGE")]
    public const string PACKAGE;

    /**
    * Оригинальная функция dgettext (не GLib'овская, которая отключает переводы библиотек,
    * если не найдены переводы для дефолтного домена).
    */
    [CCode (cname = "dgettext")]
    public unowned string dgettext(string? domain, string msg);
  }
}

