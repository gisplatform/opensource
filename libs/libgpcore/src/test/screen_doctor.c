/*
 * GpScreenDoctor is an object preventing screen timeout.
 *
 * Copyright (C) 2016 Sergey Volkhin.
 *
 * GpScreenDoctor is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpScreenDoctor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpScreenDoctor. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#include <stdio.h>
#include <gtk/gtk.h>
#include <gp-core-gui.h>


static gboolean on_delete_event(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
  g_object_unref(G_OBJECT(user_data));
  g_message("Let's restore screensaver status and quit.");
  gtk_main_quit();
  return FALSE;
}

int main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  GtkWidget *main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(main_window), 300, 300);
  gtk_widget_show(main_window); //< Виджет должен быть смаплен до создания GpScreenDoctor.

  GpScreenDoctor *doctor = gp_screen_doctor_new(main_window);
  gchar *info = gp_screen_doctor_to_string(doctor);

  GtkWidget *label = gtk_label_new(info);
  gtk_container_add(GTK_CONTAINER(main_window), label);
  gtk_widget_show(label);

  g_signal_connect(G_OBJECT(main_window), "delete-event", G_CALLBACK(on_delete_event), doctor);

  gtk_main();

  g_free(info);

  return 0;
}

