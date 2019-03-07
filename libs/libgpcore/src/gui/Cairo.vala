/*
 * GpCairo is a library with helpers for Cairo.
 *
 * Copyright (C) 2016 Sergey Volkhin.
 *
 * GpCairo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpCairo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpCairo. If not, see <http://www.gnu.org/licenses/>.
 *
*/
using Cairo;

namespace Gp
{
  /**
  * Лимит размера стороны изображения в Cairo.
  *
  * Комментарий из cairo-image-surface.c:
  * Limit on the width / height of an image surface in pixels.  This is
  * mainly determined by coordinates of things sent to pixman at the
  * moment being in 16.16 format.
  */
  public const int CAIRO_MAX_IMAGE_SIZE = 28000;

  /**
  * Прореживает данныые в одномерном массиве до размера Gp.CAIRO_MAX_IMAGE_SIZE (если необходимо).
  * @param data Массив (обычно в формате Cairo.Format.ARGB32 или RGB24).
  */
  public static void cairo_line_clamp(uint32[] data)
  {
    if(data.length <= Gp.CAIRO_MAX_IMAGE_SIZE)
      return;

    for(uint i = 0; i < Gp.CAIRO_MAX_IMAGE_SIZE; i++)
      data[i] = data[i * data.length / Gp.CAIRO_MAX_IMAGE_SIZE];
  }

  namespace Background
  {
    /**
    * Красный компонент для цвета фона.
    */
    public const double RED = 0.7;
    /**
    * Зеленый компонент для цвета фона.
    */
    public const double GREEN = 0.8;
    /**
    * Синий компонент для цвета фона.
    */
    public const double BLUE = 1;
  }

  namespace Foreground
  {
    /**
    * Красный компонент для цвета переднего плана.
    */
    private const double RED = 0.737;
    /**
    * Зеленый компонент для цвета переднего плана.
    */
    private const double GREEN = 0.737;
    /**
    * Синий компонент для цвета переднего плана.
    */
    private const double BLUE = 0.737;
  }

  /**
   * Рисует шашечную поверхность в заданном контексте
   * @param cr          Контекст Cairo.
   * @param width       Ширина закрашиваемой области в пикселях.
   * @param height      Высота закрашиваемой области в пикселях.
   * @param check_size  Размер шашки в пикселях.
   * @param spacing     расстояние между шашками в пикселях.
   */
  public void draw_checker_background(Context cr, uint width, uint height, uint check_size = 10, uint spacing = 0)
  {
    uint i, j, xcount, ycount;

    xcount = 0;
    i = spacing;
    while (i < width)
    {
      j = spacing;
      ycount = xcount % 2;
      while (j < height)
      {
        if ((ycount % 2) > 0)
          cr.set_source_rgb(Background.RED, Background.GREEN, Background.BLUE);
        else
          cr.set_source_rgb(1.0, 1.0, 1.0);

        cr.rectangle(i, j, check_size, check_size);
        cr.fill();

        j += check_size + spacing;
        ++ycount;
      }

      i += check_size + spacing;
      ++xcount;
    }
  }

  /**
   * Рисует диаграмму завершения некоего процесса в виде "пирога".
   * @param cr        Контекст Cairo.
   * @param fraction  Доля завершения процесса, от 0.0 до 1.0, если fraction отрицательный, то рисуется пирог с кругом внутри.
   * @param radius    Радиус "пирога".
   */
  public void draw_pie(Context cr, double fraction, uint radius = 12)
  {
    cr.scale(2 * radius, 2 * radius);
    cr.translate(0.5, 0.5);

    // Всю отрисовку ограничим радиусом.
    cr.arc(0, 0, 0.5, 0, 2 * Math.PI);
    cr.clip();

    // Внешнняя окружность.
    cr.arc(0, 0, 0.5, 0, 2 * Math.PI);
    cr.set_line_width(0.1);
    cr.set_source_rgb(Foreground.RED, Foreground.GREEN, Foreground.BLUE);
    cr.stroke();

    cr.set_line_width(0.0);

    if(fraction >= 0)
    {
      cr.arc(0, 0, 0.5, - Math.PI_2, 2 * Math.PI * fraction - Math.PI_2);
      cr.line_to(0, 0);
      cr.line_to(0, -0.5);
      cr.fill();
    }
    else
    {
      cr.set_source_rgba(Foreground.RED, Foreground.GREEN, Foreground.BLUE, 0.66);
      cr.arc(0, 0, 0.3, 0.0, 2 * Math.PI);
    }

    cr.fill();
  }

  /**
   * Создает новую поверхность ImageSurface, отресайзив поверхность src.
   * Если указанные пропорции отличаются от пропорций src,
   * то src умещается в новый размер с полями.
  */
  public Surface cairo_surface_resize(int dest_width, int dest_height, Surface src, int src_width, int src_height)
  {
    var dest = new Cairo.Surface.similar(src, Content.COLOR_ALPHA, dest_width, dest_height);
    var cr = new Cairo.Context(dest);

    double ratio_width = (double)dest_width / src_width;
    double ratio_height = (double)dest_height / src_height;
    double ratio = double.min(ratio_width, ratio_height);

    cr.translate((dest_width - ratio * src_width) / 2, (dest_height - ratio * src_height) / 2);
    cr.scale(ratio, ratio);
    cr.set_source_surface(src, 0, 0);
    cr.get_source().set_filter(Cairo.Filter.NEAREST);
    cr.paint();

    return dest;
  }
}

