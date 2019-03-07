/*!
 * \author Sergey Volkhin
 * \date 06.01.2017
 * \brief Исходный файл с примером MenuTree.
 *
*/


#include <gtk/gtk.h>
#include <gp-core.h>
#include <gp-core-gui.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static gboolean show_icons = FALSE;
static gboolean show_detailed = TRUE;
static gboolean show_two_menus = TRUE;
static gboolean show_root_texts = TRUE;
static gboolean quit_on_root_up = TRUE;
static GOptionEntry entries[] =
{
  { "show-icons", 'i', 0, G_OPTION_ARG_NONE, &show_icons, "Show root menu with IconView", NULL },
  { "dont-show-detailed", 'd', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE, &show_detailed, "Don't use detailed_column", NULL },
  { "show-only-one-menu", '1', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE, &show_two_menus, "Show only one MenuTree instead of two", NULL },
  { "dont-set-root-texts", 'r', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE, &show_root_texts, "Don't set root texts", NULL },
  { "dont-quit-on-rootup", 'q', G_OPTION_FLAG_REVERSE, G_OPTION_ARG_NONE, &quit_on_root_up, "Don't quit on root up button", NULL },
  { NULL }
};

enum
{
  COL_WIDGET,
  COL_DETAILED,
  COL_BRIEF,
  COL_ICON,
  COLS_NUM
};


int main(int argc, char **argv)
{
#if GTK_CHECK_VERSION(3, 10, 0)
  gtk_init(&argc, &argv);

  GError *error = NULL;
  GOptionContext *context;
  context = g_option_context_new("- test MenuTree");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_add_group(context, gtk_get_option_group (TRUE));
  if(!g_option_context_parse(context, &argc, &argv, &error))
  {
    g_print("option parsing failed: %s\n", error->message);
    exit(1);
  }

  GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  //gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);

  GtkTreeStore *tree_store = gtk_tree_store_new(COLS_NUM,
    GTK_TYPE_WIDGET, G_TYPE_STRING, G_TYPE_STRING, GDK_TYPE_PIXBUF);

  GpMenuTree *menu1 = gp_menu_tree_new(GTK_TREE_MODEL(tree_store),
    COL_WIDGET, COL_BRIEF,
    show_detailed ? COL_DETAILED : -1,
    show_icons ? COL_ICON : -1);
  if(show_root_texts)
    gp_menu_tree_set_root_texts(menu1, "Menu name", "Close", "Menu descritpion", "This will close all");

  GtkWidget *paned = NULL;
  if(show_two_menus)
  {
    paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(paned));
    gtk_paned_add1(GTK_PANED(paned), GTK_WIDGET(menu1));
  }
  else
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(menu1));

  GtkWidget *barista;
  GtkTreeIter iter, subiter, subsubiter;
  {
    gtk_tree_store_append(tree_store, &iter, NULL);

    barista = GTK_WIDGET(gp_barista_new());
    gtk_container_add(GTK_CONTAINER(gp_barista_get_scrolled(GP_BARISTA(barista))),
      gtk_label_new("Widget from menu (no bar)"));

    gtk_tree_store_set(tree_store, &iter, COL_WIDGET, barista,
      COL_DETAILED, "Item with widget", COL_BRIEF, "Some widget",
      COL_ICON, gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), "document-print", 20, 0, NULL), -1);
    gtk_tree_store_append(tree_store, &iter, NULL);
    gtk_tree_store_set(tree_store, &iter,
      COL_DETAILED, "Item with submenu", COL_BRIEF, "Submenu",
      COL_ICON, gtk_icon_theme_load_icon(gtk_icon_theme_get_default(), "go-down", 20, 0, NULL), -1);

    gtk_tree_store_append(tree_store, &subiter, &iter);
    gtk_tree_store_set(tree_store, &subiter, COL_DETAILED, "Item with subsubmenu", COL_BRIEF, "Subsubmenu", -1);

    gtk_tree_store_append(tree_store, &subsubiter, &subiter);
    gtk_tree_store_set(tree_store, &subsubiter, COL_DETAILED, "Item with ssssmenu", COL_BRIEF, "Sssmenu", -1);

    barista = GTK_WIDGET(gp_barista_new());
    gtk_container_add(GTK_CONTAINER(gp_barista_get_scrolled(GP_BARISTA(barista))),
      gtk_label_new("Widget from sssmenu (with bar)"));
    gtk_toolbar_insert(gp_barista_get_toolbar(GP_BARISTA(barista)),
      gtk_tool_button_new(NULL, "Apply :)"), 0);

    gtk_tree_store_append(tree_store, &iter, &subsubiter);
    gtk_tree_store_set(tree_store, &iter, COL_WIDGET, barista, COL_DETAILED, "Item with widget", COL_BRIEF, "In widget", -1);
  }

  GpMenuTree *menu2 = NULL;
  if(show_two_menus)
  {
    menu2 = gp_menu_tree_new(GTK_TREE_MODEL(tree_store),
      COL_WIDGET, COL_BRIEF,
      show_detailed ? COL_DETAILED : -1,
      show_icons ? COL_ICON : -1);
    gtk_paned_add2(GTK_PANED(paned), GTK_WIDGET(menu2));
    if(show_root_texts)
      gp_menu_tree_set_root_texts(menu2, "Menu name", "Close", "Menu descritpion", "This will close all");
  }


  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  if(quit_on_root_up)
  {
    g_signal_connect(G_OBJECT(menu1), "root-up-clicked", G_CALLBACK(gtk_main_quit), NULL);

    if(show_two_menus)
      g_signal_connect(G_OBJECT(menu2), "root-up-clicked", G_CALLBACK(gtk_main_quit), NULL);
  }

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
#else
  g_error("Too old GTK version, we need at least 3.10!");
#endif
}

