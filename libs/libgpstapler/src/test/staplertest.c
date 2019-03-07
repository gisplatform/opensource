#include <math.h>
#include <gtk/gtk.h>

#include "duller.h"
#include "muddy.h"
#include "weigher.h"
#include "gp-cifroarea.h"
#include "gp-core-gui.h"
#include "gp-icastaterenderer.h"
#include "gp-getup.h"
#include "gp-stapler.h"
#include "gp-tiler.h"

#include "gp-smartcache.h"


static const int BORDER_SIZE_1 = 25;
static const int BORDER_SIZE_2 = 10;

#define DEFAULT_UPDATE_INTERVAL 1500

static GpStapler *GP_STAPLER = NULL;



static gboolean on_area_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
  GpStapler *stapler = GP_STAPLER(user_data);

  g_return_val_if_fail(stapler, FALSE);

  if(event->type == GDK_2BUTTON_PRESS)
  {
    printf("Double click... ");

    GtkTreeIter iter;
    gdouble x_val, y_val;
    GdkEventButton *ev = (GdkEventButton*)event;

    gp_ica_state_renderer_area_point_to_value( gp_stapler_get_state_renderer( stapler ), ev->x, ev->y, &x_val, &y_val );
    uint32_t pixel = gp_stapler_get_upper_tiler_in_coord(stapler, &iter, x_val, y_val, TRUE);

    if(pixel)
    {
      gchar *name = NULL;
        gtk_tree_model_get(GTK_TREE_MODEL(stapler), &iter, GP_TILER_TREE_MODEL_COLS_NAME, &name, -1);
        printf("GpTiler '%s' Val: [%x].\n", name, pixel);
      g_free(name);
      goto end;
    }

    printf("Click in emptiness.\n");
  }

end:
  return FALSE;
}



static void on_border_size_button_clicked(GtkButton *button, gpointer user_data)
{
  GpCifroArea *carea = GP_CIFRO_AREA(user_data);
  g_return_if_fail(carea);

  gint old_size;
  gp_cifro_area_get_border (carea, &old_size, NULL, NULL, NULL);

  if(old_size == BORDER_SIZE_1)
    gp_cifro_area_set_border (carea, BORDER_SIZE_2, BORDER_SIZE_2, BORDER_SIZE_2, BORDER_SIZE_2);
  else
    gp_cifro_area_set_border (carea, BORDER_SIZE_1, BORDER_SIZE_1, BORDER_SIZE_1, BORDER_SIZE_1);
}



static void on_clear_clicked(GtkButton *button, gpointer user_data)
{
  GtkToggleButton *cache_included_button = GTK_TOGGLE_BUTTON(g_object_get_data(G_OBJECT(button), "cache-included-button"));
  g_return_if_fail(cache_included_button);

  GtkTreeView *traverses_tree_view = GTK_TREE_VIEW(g_object_get_data(G_OBJECT(button), "traverses-tree-view"));
  g_return_if_fail(traverses_tree_view);

  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  if(gtk_tree_selection_get_selected(gtk_tree_view_get_selection(GTK_TREE_VIEW(traverses_tree_view)), &model, &iter))
  {
    GpTiler *tiler = NULL;
    gtk_tree_model_get(model, &iter, GP_TILER_TREE_MODEL_COLS_TILER, &tiler, -1);
    if(tiler)
      gp_stapler_clear_tiler(GP_STAPLER( model ), &iter, gtk_toggle_button_get_active( cache_included_button ), NULL,
                             NULL);
  }
}


static void on_data_updated(GpTiler* self, gpointer user_data)
{
  GpActivityButton *ab = GP_ACTIVITY_BUTTON(user_data);
  const gchar *name = gp_tiler_get_name(self);
  gp_activity_button_done(ab, name);
  printf("GpTiler '%s' updated\n", name);
}


static void on_data_updating(GpTiler* self, gdouble fraction, gpointer user_data)
{
  GpActivityButton *ab = GP_ACTIVITY_BUTTON(user_data);
  const gchar *name = gp_tiler_get_name(self);
  gp_activity_button_set_fraction(ab, name, fraction, NULL);
  printf("GpTiler '%s' updating %.0f%%\n", name, fraction * 100);
}


static void on_restacked(GpStapler* self, gpointer user_data)
{
  GpActivityButton *ab = GP_ACTIVITY_BUTTON(user_data);

  static const gchar* activity_name = "Tiles";

  gint completion = gp_ica_renderer_get_completion(GP_ICA_RENDERER(self), 0);

  printf("restacked: %d\n", completion);

  if(completion == 1000)
    gp_activity_button_done(ab, activity_name );
  else
    gp_activity_button_set_fraction(ab, activity_name , (double)completion / 1000., NULL);
}


int main( int argc, char **argv )
{
  GtkWidget *window;
  GtkWidget *main_hbox;

  GpCifroArea *carea;
  GpIcaRenderer *state_renderer;

  gtk_init( &argc, &argv );
  g_random_set_seed( 0 );

  // Опции командной строки -->
    gboolean no_grid = FALSE;
    gboolean no_scale = FALSE;
    gboolean one_tiler = FALSE;
    gboolean online = FALSE;
    gint threads_num = 4;

    {
      GOptionContext *context = NULL;

      GOptionEntry entries[] =
      {
        { "no_grid",  'r', 0, G_OPTION_ARG_NONE, &no_grid, "Turn off grid ('GpGetup' layer)", NULL },
        { "no_scale", 'x', 0, G_OPTION_ARG_NONE, &no_scale, "Turn off scale (scale as is, 1:1)", NULL },
        { "one_tiler", '1', 0, G_OPTION_ARG_NONE, &one_tiler, "Create only one tiler", NULL },
        { "online", 'o', 0, G_OPTION_ARG_NONE, &online, "Update tiler online", NULL },
        { "threads_num",'t', 0, G_OPTION_ARG_INT,     &threads_num,       "Threads num for generating tiles with aqua data", NULL },
        { NULL }
      };

      context = g_option_context_new("- GpStapler test app");
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

  GpSmartCache *cache = NULL;

  cache = gp_smart_cache_new ();
  g_assert(cache);
  gp_smart_cache_set_size (cache, 1 * 1024 * 1024 * 1024); //< TODO Гиг

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

  {
    static const char *ico_xpm[] =
    {
      "24 24 10 1", "   c None", ". c #FF0000", "+  c #BF4040", "@  c #40BFBF", "#  c #404040",
      "$  c #000000", "%  c #400000", "&  c #004040", "*  c #0000FF", "=  c #4000FF",
      "........................", "+@@@@@@@@@@+@@@@@@@@@@@.", "+@@@@@@@@@@+@@@@@@@@@@@.", "+@@@@@@@@@@+@@@@@@@@@@@.",
      "+@@@@@@@@@@+@@@@@@@@@@@.", "+@@@@@@@@@@+@@@@@@@@@@@.", "+@@######@@+@@@######@@.", "+@@######@@+@@@######@@.",
      "+@@######@@+@@@######@@.", "+@@@@@@@@@@+@@@@@@@@@@@.", "+@@@@@@@@@@+@@@@@@@@@@@.", "........................",
      "$$$$$%%%%%%%............", "$$$$$%%%%%%%@@@@@@@@@@@.", "$$$$$%%%%%%%@@@@@@@@@@@.", "$$$$$$$$$%%%@@@@@@@@@@@.",
      "&&&$$$$$$###@@@@@@@@@@@.", "&&&******###@@@@@@@@@@@.", "&&&======###@@@######@@.", "&&&$$$$$$###@@@######@@.",
      "&&&$$$$$$###@@@######@@.", "&&&&&#######@@@@@@@@@@@.", "&&&&&#######@@@@@@@@@@@.", "&&&&&#######............"
    };

    gtk_window_set_icon(GTK_WINDOW(window), gdk_pixbuf_new_from_xpm_data(ico_xpm));
  }


  carea = GP_CIFRO_AREA(gp_cifro_area_new (20) );

puts("Adding state");
  state_renderer = GP_ICA_RENDERER( gp_ica_state_renderer_new() );
  gp_cifro_area_add_layer (carea, state_renderer);

  GpDispatcher *pool;
  if(threads_num)
    pool = GP_DISPATCHER(gp_dispatcher_thread_pool_new(threads_num, TRUE, NULL));
  else
    pool = GP_DISPATCHER(gp_dispatcher_loop_new());

  GpActivityButton *ab = gp_activity_button_new();

puts("Adding stapler");
  GP_STAPLER = gp_stapler_new(GP_ICA_STATE_RENDERER( state_renderer ));
  g_signal_connect(GP_STAPLER, "restacked", G_CALLBACK(on_restacked), ab);

  GpTiler *tiler;
  GtkTreeIter iter;

  if(one_tiler)
  {
    tiler = GP_TILER(duller_new(cache, pool));

    g_signal_connect(tiler, "data-updated", G_CALLBACK(on_data_updated), ab);
    g_signal_connect(tiler, "data-updating", G_CALLBACK(on_data_updating), ab);
    gp_async_tiler_update_data_all(GP_ASYNC_TILER(tiler));
    gp_stapler_append_tiler( GP_STAPLER, tiler, duller_get_tiles_type(DULLER( tiler )), &iter );

    if(online)
      gp_stapler_set_tiler_update_data_timeout( GP_STAPLER, &iter, DEFAULT_UPDATE_INTERVAL );

  }
  else
  {
    GpMixTiler *mixer = GP_MIX_TILER(gp_mix_tiler_new(cache, GTK_TREE_MODEL(GP_STAPLER), 1));
    gp_stapler_append_tiler( GP_STAPLER, GP_TILER(mixer), 0, &iter );

    tiler = GP_TILER(duller_new(cache, pool));

    g_signal_connect(tiler, "data-updated", G_CALLBACK(on_data_updated), ab);
    g_signal_connect(tiler, "data-updating", G_CALLBACK(on_data_updating), ab);
    gp_async_tiler_update_data_all(GP_ASYNC_TILER(tiler));
    gp_stapler_append_tiler( GP_STAPLER, tiler, duller_get_tiles_type(DULLER( tiler )), &iter );

    if(online)
      gp_stapler_set_tiler_update_data_timeout( GP_STAPLER, &iter, DEFAULT_UPDATE_INTERVAL );

    tiler = GP_TILER(weigher_new(cache));
    gp_stapler_append_tiler( GP_STAPLER, tiler, weigher_get_tiles_type(WEIGHER( tiler )), &iter );
    weigher_set_area(WEIGHER(tiler), 55, 155, 500, 100, 0.1);
    weigher_set_bottom_type(WEIGHER(tiler), 0);
    weigher_set_data_type(WEIGHER(tiler), 0);

    gp_stapler_set_tiler_fixed_l(GP_STAPLER, &iter, weigher_get_tiles_type(WEIGHER(tiler)), 10000 /* 10 метров*/);

    tiler = GP_TILER(weigher_new(cache));
    gp_stapler_append_tiler( GP_STAPLER, tiler, weigher_get_tiles_type(WEIGHER( tiler )), &iter );
    weigher_set_area(WEIGHER(tiler), 55, 155, 100, 500, 0.1);
    weigher_set_bottom_type(WEIGHER(tiler), 1);
    weigher_set_data_type(WEIGHER(tiler), 0);

    gp_stapler_set_tiler_fixed_l(GP_STAPLER, &iter, weigher_get_tiles_type(WEIGHER(tiler)), 10000 /* 10 метров*/);

    //~ gp_mix_tiler_determ_area(mixer);
    //~ tiler = GP_TILER(muddy_new(cache));
    //~ gp_stapler_append_tiler( GP_STAPLER, tiler, muddy_get_tiles_type(MUDDY( tiler )), &iter );
    //~ muddy_set_model(MUDDY( tiler ), GTK_TREE_MODEL(GP_STAPLER),  GP_TILER_TREE_MODEL_COLS_TILER, GP_TILER_TREE_MODEL_COLS_VISIBLE);
    //~ muddy_set_sum_type(MUDDY( tiler ), 0);
//~
    //~ gp_stapler_set_tiler_fixed_l(GP_STAPLER, &iter, muddy_get_tiles_type(MUDDY(tiler)), 10000 /* 10 метров*/);

    //~ gdouble delta = 10.;
    //~ gp_stapler_set_tiler_deltas( GP_STAPLER, &iter, &delta, NULL);

  }

  gp_cifro_area_add_layer (carea, GP_ICA_RENDERER( GP_STAPLER ));

  if(!no_grid)
  {
    puts("Adding getup");
    GpGetup *getup = gp_getup_new(GP_ICA_STATE_RENDERER( state_renderer ));
    gp_cifro_area_add_layer (carea, GP_ICA_RENDERER(getup));
  }

  gp_cifro_area_set_swap (carea, TRUE, TRUE);

  gp_cifro_area_set_border (carea, BORDER_SIZE_1, BORDER_SIZE_1, BORDER_SIZE_1, BORDER_SIZE_1);

  gp_cifro_area_set_shown_limits (carea, -100, 10000, -100, 10000);
//  gp_cifro_area_set_shown_limits( carea, -G_MAXDOUBLE, G_MAXDOUBLE, -G_MAXDOUBLE, G_MAXDOUBLE );
//  gp_cifro_area_set_scale_limits( carea, 0.0001, 1000.0, 0.0001, 1000.0 );

  if(no_scale)
  {
    puts("Setting no scale mode...");
    gp_cifro_area_set_scale_limits (carea, 1.0, 1.0, 1.0, 1.0);
  }
  else
    gp_cifro_area_set_scale_limits (carea, 0.02, 20.0, 0.02, 20.0);

  gp_cifro_area_set_minimum_visible (carea, 64, 64);
  gp_cifro_area_set_scale_on_resize (carea, TRUE);
  gp_cifro_area_set_swap (carea, FALSE, FALSE);
//  gp_cifro_area_set_angle( carea, G_PI / 31.4 );
  gp_cifro_area_set_rotation (carea, TRUE);
  //gp_cifro_area_set_scale_aspect( carea, 1.0 );
//  gp_cifro_area_set_shown( carea, -100, 100, -100, 100 );
  gp_cifro_area_set_shown (carea, -30, 30, -30, 30);
  gp_cifro_area_set_zoom_on_center (carea, FALSE);
  gp_cifro_area_set_zoom_scale (carea, 2.0);
 // gp_cifro_area_set_fixed_zoom_scales( carea, scales_x, scales_y, 3);
  gp_cifro_area_set_move_multiplier (carea, 25);
  gp_cifro_area_set_rotate_multiplier (carea, 0.1);


  GtkWidget *frame = gtk_frame_new("demo");
  gtk_box_pack_start(GTK_BOX(main_hbox), GTK_WIDGET(frame), FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(main_hbox), GTK_WIDGET(carea), TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(window), main_hbox);

  {
    GtkWidget *widget, *prop_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(frame), prop_vbox);

    // Список GpTiler'ов -->
      gtk_box_pack_start(GTK_BOX(prop_vbox), gtk_label_new("Tilers:"), FALSE, FALSE, 0);

      GtkWidget *traverses_sw = gtk_scrolled_window_new(NULL, NULL);
      gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(traverses_sw), GTK_SHADOW_ETCHED_IN);
      gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(traverses_sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
      gtk_widget_set_size_request(traverses_sw, 200, 200);
      gtk_box_pack_start(GTK_BOX(prop_vbox), traverses_sw, FALSE, FALSE, 0);

      GtkWidget *traverses_tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL( GP_STAPLER ));
      gtk_tree_view_set_reorderable(GTK_TREE_VIEW(traverses_tree_view), TRUE);
      gtk_container_add(GTK_CONTAINER(traverses_sw), traverses_tree_view);

      { // Отображаемые колонки -->
        GtkCellRenderer *renderer = NULL;
        GtkTreeViewColumn *column = NULL;

        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Name",
          renderer, "text", GP_TILER_TREE_MODEL_COLS_NAME, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(traverses_tree_view), column);

        renderer = gtk_cell_renderer_toggle_new();
        g_signal_connect_swapped(renderer, "toggled",
                                 G_CALLBACK( gp_stapler_cell_renderer_visibility_toggled_cb ), GP_STAPLER );
        column = gtk_tree_view_column_new_with_attributes("Vis.",
          renderer, "active", GP_TILER_TREE_MODEL_COLS_VISIBLE, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(traverses_tree_view), column);

        renderer = gtk_cell_renderer_toggle_new();
        g_signal_connect_swapped(renderer, "toggled",
                                 G_CALLBACK( gp_stapler_cell_renderer_highlight_toggled_cb ), GP_STAPLER );
        column = gtk_tree_view_column_new_with_attributes("Ligh.",
          renderer, "active", GP_TILER_TREE_MODEL_COLS_HIGHLIGHT, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(traverses_tree_view), column);

        renderer = gtk_cell_renderer_text_new();
        g_object_set(renderer, "editable", TRUE, NULL);
        g_signal_connect_swapped(renderer, "edited",
                                 G_CALLBACK( gp_stapler_cell_renderer_update_timeout_edited_cb ), GP_STAPLER );
        column = gtk_tree_view_column_new_with_attributes("ms.",
          renderer, "text", GP_TILER_TREE_MODEL_COLS_UPD_TIMEOUT, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(traverses_tree_view), column);

        GtkAdjustment *adjustment = NULL;
          adjustment = gtk_adjustment_new(
            0,            //< The initial value.
            -1000, 1000,  //< The minimum and maximum value.
            10,           //< The step increment.
            1,            //< The page increment.
            0);           //< The page size.
        g_object_ref_sink(adjustment);

          renderer = gtk_cell_renderer_spin_new();
          g_object_set(renderer, "editable", TRUE, "adjustment", adjustment, NULL);
          g_signal_connect_swapped(renderer, "edited",
                                   G_CALLBACK( gp_stapler_cell_renderer_delta_x_edited_cb ), GP_STAPLER );
          column = gtk_tree_view_column_new_with_attributes("∆X",
            renderer, "text", GP_TILER_TREE_MODEL_COLS_DELTA_X, NULL);
          gtk_tree_view_append_column(GTK_TREE_VIEW(traverses_tree_view), column);

          renderer = gtk_cell_renderer_spin_new();
          g_object_set(renderer, "editable", TRUE, "adjustment", adjustment, NULL);
          g_signal_connect_swapped(renderer, "edited",
                                   G_CALLBACK( gp_stapler_cell_renderer_delta_y_edited_cb ), GP_STAPLER );
          column = gtk_tree_view_column_new_with_attributes("∆Y",
            renderer, "text", GP_TILER_TREE_MODEL_COLS_DELTA_Y, NULL);
          gtk_tree_view_append_column(GTK_TREE_VIEW(traverses_tree_view), column);

        g_clear_object(&adjustment);
      } // Отображаемые колонки <--
    // Список GpTiler'ов <--

    // Очистка -->
      gtk_box_pack_start(GTK_BOX(prop_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);

      gtk_box_pack_start(GTK_BOX(prop_vbox), gtk_label_new("Clear:"), FALSE, FALSE, 0);

      GtkWidget *cache_included_button = gtk_check_button_new_with_label("(cache included)");
      gtk_box_pack_start(GTK_BOX(prop_vbox), cache_included_button, FALSE, FALSE, 0);

      widget = gtk_button_new_with_label("Clear now!");
      g_object_set_data_full(G_OBJECT(widget), "cache-included-button", g_object_ref(cache_included_button), g_object_unref);
      g_object_set_data_full(G_OBJECT(widget), "traverses-tree-view", g_object_ref(traverses_tree_view), g_object_unref);
      g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(on_clear_clicked), NULL);
      gtk_box_pack_start(GTK_BOX(prop_vbox), GTK_WIDGET(widget), FALSE, FALSE, 0);

      gtk_box_pack_start(GTK_BOX(prop_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    // Очистка <--

    widget = gtk_button_new_with_label("Bord.size");
      g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(on_border_size_button_clicked), carea);
    gtk_box_pack_start(GTK_BOX(prop_vbox), GTK_WIDGET(widget), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(prop_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(prop_vbox), GTK_WIDGET(ab), FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(prop_vbox), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL), FALSE, FALSE, 5);
    widget = gtk_button_new_from_icon_name("application-exit", GTK_ICON_SIZE_BUTTON);
      g_signal_connect(G_OBJECT(widget), "clicked", G_CALLBACK(gtk_main_quit), NULL);
    gtk_box_pack_start(GTK_BOX(prop_vbox), GTK_WIDGET(widget), FALSE, FALSE, 0);
  }

  g_signal_connect_after(G_OBJECT(carea), "button-press-event", G_CALLBACK(on_area_button_press), GP_STAPLER );

  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_default_size( GTK_WINDOW( window ), 640, 480 );
  gtk_widget_show_all( window );

  gtk_main();

  if(cache)
    g_object_unref(cache);

  return 0;
}
