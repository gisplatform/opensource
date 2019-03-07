
#include <gtk/gtk.h>
#include <gp-core.h>
#include <gp-core-gui.h>

void bitton_clicked_cb(GtkButton *button, gpointer data)
{
  GpPaletteBox *plt_box = (GpPaletteBox *)data;
  GtkWidget *dialog = gtk_dialog_new();
  GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_container_add(GTK_CONTAINER(content_area), GTK_WIDGET(plt_box));
  gtk_widget_show_all(GTK_WIDGET(dialog));

  gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);
}

int main(int argc, char **argv)
{
  gtk_init(&argc, &argv);

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  g_signal_connect (window, "destroy",
                    G_CALLBACK (gtk_main_quit), &window);


  GtkWidget *vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 5);
  gtk_container_add(GTK_CONTAINER(window), vbox);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);

  GpPalette plt_advanced;
  GpPaletteBox *plt_box = gp_palette_box_new(&plt_advanced, GP_PALETTE_TYPE_ADVANCED);
  gp_palette_box_set_widget_name(plt_box, "Bthymetry palette");
  GtkWidget *button = gtk_button_new_with_label(gp_palette_box_get_widget_name(plt_box));
  g_signal_connect (button, "clicked", G_CALLBACK (bitton_clicked_cb), plt_box);
  gtk_box_pack_start(GTK_BOX (vbox), button, TRUE, TRUE, 0);

  GpPalette plt_acoustic;
  plt_box = gp_palette_box_new(&plt_acoustic, GP_PALETTE_TYPE_SIMPLE);
  gp_palette_box_set_widget_name(plt_box, "Acoustic palette");
  button = gtk_button_new_with_label(gp_palette_box_get_widget_name(plt_box));
  g_signal_connect (button, "clicked", G_CALLBACK (bitton_clicked_cb), plt_box);
  gtk_box_pack_start(GTK_BOX (vbox), button, TRUE, TRUE, 0);

  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

