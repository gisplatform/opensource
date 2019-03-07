// Источник: gtk3-demo.

#include <glib/gi18n.h>
#include <gtk/gtk.h>

static GtkWidget *window = NULL;
static GtkWidget *spinner_button = NULL;
static GtkWidget *spinner_sensitive = NULL;
static GtkWidget *spinner_unsensitive = NULL;

static void
on_play_clicked (GtkButton *button, gpointer user_data)
{
  if(gtk_bin_get_child(GTK_BIN(spinner_button)) != spinner_sensitive)
  {
    gtk_container_remove(GTK_CONTAINER(spinner_button), gtk_bin_get_child(GTK_BIN(spinner_button)));
    gtk_container_add(GTK_CONTAINER(spinner_button), spinner_sensitive);
  }

  gtk_spinner_start (GTK_SPINNER (spinner_sensitive));
  gtk_spinner_start (GTK_SPINNER (spinner_unsensitive));
}

static void
on_stop_clicked (GtkButton *button, gpointer user_data)
{
  gtk_spinner_stop (GTK_SPINNER (spinner_sensitive));
  gtk_spinner_stop (GTK_SPINNER (spinner_unsensitive));
}

int main(int argc, char **argv)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *spinner;

  {
    gchar *prog_bin_dir = g_path_get_dirname(argv[0]);
    gchar *prog_root_dir = g_build_filename(prog_bin_dir, "..", NULL);
    gchar *locale_path = g_build_filename(prog_root_dir, "share", "locale", NULL);

    bindtextdomain(PACKAGE, locale_path);

    #if defined(G_OS_WIN32)
      bind_textdomain_codeset(PACKAGE, "UTF-8");
    #endif

    textdomain(PACKAGE);

    g_free(prog_bin_dir);
    g_free(prog_root_dir);
    g_free(locale_path);
  }

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_main_quit), &window);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

  /* Sensitive */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  spinner = gtk_spinner_new ();

  spinner_button = gtk_button_new(); //< Test spinner in button and will add it in button later.
  gtk_box_pack_start(GTK_BOX (hbox), spinner_button, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(spinner_button),
    gtk_image_new_from_icon_name("task-past-due", GTK_ICON_SIZE_BUTTON));
  gtk_spinner_start(GTK_SPINNER(spinner));
  gtk_widget_show(spinner);

  gtk_container_add (GTK_CONTAINER (hbox), gtk_entry_new ());
  gtk_box_pack_start(GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  spinner_sensitive = spinner;

  /* Disabled */
  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  spinner = gtk_spinner_new ();
  gtk_container_add (GTK_CONTAINER (hbox), spinner);
  gtk_container_add (GTK_CONTAINER (hbox), gtk_entry_new ());
  gtk_container_add (GTK_CONTAINER (vbox), hbox);
  spinner_unsensitive = spinner;
  gtk_widget_set_sensitive (hbox, FALSE);

  button = gtk_button_new_with_label (_("Play"));
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (on_play_clicked), spinner);
  gtk_container_add (GTK_CONTAINER (vbox), button);

  button = gtk_button_new_with_label (_("Stop"));
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (on_stop_clicked), spinner);
  gtk_container_add (GTK_CONTAINER (vbox), button);

  /* Start by default to test for:
   * https://bugzilla.gnome.org/show_bug.cgi?id=598496 */
  //on_play_clicked (NULL, NULL);

  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

