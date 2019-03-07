#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gp-core.h>


static GpRumorista *rumorista;


void play(GtkButton *button, gpointer effect_id)
{
  gp_rumorista_play(rumorista, GPOINTER_TO_INT(effect_id));
}


int main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 5);

  GtkWidget *box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_container_add(GTK_CONTAINER(window), box);

  rumorista = gp_rumorista_new();

  GtkWidget *button = gtk_button_new_with_label("Ahtung!");
  g_signal_connect(button, "clicked", G_CALLBACK(play), GINT_TO_POINTER(G_PRIORITY_HIGH));
  gtk_container_add(GTK_CONTAINER(box), button);

  button = gtk_button_new_with_label("Dzin-dzin.");
  g_signal_connect(button, "clicked", G_CALLBACK(play), GINT_TO_POINTER(G_PRIORITY_DEFAULT));
  gtk_container_add(GTK_CONTAINER(box), button);

  button = gtk_button_new_with_label("Click.");
  g_signal_connect(button, "clicked", G_CALLBACK(play), GINT_TO_POINTER(G_PRIORITY_LOW));
  gtk_container_add(GTK_CONTAINER(box), button);

  gtk_widget_show_all(window);

  gtk_main();

  g_clear_object(&rumorista);

  return 0;
}

