#include <stdio.h>
#include <math.h>
#include <pixman.h>
#include <gtk/gtk.h>

// Длина строки псевдогалса.
#define GALS_SIZE (G_MAXUINT8)

#define PRINT_MATRIX(M) do                \
{                                         \
  int i;                                  \
  printf("-------\n");                    \
  printf("Matrix:\n");                    \
  for(i = 0; i < 3; i++)                  \
    printf("%08.4f\t%08.4f\t%08.4f\n",    \
      (double)(M).matrix[i][0] / 0x10000, \
      (double)(M).matrix[i][1] / 0x10000, \
      (double)(M).matrix[i][2] / 0x10000);\
  printf("-------\n");                    \
}                                         \
while(0)



G_GNUC_UNUSED static void draw_gals_deformation(cairo_surface_t *surf)
{
  // Do any pending drawing for the surface.
  // This function must be called before switching from drawing on the surface with cairo
  // to drawing on it directly with native APIs.
  cairo_surface_flush(surf);

  pixman_image_t *pimage = pixman_image_create_bits(
    PIXMAN_a8r8g8b8,
    cairo_image_surface_get_width(surf),
    cairo_image_surface_get_height(surf),
    (uint32_t*)cairo_image_surface_get_data(surf),
    cairo_image_surface_get_stride(surf));

  { // PIXMAN -->
    pixman_image_t *fill = pixman_image_create_bits(PIXMAN_a8r8g8b8, GALS_SIZE, 1, NULL, 0);

    { // Формируем псевдогалс -->
      gint i;
      uint32_t *data = pixman_image_get_data(fill);

      // Градиент, псевдогалс.
      i = GALS_SIZE / 2; // Строка!
      for(i = 0; i < GALS_SIZE; i++)
      {
        uint8_t *ptr = (uint8_t*)(data + i);
        *(ptr + 0) = G_MAXUINT8 - i;
        *(ptr + 1) = G_MAXUINT8 - i;
        *(ptr + 2) = 0;
        *(ptr + 3) = G_MAXUINT8;
      }
    } // Формируем псевдогалс <--

    // TRANSFORM -->
      #define D2F(d) (pixman_double_to_fixed(d))

      #if 1
        gdouble alpha = 0;
        alpha = (alpha / 360.0) * 2 * M_PI;
        pixman_transform_t trans = { {
          { D2F (cos(alpha)), D2F (-10-sin(alpha)), D2F (0)},
          { D2F (0.02 + sin(alpha)), D2F (cos(alpha)), D2F (0)},
          {  70 + D2F (0.00000), D2F (0.25 + 0), D2F (1)},
        }};
      #else
        pixman_transform_t trans;
        pixman_transform_init_identity(&trans);
      #endif

      pixman_transform_scale(NULL, &trans, D2F(1), D2F(30));

      //gdouble rotation = (-45 / 360.0) * 2 * M_PI;
      //pixman_transform_rotate(&trans, NULL, D2F(cos(rotation)), D2F(sin(rotation)));

      //pixman_transform_translate(NULL, &trans, D2F((double)GALS_SIZE / 2), D2F(0));

      // Применение матрицы -->
        pixman_image_set_transform(fill, &trans);

        #if 0
          pixman_image_set_filter(fill, PIXMAN_FILTER_BEST, NULL, 0);
        #else
          pixman_image_set_filter(fill, PIXMAN_FILTER_FAST, NULL, 0);
        #endif

        #if 0
          pixman_image_set_repeat(fill, PIXMAN_REPEAT_NORMAL);
        #else
          pixman_image_set_repeat(fill, PIXMAN_REPEAT_NONE);
        #endif
      // Применение матрицы <--
    // TRANSFORM <--

    // Функция композитинга -- смешивания слоев.
    //
    // -GALS_SIZE, ..., 0 -- сдвиги по оси X и по оси Y для src, mask и dest соответственно.
    //
    // 3 * GALS_SIZE, 3 * GALS_SIZE -- рисуем галс по всему окну.
    pixman_image_composite(PIXMAN_OP_OVER, fill, NULL, pimage,
      -GALS_SIZE, -GALS_SIZE, 0, 0, 0, 0,
      3 * GALS_SIZE, 3 * GALS_SIZE);

    pixman_image_unref(fill);
  } // PIXMAN <--

  pixman_image_unref(pimage);
}



G_GNUC_UNUSED static void draw_gals_rotation(cairo_surface_t *surf)
{
  // Do any pending drawing for the surface.
  // This function must be called before switching from drawing on the surface with cairo
  // to drawing on it directly with native APIs.
  cairo_surface_flush(surf);

  pixman_image_t *pimage = pixman_image_create_bits(
    PIXMAN_a8r8g8b8,
    cairo_image_surface_get_width(surf),
    cairo_image_surface_get_height(surf),
    (uint32_t*)cairo_image_surface_get_data(surf),
    cairo_image_surface_get_stride(surf));

  { // PIXMAN -->
    pixman_image_t *fill = pixman_image_create_bits(PIXMAN_a8r8g8b8, GALS_SIZE, 1, NULL, 0);

    { // ПОВОРОТ -->
      { // Формируем псевдогалс -->
        gint i;
        uint32_t *data = pixman_image_get_data(fill);

        // Градиент, псевдогалс.
        for(i = 0; i < GALS_SIZE; i++)
        {
          uint8_t *ptr = (uint8_t*)(data + i);
          *(ptr + 0) = G_MAXUINT8 - i;
          *(ptr + 1) = 0;
          *(ptr + 2) = G_MAXUINT8 - i;
          *(ptr + 3) = G_MAXUINT8;
        }
      } // Формируем псевдогалс <--

      // TRANSFORM -->
        #define D2F(d) (pixman_double_to_fixed(d))

        struct pixman_f_transform trans;
        pixman_f_transform_init_identity(&trans);

        #if 1 // СОБСТВЕННО ВКЛ./ОТКЛ. ПЕРЕВОРОТА.
          pixman_f_transform_translate(NULL, &trans, 0, -1);
          pixman_f_transform_translate(NULL, &trans, -(gdouble)GALS_SIZE / 2, 0);
            pixman_f_transform_rotate(NULL, &trans, -1, 0);
          pixman_f_transform_translate(NULL, &trans, (gdouble)GALS_SIZE / 2, 0);
        #endif

        // Немного масштабирования для теста.
        pixman_f_transform_scale(NULL, &trans, 0.75, 30);

        // Применение матрицы.
        {
          struct pixman_transform fixed_transform;
          pixman_transform_from_pixman_f_transform (&fixed_transform, &trans);
          pixman_image_set_transform(fill, &fixed_transform);
        }
      // TRANSFORM <--

      pixman_image_set_repeat(fill, PIXMAN_REPEAT_NONE);

      // Функция композитинга -- смешивания слоев.
      pixman_image_composite(PIXMAN_OP_OVER, fill, NULL, pimage,
        -GALS_SIZE, -GALS_SIZE, 0, 0, 0, 0,
        3 * GALS_SIZE, 3 * GALS_SIZE);
    } // ПОВОРОТ -->

    pixman_image_unref(fill);
  } // PIXMAN <--

  pixman_image_unref(pimage);
}



G_GNUC_UNUSED static void draw_gals_strong_zoom(cairo_surface_t *surf)
{
  // Do any pending drawing for the surface.
  // This function must be called before switching from drawing on the surface with cairo
  // to drawing on it directly with native APIs.
  cairo_surface_flush(surf);

  pixman_image_t *pimage = pixman_image_create_bits(
    PIXMAN_a8r8g8b8,
    cairo_image_surface_get_width(surf),
    cairo_image_surface_get_height(surf),
    (uint32_t*)cairo_image_surface_get_data(surf),
    cairo_image_surface_get_stride(surf));

  { // PIXMAN -->
    pixman_image_t *fill = pixman_image_create_bits(PIXMAN_a8r8g8b8, GALS_SIZE, 1, NULL, 0);

    { // Формируем псевдогалс -->
      gint i;
      uint32_t *data = pixman_image_get_data(fill);

      // Градиент, псевдогалс.
      for(i = 0; i < GALS_SIZE; i++)
      {
        uint8_t *ptr = (uint8_t*)(data + i);
        *(ptr + 0) = G_MAXUINT8 - i;
        *(ptr + 1) = G_MAXUINT8 - i;
        *(ptr + 2) = 0;
        *(ptr + 3) = G_MAXUINT8;
      }
    } // Формируем псевдогалс <--

    #define D2F(d) (pixman_double_to_fixed(d))
    pixman_transform_t trans;
    pixman_bool_t rval;
    double rate;

    pixman_image_set_filter(fill, PIXMAN_FILTER_FAST, NULL, 0);
    pixman_image_set_repeat(fill, PIXMAN_REPEAT_NONE);

    // GALS1 -->
      pixman_transform_init_identity(&trans);

      rate = 0.01558;
      rval = pixman_transform_scale(NULL, &trans, D2F(rate), D2F(30));
      printf("GALS1 Scale rval = %d, rate: float = %f, fixed = %d\n", rval, rate, D2F(rate));

      // Применение матрицы.
      rval = pixman_image_set_transform(fill, &trans);

      pixman_image_composite(PIXMAN_OP_OVER, fill, NULL, pimage,
        0, 0, 0, 0, GALS_SIZE, GALS_SIZE,
        GALS_SIZE, GALS_SIZE);
    // GALS1 <--

    // GALS2 -->
      pixman_transform_init_identity(&trans);

      rate = 0.01557;
      rval = pixman_transform_scale(NULL, &trans, D2F(rate), D2F(30));

      // Применение матрицы.
      rval = pixman_image_set_transform(fill, &trans);
      printf("GALS2 set transform rval = %d\n", rval);

      pixman_image_composite(PIXMAN_OP_OVER, fill, NULL, pimage,
        0, 0, 0, 0, GALS_SIZE, 1.5 * GALS_SIZE,
        GALS_SIZE, GALS_SIZE); //< !!
    // GALS2 <--

    // GALS3 -->
      pixman_transform_init_identity(&trans);

      rate = 0.01557;
      rval = pixman_transform_scale(NULL, &trans, D2F(rate), D2F(30));

      // Применение матрицы.
      rval = pixman_image_set_transform(fill, &trans);

      pixman_image_composite(PIXMAN_OP_OVER, fill, NULL, pimage,
        -GALS_SIZE, -2 * GALS_SIZE, 0, 0, 0, 0,
        3 * GALS_SIZE, 3 * GALS_SIZE); //< !!
    // GALS3 <--

    pixman_image_unref(fill);
  } // PIXMAN <--

  pixman_image_unref(pimage);
}



gboolean do_draw(GtkWidget *widget, cairo_t *widget_cr, gpointer user_data)
{
  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);

  cairo_surface_t *surf = cairo_image_surface_create(
    CAIRO_FORMAT_ARGB32, allocation.width, allocation.height);

  { // Полоса, чтобы поверх нее рисовать -->
    cairo_t *cr = cairo_create(surf);
    cairo_set_source_rgba(cr, 1, 1, 0, 0.5);
    cairo_set_line_width(cr, 50);

    cairo_move_to(cr, 0, 3 * GALS_SIZE);
    cairo_line_to(cr, 3 * GALS_SIZE, 0);
    cairo_stroke(cr);

    cairo_destroy(cr);
  } // Полоса, чтобы поверх нее рисовать <--

  // Рисуем галс.
  draw_gals_deformation(surf);
  //draw_gals_rotation(surf);
  //draw_gals_strong_zoom(surf);

  { // Сетка -->
    cairo_t *cr = cairo_create(surf);

    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_set_line_width(cr, 1);

    cairo_move_to(cr, 0,  GALS_SIZE);
    cairo_line_to(cr, GALS_SIZE, GALS_SIZE);
    cairo_line_to(cr, GALS_SIZE, 0);
    cairo_stroke(cr);

    cairo_move_to(cr, 2 * GALS_SIZE, 0);
    cairo_line_to(cr, 2 * GALS_SIZE, GALS_SIZE);
    cairo_line_to(cr, 3 * GALS_SIZE, GALS_SIZE);
    cairo_stroke(cr);

    cairo_move_to(cr, 2 * GALS_SIZE, 3 * GALS_SIZE);
    cairo_line_to(cr, 2 * GALS_SIZE, 2 * GALS_SIZE);
    cairo_line_to(cr, 3 * GALS_SIZE, 2 * GALS_SIZE);
    cairo_stroke(cr);

    cairo_move_to(cr, 0, 2 * GALS_SIZE);
    cairo_line_to(cr, GALS_SIZE, 2 * GALS_SIZE);
    cairo_line_to(cr, GALS_SIZE, 3 * GALS_SIZE);
    cairo_stroke(cr);

    cairo_destroy(cr);
  } // Сетка <--

  { // Сбрасываем рисунок на виджет -->
    cairo_set_source_surface(widget_cr, surf, 0, 0);
    cairo_paint(widget_cr);
  } // Сбрасываем рисунок на виджет <--

  return TRUE;
}



int main( int argc, char **argv )
{
  GtkWindow *main_window;
  GtkDrawingArea *drawing_area;

  gtk_init(&argc, &argv);

  main_window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_position(main_window, GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(main_window, 3 * GALS_SIZE, 3 * GALS_SIZE);

  drawing_area = GTK_DRAWING_AREA(gtk_drawing_area_new());
  gtk_container_add(GTK_CONTAINER(main_window), GTK_WIDGET(drawing_area));

  gtk_widget_show_all( GTK_WIDGET( main_window ) );

  g_signal_connect(G_OBJECT(main_window), "delete-event", gtk_main_quit, NULL);
  g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(do_draw), NULL);

  gtk_main();

  return 0;
}
