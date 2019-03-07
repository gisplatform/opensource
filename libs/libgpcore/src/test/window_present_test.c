#include <glib/gi18n.h>
#include <gtk/gtk.h>


static GtkWidget *window = NULL;
static GtkWidget *label = NULL;
static int tick = 0;
static const int TICKS = 5;


static void set_number(gint num)
{
  gchar *markup = g_markup_printf_escaped("<span font_size=\"xx-large\">%d</span>", num);
    gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);
}


static void set_text(const char *text)
{
  gchar *markup = g_markup_printf_escaped("<span font_size=\"xx-large\">%s</span>", text);
    gtk_label_set_markup(GTK_LABEL(label), markup);
  g_free(markup);
}


static gboolean on_timer(gpointer user_data)
{
  if(++tick != TICKS)
  {
    set_number(TICKS - tick);
    return TRUE;
  }
  else
  {
    set_text("Bump!");
    gtk_window_present(GTK_WINDOW(window));
    return FALSE;
  }
}


int main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

  label = gtk_label_new(NULL);
  gtk_container_add(GTK_CONTAINER(window), label);
  gtk_container_set_border_width(GTK_CONTAINER(window), 5);

  set_number(TICKS);
  g_timeout_add(1000, on_timer, NULL);

  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

