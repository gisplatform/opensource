/*!
 * \author Sergey Volkhin
 * \date 18.01.2017
 * \brief Исходный файл с примером ActivityButton.
 *
*/


#include <gtk/gtk.h>
#include <gp-core.h>
#include <gp-core-gui.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if GTK_CHECK_VERSION(3, 12, 0)
static guint cur_actions_number = 0;

static gint opt_step_time = 300;
static gint opt_steps = 10;
static GOptionEntry entries[] =
{
  { "opt_steps", 'n', 0, G_OPTION_ARG_INT, &opt_steps, "Number of fraction steps of action", NULL },
  { "step-time", 't', 0, G_OPTION_ARG_INT, &opt_step_time, "Duration of step in milliseconds", NULL },
  { NULL }
};


typedef struct _Action Action;
struct _Action
{
  guint id;
  gchar name[64];
  guint step;
  GpActivityButton *activity_button;
  GCancellable *cancellable;
};


gboolean timeout_func(gpointer user_data)
{
  Action *a = user_data;

  if(a->step == opt_steps)
  {
    gp_activity_button_done(a->activity_button, a->name);
    goto destroy;
  }

  if(a->cancellable && g_cancellable_is_cancelled(a->cancellable))
  {
    g_message("'%s' cancelled", a->name);
    goto destroy;
  }

  gp_activity_button_set_fraction(a->activity_button, a->name, (gdouble)a->step / opt_steps, a->cancellable);
  a->step++;

  return TRUE;

destroy:
  g_clear_object(&a->activity_button);
  g_clear_object(&a->cancellable);
  g_free(a);

  return FALSE;
}


void add_clicked(GtkButton *button, GObject *activity_button)
{
  Action *a = g_new0(Action, 1);

  a->id = cur_actions_number;
  g_snprintf(a->name, sizeof(a->name), "Action #%u", a->id);
  a->activity_button = g_object_ref(activity_button);

  // В каждой третьей активности добавим cancellable.
  if(a->id % 3 == 0)
    a->cancellable = g_cancellable_new();

  if(timeout_func(a))
    g_timeout_add(opt_step_time, timeout_func, a);

  cur_actions_number++;
}
#endif


int main(int argc, char **argv)
{
#if GTK_CHECK_VERSION(3, 12, 0)
  gtk_init(&argc, &argv);

  { // Переводы -->
    gchar *prog_bin_dir = g_path_get_dirname(argv[0]);
    gchar *locale_path = g_build_filename(prog_bin_dir, "..", "share", "locale", NULL );
      bindtextdomain(PACKAGE, locale_path);
    g_free(prog_bin_dir);
    g_free(locale_path);
  } // Переводы <--

  GError *error = NULL;
  GOptionContext *context;
  context = g_option_context_new("- test ActivityButton");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group(context, gtk_get_option_group (TRUE));
  if(!g_option_context_parse(context, &argc, &argv, &error))
  {
    g_print("option parsing failed: %s\n", error->message);
    exit(1);
  }

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(window), 320, 240);

  GtkWidget *grid = gtk_grid_new();
  gtk_container_add(GTK_CONTAINER(window), grid);

  GtkWidget *activity_button = GTK_WIDGET(gp_activity_button_new());
  gtk_widget_set_hexpand(activity_button, TRUE);
  gtk_widget_set_vexpand(activity_button, TRUE);
  gtk_grid_attach_next_to(GTK_GRID(grid), activity_button, NULL, GTK_POS_RIGHT, 1, 1);

  GtkWidget *add_button = gtk_button_new_from_icon_name("list-add", GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_hexpand(add_button, TRUE);
  gtk_widget_set_vexpand(add_button, TRUE);
  g_signal_connect(add_button, "clicked", G_CALLBACK(add_clicked), activity_button);
  gtk_grid_attach_next_to(GTK_GRID(grid), add_button, activity_button, GTK_POS_RIGHT, 1, 1);

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_widget_show_all(window);
  gtk_main();

  return 0;
#else
  g_error("Too old GTK version, we need at least 3.12!");
#endif
}

