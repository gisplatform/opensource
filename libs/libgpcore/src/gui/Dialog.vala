/*
 * GpDialog is a GUI library with set of dialogs.
 *
 * Copyright (C) 2015 Sergey Volkhin, Andrey Vodilov.
 *
 * GpDialog is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpDialog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpDialog. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using Gtk;


namespace Gp
{
  /**
  * Иициализация параметров, необходимых для работы остальных функций Dialog.vala.
  * Функцию необходимо вызывать в программе как можно раньше, к примеру сразу после Gtk.init().
  */
  public void dialog_customization_init()
  {
    Gtk.Settings.get_default( ).set("gtk-dialogs-use-header", false);
  }


  /**
  * Настраивает свойства окна диалога (убирает декорации, делает его модальным и т.п.).
  * @param dialog Диалог, который необходимо настроить.
  * @param widget_in_parent Любой виджет на окне, поверх которого необходимо показать диалог.
  */
  public void dialog_customize(Dialog dialog, Widget widget_in_parent)
  {
    dialog.set_decorated(false);
    dialog.set_modal(true);
    dialog.set_urgency_hint(true);


    Widget toplevel = widget_in_parent.get_toplevel();
    if(toplevel.is_toplevel() && (toplevel is Window))
    {
      dialog.set_transient_for((Window)toplevel);

      if(toplevel.get_sensitive())
      {
        toplevel.set_sensitive(false);
        dialog.unmap.connect(() => { toplevel.set_sensitive(true); });
      }
    }
  }


  /**
  * Создает и сразу показывает диалог с ошибкой.
  * Перед показом для него будет вызван метод настройки dialog_customize().
  * @param text Текст для диалога.
  * @param widget_in_parent Любой виджет на окне, поверх которого необходимо показать диалог.
  */
  public void dialog_fail(string text, Widget widget_in_parent)
  {
    var fail_dialog = new MessageDialog(null,
      0, MessageType.ERROR, ButtonsType.CLOSE, text);

    dialog_customize(fail_dialog, widget_in_parent);
    fail_dialog.run();
    fail_dialog.destroy();
  }


  /**
  * Создает и сразу показывает информационный диалог.
  * Перед показом для него будет вызван метод настройки dialog_customize().
  * @param text Текст для диалога.
  * @param widget_in_parent Любой виджет на окне, поверх которого необходимо показать диалог.
  */
  public void dialog_info(string text, Widget widget_in_parent)
  {
    var info_dialog = new MessageDialog(null,
      0, MessageType.INFO, ButtonsType.OK, text);

    dialog_customize(info_dialog, widget_in_parent);
    info_dialog.run();
    info_dialog.destroy();
  }


  /**
  * Создает и сразу показывает диалог с вопросом.
  * Перед показом для него будет вызван метод настройки dialog_customize().
  * @param text Текст с вопросом для диалога.
  * @param widget_in_parent Любой виджет на окне, поверх которого необходимо показать диалог.
  * @return TRUE, если была нажата кнопка Да, иначе -- FALSE.
  */
  public bool dialog_question(string text, Widget widget_in_parent)
  {
    var question_dialog = new MessageDialog(null,
      0, MessageType.QUESTION, ButtonsType.YES_NO, text);

    dialog_customize(question_dialog, widget_in_parent);

    var rval = question_dialog.run();

    question_dialog.destroy();

    return (rval == Gtk.ResponseType.YES);
  }
}

