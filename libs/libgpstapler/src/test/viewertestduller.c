#include "duller.h"
#include "gp-tile-viewer.h"


#define DEFAULT_UPDATE_INTERVAL 1500



typedef struct
{
  GpTileViewer *viewer;
  Duller *tiler;

  GpSmartCache *cache;

  gboolean online_update;
  guint upd_interval;

  guint upd_event_source_id;

  gdouble test_coef;

  GpDispatcher *pool;

  GtkWidget *window;
}
Tester;


// Статические функции -->
  static void on_quit_cb(Tester *tt);
// Статические функции <--



static void on_quit_cb(Tester *tt)
{
  g_return_if_fail(tt);
  gtk_main_quit();
}


int main(int argc, char **argv)
{
  Tester *tt = g_new0(Tester, 1);

  gtk_init(&argc, &argv);

  // Опции командной строки -->
    gint threads_num = 4;

    {
      GOptionContext *context = NULL;

      GOptionEntry entries[] =
      {
        { "online",     'o', 0, G_OPTION_ARG_NONE,    &tt->online_update, "Online update", NULL },
        { "interval",   'i', 0, G_OPTION_ARG_INT,     &tt->upd_interval,  "Interval for online update (default is " G_STRINGIFY(DEFAULT_UPDATE_INTERVAL) ")", NULL },
        { "test_coef",  'c', 0, G_OPTION_ARG_DOUBLE,  &tt->test_coef,     "Experimental coefficient (e.g. brightness, e.g. '-c 16')", NULL },
        { "threads_num",'t', 0, G_OPTION_ARG_INT,     &threads_num,       "Threads num for generating tiles with aqua data", NULL },
        { NULL }
      };

      context = g_option_context_new("- TileView test app");
      g_option_context_add_main_entries(context, entries, NULL);
      g_option_context_set_ignore_unknown_options(context, TRUE );
      g_option_context_add_group(context, gtk_get_option_group(TRUE));

      if(!g_option_context_parse(context, &argc, &argv, NULL))
      {
        g_warning("Failed to parse options.");
        return -1;
      }

      g_option_context_free(context);
    }
  // Опции командной строки <--

  if(threads_num)
    tt->pool = GP_DISPATCHER(gp_dispatcher_thread_pool_new(threads_num, FALSE, NULL));
  else
    tt->pool = GP_DISPATCHER(gp_dispatcher_loop_new());


  tt->cache = gp_smart_cache_new ();
  g_assert(tt->cache);
  gp_smart_cache_set_size (tt->cache, 1 * 1024 * 1024 * 1024); //< TODO Гиг

  tt->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  //gtk_window_set_default_size(GTK_WINDOW(tt->window), 800, 600);

  {
    static const char *ico_xpm[] =
    {
      "16 16 34 1"," 	c None",
      ".	c #6E939C","+	c #8E8EC6","@	c #8E8EAA","#	c #5C5C5C",
      "$	c #E3C7C7","%	c #E3ABAB","&	c #8E7272","*	c #FFC7AB",
      "=	c #FFC7C7","-	c #404040",";	c #5D7777",">	c #FFE4C7",
      ",	c #000000","'	c #C68E8E",")	c #E38F8F","!	c #FFABAB",
      "~	c #553939","{	c #C6AAAA","]	c #AA8E8E","^	c #7A7AB0",
      "/	c #7272AA","(	c #4E4E4E","_	c #ABABE3",":	c #72728E",
      "<	c #4C6166","[	c #AAAAC6","}	c #AAAAAA","|	c #717171",
      "1	c #555555","2	c #715555","3	c #717155","4	c #070707",
      "5	c #393939",".+++..@@....++..","+.#$%&*%%*===-;+",
      "#%=>&,'=*)!=>~,{","!%*=],'*%=%==]~=","*=*=={=%*=%%*!>*","===**==*===*%%*=",
      "=%%=%*=^==%%=%*$","^!==**=.#=%==%*{",";==*!=^.@%*==!$+","+.==*$.+@.{==%..",
      "@/;((;+_.++(@@@;","+;::<..@@+.;;;@.","+[+[...;.@+.....",".@.+[:([[#}@@@++",
      ".+.+++|123|@..[@","+++.+;.:45;..;@@"
    };

    gtk_window_set_icon(GTK_WINDOW(tt->window), gdk_pixbuf_new_from_xpm_data(ico_xpm));
  }

  { // GpTiler -->
    tt->tiler = duller_new(tt->cache, tt->pool);

    gp_tiler_update_data(GP_TILER(tt->tiler));
  } // GpTiler -->

  GtkWidget *main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_container_add(GTK_CONTAINER(tt->window), main_hbox);

  tt->viewer = gp_tile_viewer_new(GP_TILER(tt->tiler));
  {
    GpTile tile;
    tile.x = 0;
    tile.y = 0;
    tile.l = 1;
    tile.type = duller_get_tiles_type(tt->tiler);
    gp_tile_viewer_show_tile( tt->viewer, tile );
  }
  gtk_box_pack_start(GTK_BOX(main_hbox), GTK_WIDGET(tt->viewer), TRUE, TRUE, 0);

  if(tt->online_update || tt->upd_interval)
  {
    tt->upd_event_source_id = g_timeout_add(
    tt->upd_interval ? tt->upd_interval : DEFAULT_UPDATE_INTERVAL,
    (GSourceFunc) gp_tiler_update_data, tt->tiler);
    g_signal_connect_swapped(tt->tiler, "data-updated",
                             G_CALLBACK( gp_tile_viewer_refresh ), tt->viewer);
  }

  g_signal_connect_swapped(G_OBJECT(tt->window), "destroy", G_CALLBACK(on_quit_cb), tt);
  gtk_widget_show_all(tt->window);

  gtk_main();

  if(tt->upd_event_source_id != 0)
    g_source_remove(tt->upd_event_source_id);

  g_clear_object(&tt->cache);
  g_free(tt);

  #if 0
    puts("Waiting 1 sec...");
    fflush(stdout);
    sleep(1); // Yep, sleep..
  #endif

  puts("Exiting...");
  fflush(stdout);

  return 0;
}
