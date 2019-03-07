/*
 * GpPoint is a library for geographical points manipulation.
 *
 * Copyright (C) 2015 Gennadiy Nefediev, Sergey Volkhin.
 *
 * GpPoint is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpPoint is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpPoint. If not, see <http://www.gnu.org/licenses/>.
 *
*/

using GLib;

namespace Gp
{
  /**
  * Параметры трансформирования систем координат при пересчете между эллипсоидами.
  */
  private struct GeoTransParams
  {
    /**
    * Масштабный коэффициент.
    */
    double m;

    /**
    * Зд. и далее -- линейные элементы (м).
    */
    double dx;
    double dy;
    double dz;

    /**
    * Зд. и далее -- угловые элементы (угл.сек.)
    */
    double wx;
    double wy;
    double wz;
  }

  /**
  * Относительные параметры эллипсоидов при трансформировании из системы 1 в систему 2.
  */
  private struct EllTransParams
  {
    /**
    * ах_а(2) - ах_а(1)
    */
    double da;
    /**
    * (ах_а(2) + ах_а(1)) / 2
    */
    double ha;
    /**
    * е2(2) - е2(1)
    */
    double de2;
    /**
    *(е2(2) + е2(1)) / 2
    */
    double he2;
  }

  /**
  * Параметры общеземного эллипсоида.
  */
  private struct EllParams
  {
    /**
    * Большая полуось (м).
    */
    double ax_a;
    /**
    * Малая полуось (м).
    */
    double ax_b;
    /**
    * Полярный радиус (м).
    */
    double ax_c;
    /**
    * Сжатие.
    */
    double alpha;
    /**
    * Квадрат эксцентриситета.
    */
    double e2;
    /**
    * Квадрат второго эксцентриситета.
    */
    double es2;
  }

  /**
  * Коэффициенты расчета длин дуг меридианов между разными широтами, и наоборот.
  */
  private struct ArcParams
  {
    /**
    * Коэффициенты вычисления длин дуг меридианов.
    */
    double c0;
    double c2;
    double c4;
    double c6;
    /**
    * Коэффициенты вычисления широт по длинам дуг меридианов.
    */
    double d2;
    double d4;
    double d6;
  }

  /**
  * Направления преобразований между геодезическими системами координат.
  */
  private enum GeoTransType
  {
    /**
    * wgs-84 (g1150) -> пз-90.11.
    */
    WGS_TO_PZ = 0,
    /**
    * wgs-84 (g1150) -> ск-42.
    */
    WGS_TO_CK,
    /**
    * пз-90.11 -> ск-42.
    */
    PZ_TO_CK,
    MAX
  }

  /**
  * Географические системы координат.
  */
  public enum CoordSysType
  {
    NULL,
    /**
    * WGS-84.
    */
    WGS_84,
    /**
    * ПЗ-90.
    */
    PZ_90,
    /**
    * СК-42.
    */
    CK_42,
    GEO_MAX,
    /**
    * UTM.
    */
    UTM,
    /**
    * Прямоугольные координаты Гаусса-Крюгера для WGS-84.
    */
    GK,
    /**
    * Координаты Гаусса-Крюгера для СК-42.
    */
    CK_42_GK,
    MAX;


    public string to_string()
    {
      switch(this)
      {
        case WGS_84:   return _("WGS-84");
        case PZ_90:    return _("PZ-90");
        case CK_42:    return _("CK-42");
        case UTM:      return _("UTM");
        case GK:       return _("Gauss-Kruger");
        case CK_42_GK: return _("CK-42 Gauss-Kruger");
        default:       return _("Unknown system");
      }
    }

    /**
    * Проверяет, что виджет с координатной сеткой поддерживает отображение в данной системе.
    */
    public bool grid_supported()
    {
      switch(this)
      {
        case WGS_84:
        case PZ_90:
        case UTM:
          return true;
        default:
          return false;
      }
    }

  }


  /**
   * Коэффициенты для рсчета магнитного склонения.
   *
   * Fields are: n, m, g, h, dg/dt, dh/dt.
   */
  private struct WMM
  {
    public int n;
    public int m;
    public double g_t0;
    public double h_t0;
    public double dg_t0;
    public double dh_t0;
  }


  /**
  * Объект для хранения и преобразования географических координат точки.
  */
  public struct Point
  {
    /**
    * Система кординат (WGS-84 / ПЗ-90 / UTM / Гаусс-Крюгер).
    */
    public CoordSysType type;
    /**
    * Номер зоны UTM (1-60, 0 - не определено).
    */
    public uint zone;
    /**
    * L: долгота / longitude (рад), восточное смещение / easting (м).
    */
    public double x;
    /**
    * B: широта / latitude (рад), северное смещение / northing (м).
    */
    public double y;
    /**
    * H: высота / altitude, m.
    */
    public double h;


    /**
     * Таблица коэффициентов WMM2010.
     *
     * Fields are: n, m, g, h, dg/dt, dh/dt.
     * Units are nT for g and h and nT/year for dg/dt and dh/dt.
     */
    private const WMM WMM_2010[] =
    {
      {  1,  0, -29496.6,     0.0,  11.6,   0.0 },
      {  1,  1,  -1586.3,  4944.4,  16.5, -25.9 },
      {  2,  0,  -2396.6,     0.0, -12.1,   0.0 },
      {  2,  1,   3026.1, -2707.7,  -4.4, -22.5 },
      {  2,  2,   1668.6,  -576.1,   1.9, -11.8 },
      {  3,  0,   1340.1,     0.0,   0.4,   0.0 },
      {  3,  1,  -2326.2,  -160.2,  -4.1,   7.3 },
      {  3,  2,   1231.9,   251.9,  -2.9,  -3.9 },
      {  3,  3,    634.0,  -536.6,  -7.7,  -2.6 },
      {  4,  0,    912.6,     0.0,  -1.8,   0.0 },
      {  4,  1,    808.9,   286.4,   2.3,   1.1 },
      {  4,  2,    166.7,  -211.2,  -8.7,   2.7 },
      {  4,  3,   -357.1,   164.3,   4.6,   3.9 },
      {  4,  4,     89.4,  -309.1,  -2.1,  -0.8 },
      {  5,  0,   -230.9,     0.0,  -1.0,   0.0 },
      {  5,  1,    357.2,    44.6,   0.6,   0.4 },
      {  5,  2,    200.3,   188.9,  -1.8,   1.8 },
      {  5,  3,   -141.1,  -118.2,  -1.0,   1.2 },
      {  5,  4,   -163.0,     0.0,   0.9,   4.0 },
      {  5,  5,     -7.8,   100.9,   1.0,  -0.6 },
      {  6,  0,     72.8,     0.0,  -0.2,   0.0 },
      {  6,  1,     68.6,   -20.8,  -0.2,  -0.2 },
      {  6,  2,     76.0,    44.1,  -0.1,  -2.1 },
      {  6,  3,   -141.4,    61.5,   2.0,  -0.4 },
      {  6,  4,    -22.8,   -66.3,  -1.7,  -0.6 },
      {  6,  5,     13.2,     3.1,  -0.3,   0.5 },
      {  6,  6,    -77.9,    55.0,   1.7,   0.9 },
      {  7,  0,     80.5,     0.0,   0.1,   0.0 },
      {  7,  1,    -75.1,   -57.9,  -0.1,   0.7 },
      {  7,  2,     -4.7,   -21.1,  -0.6,   0.3 },
      {  7,  3,     45.3,     6.5,   1.3,  -0.1 },
      {  7,  4,     13.9,    24.9,   0.4,  -0.1 },
      {  7,  5,     10.4,     7.0,   0.3,  -0.8 },
      {  7,  6,      1.7,   -27.7,  -0.7,  -0.3 },
      {  7,  7,      4.9,    -3.3,   0.6,   0.3 },
      {  8,  0,     24.4,     0.0,  -0.1,   0.0 },
      {  8,  1,      8.1,    11.0,   0.1,  -0.1 },
      {  8,  2,    -14.5,   -20.0,  -0.6,   0.2 },
      {  8,  3,     -5.6,    11.9,   0.2,   0.4 },
      {  8,  4,    -19.3,   -17.4,  -0.2,   0.4 },
      {  8,  5,     11.5,    16.7,   0.3,   0.1 },
      {  8,  6,     10.9,     7.0,   0.3,  -0.1 },
      {  8,  7,    -14.1,   -10.8,  -0.6,   0.4 },
      {  8,  8,     -3.7,     1.7,   0.2,   0.3 },
      {  9,  0,      5.4,     0.0,   0.0,   0.0 },
      {  9,  1,      9.4,   -20.5,  -0.1,   0.0 },
      {  9,  2,      3.4,    11.5,   0.0,  -0.2 },
      {  9,  3,     -5.2,    12.8,   0.3,   0.0 },
      {  9,  4,      3.1,    -7.2,  -0.4,  -0.1 },
      {  9,  5,    -12.4,    -7.4,  -0.3,   0.1 },
      {  9,  6,     -0.7,     8.0,   0.1,   0.0 },
      {  9,  7,      8.4,     2.1,  -0.1,  -0.2 },
      {  9,  8,     -8.5,    -6.1,  -0.4,   0.3 },
      {  9,  9,    -10.1,     7.0,  -0.2,   0.2 },
      { 10,  0,     -2.0,     0.0,   0.0,   0.0 },
      { 10,  1,     -6.3,     2.8,   0.0,   0.1 },
      { 10,  2,      0.9,    -0.1,  -0.1,  -0.1 },
      { 10,  3,     -1.1,     4.7,   0.2,   0.0 },
      { 10,  4,     -0.2,     4.4,   0.0,  -0.1 },
      { 10,  5,      2.5,    -7.2,  -0.1,  -0.1 },
      { 10,  6,     -0.3,    -1.0,  -0.2,   0.0 },
      { 10,  7,      2.2,    -3.9,   0.0,  -0.1 },
      { 10,  8,      3.1,    -2.0,  -0.1,  -0.2 },
      { 10,  9,     -1.0,    -2.0,  -0.2,   0.0 },
      { 10, 10,     -2.8,    -8.3,  -0.2,  -0.1 },
      { 11,  0,      3.0,     0.0,   0.0,   0.0 },
      { 11,  1,     -1.5,     0.2,   0.0,   0.0 },
      { 11,  2,     -2.1,     1.7,   0.0,   0.1 },
      { 11,  3,      1.7,    -0.6,   0.1,   0.0 },
      { 11,  4,     -0.5,    -1.8,   0.0,   0.1 },
      { 11,  5,      0.5,     0.9,   0.0,   0.0 },
      { 11,  6,     -0.8,    -0.4,   0.0,   0.1 },
      { 11,  7,      0.4,    -2.5,   0.0,   0.0 },
      { 11,  8,      1.8,    -1.3,   0.0,  -0.1 },
      { 11,  9,      0.1,    -2.1,   0.0,  -0.1 },
      { 11, 10,      0.7,    -1.9,  -0.1,   0.0 },
      { 11, 11,      3.8,    -1.8,   0.0,  -0.1 },
      { 12,  0,     -2.2,     0.0,   0.0,   0.0 },
      { 12,  1,     -0.2,    -0.9,   0.0,   0.0 },
      { 12,  2,      0.3,     0.3,   0.1,   0.0 },
      { 12,  3,      1.0,     2.1,   0.1,   0.0 },
      { 12,  4,     -0.6,    -2.5,  -0.1,   0.0 },
      { 12,  5,      0.9,     0.5,   0.0,   0.0 },
      { 12,  6,     -0.1,     0.6,   0.0,   0.1 },
      { 12,  7,      0.5,     0.0,   0.0,   0.0 },
      { 12,  8,     -0.4,     0.1,   0.0,   0.0 },
      { 12,  9,     -0.4,     0.3,   0.0,   0.0 },
      { 12, 10,      0.2,    -0.9,   0.0,   0.0 },
      { 12, 11,     -0.8,    -0.2,  -0.1,   0.0 },
      { 12, 12,      0.0,     0.9,   0.1,   0.0 }
    };


    // Разобранные коэффициенты для рассчета магнитного склонения -->
      /**
      * Мьютекс для момента инициализации коэффициентов для рассчета магнитного склонения.
      */
      static Mutex magnetic_declination_mutex;
      /**
      * Флаг инициализации коэффициентов  для рассчета магнитного склонения.
      */
      static int magnetic_declination_initialized = 0;
      /**
      * Gauss main coefficients at time t0.
      */
      static double[,] g_t0;
      /**
      * Gauss main coefficients at time t0.
      */
      static double[,] h_t0;
      /**
      * Gauss secular variation coefficients at time t0.
      */
      static double[,] dg_t0;
      /**
      * Gauss secular variation coefficients at time t0.
      */
      static double[,] dh_t0;
      /**
      * Some equations with sqrt.
      */
      static double[] root;
      /**
      * Some equations with sqrt.
      */
      static double[,,] roots;
      /**
      * Base date of the magnetic model.
      */
      static Date base_date;
    // Разобранные коэффициенты для рассчета магнитного склонения <--


    /**
     * Магнитное склонение (точка уже должна быть в системе координат WGS_84).
     *
     * Алгоритм расчета взят из "The US/UK World Magnetic Model for 2010-2015",
     * www.ngdc.noaa.gov/geomag/WMM/.
     */
    public double get_magnetic_declination(GLib.Date date)
      requires(this.type == CoordSysType.WGS_84)
      requires(date.valid() == true)
    {
      if(AtomicInt.compare_and_exchange(ref Gp.Point.magnetic_declination_initialized, 0, 1))
      {
        Gp.Point.magnetic_declination_mutex.lock();

        Gp.Point.g_t0 = new double[13,13];
        Gp.Point.h_t0 = new double[13,13];
        Gp.Point.dg_t0 = new double[13,13];
        Gp.Point.dh_t0 = new double[13,13];
        Gp.Point.root = new double[13];
        Gp.Point.roots = new double[13,13,2];

        int n, m;
        int i = 0;

        base_date.clear(1);
        base_date.set_dmy(1, GLib.DateMonth.JANUARY, 2010 );

        for ( n = 1; n <= 12; n++ )
           for ( m = 0; m <= n; m++, i++ )
           {
              GLib.assert( i < Gp.Point.WMM_2010.length );
              GLib.assert(n == Gp.Point.WMM_2010[i].n && m == Gp.Point.WMM_2010[i].m);

              g_t0[n,m] = Gp.Point.WMM_2010[i].g_t0;
              h_t0[n,m] = Gp.Point.WMM_2010[i].h_t0;
              dg_t0[n,m] = Gp.Point.WMM_2010[i].dg_t0;
              dh_t0[n,m] = Gp.Point.WMM_2010[i].dh_t0;
           }

        for( n = 2; n <= 12; n++)
           root[n] = Math.sqrt( ( 2.0 * n - 1 ) / ( 2.0 * n ) );

        for ( m = 0; m <= 12; m++ )
           for ( n = int.max(m + 1, 2); n <= 12; n++ )
           {
              roots[m,n,0] = Math.sqrt( ( n - 1 ) * ( n - 1 ) - m * m );
              roots[m,n,1] = 1.0 / Math.sqrt( n * n - m * m );
           }

        Gp.Point.magnetic_declination_mutex.unlock();
      }

      int n, m;

      int days; // days count between the base and and the given date

      double[,] g = new double[13,13]; // Gauss coefficients at given time
      double[,] h = new double[13,13]; // Gauss coefficients at given time
      double[,] P = new double[13,13]; // Schmidt semi-normalized associated Legendre functions values at sin( fi_ )
      double[,] dP = new double[13,13]; // dP/dfi_

      double a = 6371200; // geomagnetic reference radius, m
      double A = 6378137; // semi-major axis, m
      double f = 1.0 / 298.257223563; // 1 / ( reciprocal flattening )
      double e2; // squared eccentricity
      double Rc; // radius of East-West curvature, m

      double fi_; // geocentric spherical latitude, rad
      double r; // geocentric spherical radius, m
      double p, z; // p = sqrt( x*x + y*y ) and z are the coordinates of a geocentric Cartesian coordinate system in which the positive x and z axes point in the directions of the prime meridian (λ=0) and the Earth’s rotation axis, respectively, m

      double X_, Z_, Y_; // field components in geocentric coordinates, nT

      double X; // North field component, nT
      double Y; // East field component, nT

      double D; // magnetic declination, rad

      double sin_fi_; // sin( fi_ )
      double cos_fi_; // cos( fi_ )
      double sin_mlam[13]; // sin( m * this.x )
      double cos_mlam[13]; // cos( m * this.x )

      /*
       * Compute Gauss coefficients at given time
       */

      days = (int)date.get_julian() - (int)base_date.get_julian();

      for ( n = 0; n <= 12; n++ )
         for ( m = 0; m <= 12; m++ )
         {
            g[n,m] = g_t0[n,m] + (double)days / 365.25 * dg_t0[n,m];
            h[n,m] = h_t0[n,m] + (double)days / 365.25 * dh_t0[n,m];
         }

      /*
       * Compute geocentric coordinates
       */

      e2 = f * ( 2 - f );
      Rc = A / Math.sqrt( 1 - e2 * Math.sin( this.y ) * Math.sin( this.y ) );

      p = ( Rc + this.h ) * Math.cos( this.y );
      z = ( Rc * ( 1 - e2 ) + this.h ) * Math.sin( this.y );
      r = Math.sqrt( p * p + z * z );
      fi_ = Math.asin( z / r );

      /*
       *
       */

      sin_fi_ = Math.sin( fi_ );
      cos_fi_ = Math.cos( fi_ );

      for ( m = 0; m <= 12; m++ )
      {
         sin_mlam[m] = Math.sin( m * this.x );
         cos_mlam[m] = Math.cos( m * this.x );
      }

      /*
       * Compute spherical functions
       */

      // Diagonal elements

      P[0,0] = 1;
      P[1,1] = cos_fi_;
      dP[0,0] = 0;
      dP[1,1] = -sin_fi_;
      P[1,0] = sin_fi_;
      dP[1,0] = cos_fi_;

      for ( n = 2; n <= 12; n++ )
      {
         P[n,n] = P[n - 1,n - 1] * cos_fi_ * root[n];
         dP[n,n] = ( dP[n - 1,n - 1] * cos_fi_ - P[n - 1,n - 1] * sin_fi_ ) * root[n];
      }

      // Lower triangle elements

      for ( m = 0; m <= 12; m++ )
         for ( n = int.max( m + 1, 2 ); n <= 12; n++ )
         {
            P[n,m] = ( P[n - 1,m] * sin_fi_ * ( 2.0 * n - 1 ) - P[n - 2,m] * roots[m,n,0] ) * roots[m,n,1];
            dP[n,m] = ( ( dP[n - 1,m] * sin_fi_ + P[n - 1,m] * cos_fi_ ) * ( 2.0 * n - 1 )
                  - dP[n - 2,m] * roots[m,n,0] ) * roots[m,n,1];
         }

      /*
       * Compute geocentric field
       */

      X_ = 0;
      Y_ = 0;
      Z_ = 0;

      for ( n = 1; n <= 12; n++ )
      {
         double sum_x = 0;
         double sum_y = 0;
         double sum_z = 0;
         double ar;

         for ( m = 0; m <= n; m++ )
         {
            sum_x += ( g[n,m] * cos_mlam[m] + h[n,m] * sin_mlam[m] ) * dP[n,m];

            sum_y += m * ( g[n,m] * sin_mlam[m] - h[n,m] * cos_mlam[m] ) * P[n,m];

            sum_z += ( g[n,m] * cos_mlam[m] + h[n,m] * sin_mlam[m] ) * P[n,m];
         }

         ar = Math.pow( a / r, n + 2 );

         X_ += ar * sum_x;
         Y_ += ar * sum_y;
         Z_ += ( n + 1 ) * ar * sum_z;
      }

      X_ *= -1;
      Y_ /= cos_fi_;
      Z_ *= -1;

      /*
       * Compute geodetic field
       */

      X = X_ * Math.cos( fi_ - this.y ) - Z_ * Math.sin( fi_ - this.y );
      Y = Y_;

      /*
       * Compute declination
       */

      D = Math.atan2( Y, X );

      return D;
    }


    /**
    * Параметры общеземного эллипсоида.
    */
    private static bool get_ell_params (CoordSysType type, ref EllParams eps)
    {
      const double ell_p[18] = {
        6378137, 6356752.314, 6399593.626, 1/298.257223563, 0.006694379990, 0.006739496742, //wgs-84 (g1150)
        6378136, 6356751.362, 6399592.578, 1/298.25784, 0.006694366177, 0.006739482743,     //пз-90.11
        6378245, 6356863.019, 6399698.902, 1/298.3, 0.006693421623, 0.006738525415 //ск-42, элл-д Красовского
      };

      if (type == CoordSysType.NULL || type >= CoordSysType.GEO_MAX) return false;

      //int i = 6 * (type - 1);
      int i;
      if (type == CoordSysType.WGS_84) i = 0;
      else if (type == CoordSysType.PZ_90) i = 6;
      else if (type == CoordSysType.CK_42) i = 12;
      else return false;

      eps.ax_a  = ell_p[i];    //большая полуось общеземного эллипсоида (м)
      eps.ax_b  = ell_p[i+1];  //малая полуось (м)
      eps.ax_c  = ell_p[i+2];  //полярный радиус (м)
      eps.alpha = ell_p[i+3];  //сжатие общеземного эллипсоида
      eps.e2    = ell_p[i+4];  //квадрат эксцентриситета
      eps.es2   = ell_p[i+5];  //квадрат второго эксцентриситета

      return true;
    }


    /**
    * Коэффициенты пересчета между значениями широты и длины дуги по меридиану.
    */
    private static bool get_arc_params (CoordSysType type, ref ArcParams aps)
    {
      const double arc_p[21] = {
        6367449.1458, 16038.5086, 16.8326, 0.0220, 25188265.8e-10, 37009.6e-10, 74.5e-10, //wgs-84 (g1150)
        6367448.1695, 16038.4730, 16.8325, 0.0220, 25188213.6e-10, 37009.4e-10, 74.5e-10, //пз-90.11
        6367558.4968, 16036.4802, 16.8281, 0.0220, 25184647.7e-10, 36998.9e-10, 74.4e-10  //ск-42, элл-д Красовского
      };

      if (type == CoordSysType.NULL || type >= CoordSysType.GEO_MAX) return false;

      //int i = 7 * (type - 1);
      int i;
      if (type == CoordSysType.WGS_84) i = 0;
      else if (type == CoordSysType.PZ_90) i = 7;
      else if (type == CoordSysType.CK_42) i = 14;
      else return false;

      aps.c0 = arc_p[i];    //коэффициенты вычисления длин дуг меридианов
      aps.c2 = arc_p[i+1];
      aps.c4 = arc_p[i+2];
      aps.c6 = arc_p[i+3];
      aps.d2 = arc_p[i+4];  //коэффициенты вычисления широт по длинам дуг меридианов
      aps.d4 = arc_p[i+5];
      aps.d6 = arc_p[i+6];

      return true;
    }


    /**
    * Параметры трансформирования систем координат при пересчете между эллипсоидами.
    */
    private static bool get_geo_trans_params (GeoTransType type, ref GeoTransParams gtp)
    {
      const double geo_k[21] = {
         -0.008e-6, -0.013, 0.106, 0.022, -0.0023, 0.00354, -0.00421,  // wgs-84 -> пз-90.11
         0.220e-6, -23.57, 140.95, 79.8, 0.0, 0.350, 0.790,            // wgs-84 -> ск-42
         0.228e-6, -23.557, 140.844, 79.778, 0.0023, 0.34646, 0.79421  // пз-90.11 -> ск-42
      };

      if (type >= GeoTransType.MAX) return false;

      int i = 7 * type;
      gtp.m  = geo_k[i];    //масштабный коэфф-т
      gtp.dx = geo_k[i+1];  //линейные элементы (м)
      gtp.dy = geo_k[i+2];
      gtp.dz = geo_k[i+3];
      gtp.wx = geo_k[i+4];  //угловые элементы (угл.сек.)
      gtp.wy = geo_k[i+5];
      gtp.wz = geo_k[i+6];

      return true;
    }


    /**
    * Относительные параметры эллипсоидов при трансформировании геодезических координат.
    */
    private static bool get_ell_trans_params (GeoTransType type, ref EllTransParams etp)
    {
      if (type >= GeoTransType.MAX) return false;

      CoordSysType coord_sys_1 = CoordSysType.WGS_84, coord_sys_2 = CoordSysType.CK_42;

      if (type == GeoTransType.WGS_TO_PZ) coord_sys_2 = CoordSysType.PZ_90;
        else if (type == GeoTransType.PZ_TO_CK) coord_sys_1 = CoordSysType.PZ_90;

      EllParams eps = {0};
      if (Gp.Point.get_ell_params( coord_sys_2, ref eps) != true) return false;

      double a = eps.ax_a;
      double e2 = eps.e2;

      if (Gp.Point.get_ell_params( coord_sys_1, ref eps) != true) return false;

      etp.da = a - eps.ax_a;
      etp.ha = (a + eps.ax_a) / 2;
      etp.de2 = e2 - eps.e2;
      etp.he2 = (e2 + eps.e2) / 2;

      return true;
    }


    /**
    * Задать параметры точки.
    *
    * В формате:
    * {{{UTM, номер_зоны (1-60), northing (м), easting (м), высота (м)
    * WGS-84 / ПЗ-90, 0 или номер_зоны (1-60), широта (рад), долгота (рад), высота (м)
    * WGS-84 / ПЗ-90, 0 или номер_зоны (1-60), широта (гр), долгота (гр), высота (м), 1}}}
    */
    public bool set_point (CoordSysType type, uint zone, double b, double l, double h, int grad=0)
    {
      if (type == CoordSysType.NULL || type == CoordSysType.GEO_MAX || type >= CoordSysType.MAX)
      {
        warning("Point init coordinates: unknown type.\n");
        return false;
      }

      if (zone > 60 || (zone == 0 && type > CoordSysType.GEO_MAX))
      {
        warning("Point init coordinates: unknown UTM zone number.\n");
        return false;
      }

      if (type < CoordSysType.GEO_MAX && grad != 0)
      {
        l *= GEO_DEG_TO_RAD;
        b *= GEO_DEG_TO_RAD;
      }

      this.type = type;
      this.zone = zone;
      this.y = b;
      this.x = l;
      this.h = h;

      if (this.type < CoordSysType.GEO_MAX)
      {
        if (this.y <= -90.0 * GEO_DEG_TO_RAD || this.y >= 90.0 * GEO_DEG_TO_RAD)
        {
          warning("Point init coordinates: latitude is out of range (-90° - +90°).\n");
          return false;
        }

        if (this.x < -180.0 * GEO_DEG_TO_RAD || this.x > 180.0 * GEO_DEG_TO_RAD)
        {
          warning("Point init coordinates: longitude is out of range (-180° - +180°).\n");
          return false;
        }
      }

      return true;
    }


    /**
    * Вывод на печать параметров точки.
    *
    * В формате:
    * {{{UTM / GK: зона, northing (м), easting (м), высота (м)}}}
    * или
    * {{{WGS-84 / ПЗ-90: 0, широта (гр°мин'сек"), долгота (гр°мин'сек"), высота (м)}}}
    */
    public bool get_point()
    {
      if (this.type == CoordSysType.NULL || this.type == CoordSysType.GEO_MAX || this.type >= CoordSysType.MAX)
        return false;

      string str = this.type.to_string();
      int n = str.length;
      print("%s: зона=%d, ", str[18:n], (int)this.zone);

      if (this.type > CoordSysType.GEO_MAX)
      {
        print("N=%lfм, E=%lfм, H=%lfм\n", this.y, this.x, this.h);
        return true;
      }

      bool ng;
      int deg, min;
      double sec;

      print("B=");
      Gp.Point.geo_dms( GEO_RAD_TO_DEG * this.y, out ng, out deg, out min, out sec);
      if (ng == true) stdout.printf("-");
      print("%.2d:%.2d:%09.6lf, L=", deg, min, sec);

      Gp.Point.geo_dms( GEO_RAD_TO_DEG * this.x, out ng, out deg, out min, out sec);
      if (ng == true) stdout.printf("-");
      print("%.2d:%.2d:%09.6lf, ", deg, min, sec);
      print("H=%lfм\n", this.h);

      return true;
    }


    /**
    * Строковое представление значения угла (рад).
    *
    * В формате: {{{гр°мин'сек" <полушарие>}}}
    * полушарие: =1 - N/S, =2 - W/E
    */
    public string angle_to_string (double angle, int hs = 0)
    {
      bool ng;
      int deg, min;
      double sec;
      string s = "\0";

      if (Gp.Point.geo_dms( angle * GEO_RAD_TO_DEG, out ng, out deg, out min, out sec) == false) return s;

      if (hs == 0 && angle < 0) s = "-";
      s += deg.to_string("%02d") + "°";
      s += min.to_string("%02d") + "'";

      //char[] str = new char[6];
      //s += sec.format(str,"%05.2lf") + "\"";

      int n = (int)sec;
      s += n.to_string("%02d") + ".";
      double d = sec - n;
      d *= 100;
      n = (int)d;
      s += n.to_string("%02d") + "\"";

      if (hs == 0) return s;

      if (hs == 1)
      {
        if (angle < 0) s += " S";
          else s += " N";
      }
      if (hs == 2)
      {
        if (angle < 0) s += " W";
          else s += " E";
      }

      return s;
    }


    /**
    * Строковое представление координат.
    *
    * В виде строки "широта;долгота;высота" (рад/рад/м).
    */
    public string to_string()
    {
      return this.y.to_string() + ";" + this.x.to_string() + ";" + this.h.to_string();
    }


    /**
    * Общая функция преобразования координат.
    */
    public bool convert (CoordSysType dtype)
    {
      if (dtype == CoordSysType.NULL || dtype == CoordSysType.GEO_MAX || dtype >= CoordSysType.MAX)
      {
        warning("Point.convert destination coordinates: unknown type.\n");
        return false;
      }

      if (this.type == CoordSysType.NULL || this.type == CoordSysType.GEO_MAX || this.type >= CoordSysType.MAX)
      {
        warning("Point.convert actual coordinates: unknown type.\n");
        return false;
      }

      if (this.type == dtype) return true;

      bool b_wgs = false, b_ck = false, b = true;

      if (this.type == CoordSysType.UTM) b = this.utm_to_wgs84();
      else if (this.type == CoordSysType.GK) b = this.gauss_to_geo( CoordSysType.WGS_84);
      else if (this.type == CoordSysType.CK_42_GK) b = this.gauss_to_geo( CoordSysType.CK_42);

      if (b != true || this.type == dtype) return b;

      if (dtype == CoordSysType.WGS_84 || dtype == CoordSysType.UTM || dtype == CoordSysType.GK) b_wgs = true;
      if (dtype == CoordSysType.CK_42 || dtype == CoordSysType.CK_42_GK) b_ck = true;

      switch (this.type)
      {
        case CoordSysType.WGS_84:
          if (dtype == CoordSysType.PZ_90)
            b = this.geo1_to_geo2( GeoTransType.WGS_TO_PZ);
          else if (b_ck == true)
            b = this.geo1_to_geo2( GeoTransType.WGS_TO_CK);
        break;

        case CoordSysType.PZ_90:
          if (b_wgs == true)
            b = this.geo2_to_geo1( GeoTransType.WGS_TO_PZ);
          else if (b_ck == true)
            b = this.geo1_to_geo2( GeoTransType.PZ_TO_CK);
        break;

        case CoordSysType.CK_42:
          if (b_wgs == true)
            b = this.geo2_to_geo1( GeoTransType.WGS_TO_CK);
          else if (dtype == CoordSysType.PZ_90)
            b = this.geo2_to_geo1( GeoTransType.PZ_TO_CK);
        break;
      }

      if (b == false || this.type == dtype) return b;

      if (dtype == CoordSysType.UTM) b = this.wgs84_to_utm();
      else if (dtype == CoordSysType.GK) b = this.geo_to_gauss( CoordSysType.WGS_84);
      else if (dtype == CoordSysType.CK_42_GK) b = this.geo_to_gauss( CoordSysType.CK_42);

      return b;
    }


    /**
    * Преобразование UTM в WGS-84.
    */
    private bool utm_to_wgs84()
    {
      double dx = (this.x - UTM_L0) / UTM_M0;
      double dy = this.y / UTM_M0;

      double dl, b;
      Gp.Point.gauss_to_geo_transform( CoordSysType.WGS_84, dx, dy, out dl, out b);

      this.x = dl + (6 * ((int)this.zone - 30) - 3) * GEO_DEG_TO_RAD;
      this.y = b;
      if (dl >= -6 * GEO_DEG_TO_RAD && dl <= 6 * GEO_DEG_TO_RAD) this.zone = 0;
      this.type = CoordSysType.WGS_84;

      return true;
    }


    /**
    * Преобразование WGS-84 в UTM.
    */
    private bool wgs84_to_utm()
    {
      double angle = this.y * GEO_RAD_TO_DEG;
      if ((angle > 0 && angle > UTM_NORTH) ||
          (angle < 0 && -angle > UTM_SOUTH)) return false;

      uint zone = get_zone( this.x);
      if (zone == 0 || zone > 60) return false;

      if (this.zone == 0) this.zone = zone;  // опорная зона не задана
      else  // предустановлена опорная зона, проверяем насколько она отличается от реальной
      {
        int dz = (int)this.zone - (int)zone;
        if (this.zone > 60 || (dz != -59 && dz != -1 && dz != 0 && dz != 1 && dz != 59))
        {
          warning("wgs84_to_utm coordinates: real zone = %d, requested zone = %d", (int)zone, (int)this.zone);
          this.zone = zone;
        }
      }

      double dl = this.x - (6 * ((int)this.zone - 30) - 3) * GEO_DEG_TO_RAD;
      double dx, dy;
      Gp.Point.geo_to_gauss_transform( CoordSysType.WGS_84, dl, this.y, out dx, out dy);

      this.x = UTM_M0 * dx + UTM_L0;  //Вариант отображения this.x += Z0_UTM * zone;
      this.y = dy * UTM_M0;           //Для отображения в южном полушарии this.y = N0_UTM + dy * M0_UTM;
      //this.zone = zone;
      this.type = CoordSysType.UTM;

      return true;
    }


    /**
    * Сворачивание проекции Гаусса-Крюгера в WGS-84 или СК-42.
    */
    private bool gauss_to_geo( CoordSysType type)
    {
      double dl, b;
      Gp.Point.gauss_to_geo_transform( type, this.x - UTM_L0, this.y, out dl, out b);

      this.x = dl + (6 * (int)this.zone - 3) * GEO_DEG_TO_RAD;
      if (this.zone > 30) this.x -= 360 * GEO_DEG_TO_RAD;

      this.y = b;
      if (dl >= -6 * GEO_DEG_TO_RAD && dl <= 6 * GEO_DEG_TO_RAD) this.zone = 0;
      this.type = type;

      return true;
    }


    /**
    * Развертывание WGS-84 или СК-42 в проекцию Гаусса-Крюгера.
    */
    private bool geo_to_gauss( CoordSysType type)
    {
      double angle = this.y * GEO_RAD_TO_DEG;
      if ((angle > 0 && angle > UTM_NORTH) ||
          (angle < 0 && -angle > UTM_SOUTH)) return false;

      bool ng;
      int deg, min;
      double sec;

      if (Gp.Point.geo_dms (this.x * GEO_RAD_TO_DEG, out ng, out deg, out min, out sec) == false) return false;

      if (ng == true && deg != 180) deg = 359 - deg;
      uint zone = (deg / 6) + 1;
      if (ng == false && deg == 180) zone -= 1;
      if (zone == 0 || zone > 60) return false;

      if (this.zone == 0) this.zone = zone;  // опорная зона не задана
      else  // предустановлена опорная зона, проверяем насколько она отличается от реальной
      {
        int dz = (int)this.zone - (int)zone;
        if (this.zone > 60 || (dz != -59 && dz != -1 && dz != 0 && dz != 1 && dz != 59))
        {
          warning("geo_to_gauss coordinates: real zone = %d, requested zone = %d", (int)zone, (int)this.zone);
          this.zone = zone;
        }
      }

      double dl = this.x - (6 * (int)this.zone - 3) * GEO_DEG_TO_RAD;
      if (zone > 30) dl += 360 * GEO_DEG_TO_RAD;

      double dx, dy;
      Gp.Point.geo_to_gauss_transform( type, dl, this.y, out dx, out dy);

      this.x = dx + UTM_L0;
      this.y = dy;
      //this.zone = zone;

      if (type == CoordSysType.WGS_84) this.type = CoordSysType.GK;
        else if (type == CoordSysType.CK_42) this.type = CoordSysType.CK_42_GK;

      return true;
    }


    /**
    * Преобразование прямоугольных координат из проекции Гаусса-Крюгера в геодезические.
    */
    private static bool gauss_to_geo_transform (CoordSysType type, double dx, double dy,
                                                            out double geo_dl, out double geo_b)
    {
      geo_dl = 0;
      geo_b = 0;

      // Широта b_y соответствует длине дуги dy по осевому меридиану от экватора
      double b_y = arc_to_lat( type, dy);

      double cos_b = Math.cos(b_y);
      double sin_b = Math.sin(b_y);
      double tan_b = Math.tan(b_y);
      double tan2_b = tan_b * tan_b;

      EllParams eps = {0};
      if (Gp.Point.get_ell_params( type, ref eps) != true) return false;

      double t = Math.sqrt(1 - eps.e2 * sin_b * sin_b);
      double m = eps.ax_a * (1 - eps.e2) / (t * t * t);
      double n = eps.ax_a / t;
      double nu2 = eps.es2 * cos_b * cos_b;

      // Поправка к долготе
      t = dx * dx / (n * n);
      double s = 1 - (t * (1 + 2 * tan2_b + nu2) / 6) +
            (t * t * (5 + 28 * tan2_b + 24 * tan2_b * tan2_b + 6 * nu2 + 8 * nu2 * tan2_b) / 120);
      geo_dl = s * dx / (n * cos_b);

      // Широта
      s = 1 - (t * (5 + 3 * tan2_b + nu2 - 9 * nu2 * tan2_b) / 12) +
          (t * t * (61 + 90 * tan2_b + 45 * tan2_b * tan2_b) / 360);
      geo_b = b_y - (s * dx * dx * tan_b / (2 * m * n));

      return true;
    }


    /**
    * Преобразование геодезических координат в прямоугольные в проекции Гаусса-Крюгера.
    */
    private static bool geo_to_gauss_transform (CoordSysType type, double geo_dl, double geo_b,
                                                            out double dx, out double dy)
    {
      dx = 0;
      dy = 0;

      double cos_b = Math.cos(geo_b);
      double sin_b = Math.sin(geo_b);
      double tan2_b = Math.tan(geo_b);
      tan2_b *= tan2_b;

      EllParams eps = {0};
      if (Gp.Point.get_ell_params( type, ref eps) != true) return false;

      double n = eps.ax_a / Math.sqrt(1 - eps.e2 * sin_b * sin_b);
      double nu2 = eps.es2 * cos_b * cos_b;

      // Смещение относительно осевого меридиана зоны
      double t = geo_dl * geo_dl * cos_b * cos_b;
      double s = 1 + (t * (1 - tan2_b + nu2) / 6) +
                 (t * t * (5 - 18 * tan2_b - tan2_b * tan2_b + 14 * nu2 - 58 * nu2 * tan2_b) / 120);
      dx = geo_dl * n * cos_b * s;

      // Длина дуги меридиана от экватора до параллели под широтой geo_b
      double s_b = lat_to_arc( type, geo_b);

      // Смещение относительно экватора
      s = 0.5 + (t * (5 - tan2_b + 9 * nu2 + 4 * nu2 * nu2) / 24) +
          (t * t * (61 - 58 * tan2_b + tan2_b * tan2_b + 270 * nu2 - 330 * nu2 * tan2_b) / 720);
      dy = s_b + s * geo_dl * geo_dl * n * sin_b * cos_b;

      return true;
    }


    /**
    * Получить номер зоны UTM по долготе (рад).
    */
    private static uint get_zone (double longitude)
    {
      bool ng;
      int deg, min;
      double sec;
      uint z;

      if (Gp.Point.geo_dms (longitude * GEO_RAD_TO_DEG, out ng, out deg, out min, out sec) == false) return 0;

      if (ng == true)
        z = (179 - deg) / 6 + 1;
      else if (deg == 180)
        z = 60;
      else
        z = (180 + deg) / 6 + 1;

      return z;
    }


    /**
    * Разложить угол (град) на градусы, минуты, секунды.
    */
    public static bool geo_dms(double angle, out bool negative, out int deg, out int min, out double sec)
    {
      if (angle < 0)
      {
        negative = true;
        angle = -angle;
      }
      else
        negative = false;

      deg = (int)angle;
      double frac = angle - deg;
      frac *= 60;
      min = (int)frac;

      frac = frac - (double)min;
      sec = Math.nearbyint (frac * 600000.0);
      sec /= 10000.0;

      if (sec >= 60.0)
      {
        min += 1;
        sec -= 60.0;
      }
      if (min >= 60)
      {
        deg += 1;
        min -= 60;
      }

      return true;
    }


    /**
    * Преобразование геодезических координат из системы 1 в систему 2 (GeoTransType 1->2).
    */
    private bool geo1_to_geo2 (GeoTransType type)
    {
      double dx, dy, dh;

      if (type >= GeoTransType.MAX) return false;

      Gp.Point.geodetic_transform (type, this.x, this.y, this.h, out dx, out dy, out dh);
      Gp.Point.geodetic_transform (type, this.x + dx/2, this.y + dy/2, this.h + dh/2, out dx, out dy, out dh);

      this.x += dx;
      this.y += dy;
      this.h += dh;

      if (type == GeoTransType.WGS_TO_PZ) this.type = CoordSysType.PZ_90;
        else this.type = CoordSysType.CK_42;

      return true;
    }


    /**
    * Преобразование геодезических координат из системы 2 в систему 1 (GeoTransType 1->2).
    */
    private bool geo2_to_geo1 (GeoTransType type)
    {
      double dx, dy, dh;

      if (type >= GeoTransType.MAX) return false;

      Gp.Point.geodetic_transform (type, this.x, this.y, this.h, out dx, out dy, out dh);
      Gp.Point.geodetic_transform (type, this.x - dx/2, this.y - dy/2, this.h - dh/2, out dx, out dy, out dh);

      this.x -= dx;
      this.y -= dy;
      this.h -= dh;

      if (type == GeoTransType.PZ_TO_CK) this.type = CoordSysType.PZ_90;
        else this.type = CoordSysType.WGS_84;

      return true;
    }


    /**
    * Поправки для взаимных преобразований геодезических координат WGS-84 и ПЗ-90.
    */
    private static bool geodetic_transform (GeoTransType type,
                                     double coord_x, double coord_y, double coord_h,
                                     out double dx, out double dy, out double dh)
    {
      dx = 0;
      dy = 0;
      dh = 0;

      GeoTransParams gtp = {0};
      if (Gp.Point.get_geo_trans_params( type, ref gtp) != true) return false;

      EllTransParams etp = {0};
      if (Gp.Point.get_ell_trans_params( type, ref etp) != true) return false;

      double sin_b = Math.sin( coord_y);
      double cos_b = Math.cos( coord_y);
      double sin_l = Math.sin( coord_x);
      double cos_l = Math.cos( coord_x);

      double s = Math.sqrt(1 - etp.he2 * sin_b * sin_b);
      double m = etp.ha * (1 - etp.he2) / (s * s * s);
      double n = etp.ha / s;

      // Поправка к широте
      s = (n * etp.he2 * sin_b * cos_b * etp.da / etp.ha) +
          ((1 + n * n / etp.ha / etp.ha) * n * sin_b * cos_b * etp.de2 / 2) -
          ((gtp.dx * cos_l + gtp.dy * sin_l) * sin_b) + (gtp.dz * cos_b);
      s /= m + coord_h;
      s += ((1 + etp.he2 * Math.cos(2 * coord_y)) * (gtp.wy * cos_l - gtp.wx * sin_l)) / GEO_RAD_TO_SEC;
      s -= gtp.m * etp.he2 * sin_b * cos_b;
      dy = s;

      // Поправка к долготе
      s = (gtp.dy * cos_l - gtp.dx * sin_l) / (n + coord_h) / cos_b;
      s += ((gtp.wx * cos_l + gtp.wy * sin_l) * (1 - etp.he2) * Math.tan(coord_y) - gtp.wz) / GEO_RAD_TO_SEC;
      dx = s;

      // Поправка к высоте
      s = ((etp.ha * etp.ha / n + coord_h) * gtp.m) - (etp.ha * etp.da / n) + (n * sin_b * sin_b * etp.de2 / 2) +
          ((gtp.dx * cos_l + gtp.dy * sin_l) * cos_b) + (gtp.dz * sin_b);
      s -= n * etp.he2 * sin_b * cos_b * (gtp.wx * sin_l - gtp.wy * cos_l) / GEO_RAD_TO_SEC;
      dh = s;

      return true;
    }


    /**
    * Прямая геодезическая задача.
    *
    * Даны координаты точки А, азимут на точку В и расстояние до нее;
    * найти координаты точки В и обратный азимут в точке B.
    * Метод преобразовывает координаты исходной точки по заданному азимуту и расстоянию.
    */
    public bool move_to (double s, double a1, out double a2)
    {
      double a=0, b=0, l=0;
      double am, bm, m, n, t, sin_bm, nm2;
      double beta, delta, alpha;

      a2 = 0;
      bool bl = true;
      CoordSysType type = this.type;

      if (type == CoordSysType.NULL || type == CoordSysType.GEO_MAX || type >= CoordSysType.MAX)
        return false;

      if (type == CoordSysType.UTM || type == CoordSysType.GK) bl = this.convert(CoordSysType.WGS_84);
        else if (type == CoordSysType.CK_42_GK) bl = this.convert(CoordSysType.CK_42);
      if (bl != true) return false;

      EllParams eps = {0};
      if (Gp.Point.get_ell_params( this.type, ref eps) != true) return false;


      t = s / GEO_RE;
      l = Math.atan( Math.sin(t) * Math.sin(a1) /
          (Math.cos(t) * Math.cos(this.y) - Math.sin(t) * Math.sin(this.y) * Math.cos(a1)) );

      for (int i=0;i<8;i++)
      {
        am = a1 + a/2;
        bm = this.y + b/2;

        t = Math.cos(bm);
        nm2 = eps.es2 * t * t;

        sin_bm = Math.sin(bm);
        t = Math.sqrt(1 - eps.e2 * sin_bm * sin_bm);
        m = eps.ax_a * (1 - eps.e2) / (t * t * t);
        n = eps.ax_a / t;

        beta = s * Math.cos(am) / m;
        delta = s * Math.sin(am) / (n * Math.cos(bm));
        alpha = l * sin_bm;

        m = alpha * alpha;
        n = beta * beta;
        t = delta * delta;

        b = beta * (1 + (3 * eps.es2 - 6 * nm2) * n / 24 + (1 + nm2) * t / 12 + (1 - 2 * nm2) * m / 24);
        l = delta * (1 + m / 24 - (1 - 9 * eps.es2 + 8 * nm2) * n / 24);
        a = alpha * (1 + (2 + 9 * eps.es2 - 6 * nm2) * n / 24 + (1 + nm2) * t / 12 - (1 + 2 * nm2) * m / 24);

//print("b: %lf  l: %lf  a: %lf\n", b, l, a);
      }

      this.y += b;
      this.x += l;
      a2 = a1 + a;

      if (this.y > Math.PI_2) this.y -= Math.PI;
        else if (this.y < -Math.PI_2) this.y += Math.PI;

      if (this.x > Math.PI) this.x -= 2 * Math.PI;
        else if (this.x < -Math.PI) this.x += 2 * Math.PI;

      if (a2 >= 2 * Math.PI) a2 -= 2 * Math.PI;
        else if (a2 <= -2 * Math.PI) a2 += 2 * Math.PI;
      if (a2 < 0) a2 += 2 * Math.PI;

      bl = true;
      if (type != this.type) bl = this.convert(type);

      return bl;
    }


    /**
    * Обратная геодезическая задача.
    *
    * Даны координаты точек А и В; найти расстояние между ними, прямой и обратный азимуты.
    * Метод получает на входе координаты второй точки и вычисляет расстояние и азимуты.
    * Обратный азимут может требовать поправки ±180°.
    */
    public bool get_diff (Point pnt2, out double s, out double a1, out double a2)
    {
      double b, l, bm, am;
      double n, m, nm2, t1, t2;
      double q, p, a;
      double cos_bm, sin_bm;

      s = 0;
      a1 = 0;
      a2 = 0;

      bool bl = true;
      CoordSysType type1 = this.type;
      CoordSysType type2 = pnt2.type;

      if (type1 == CoordSysType.NULL || type1 == CoordSysType.GEO_MAX || type1 >= CoordSysType.MAX ||
          type2 == CoordSysType.NULL || type2 == CoordSysType.GEO_MAX || type2 >= CoordSysType.MAX)
        return false;

      if (type1 == CoordSysType.UTM || type1 == CoordSysType.GK) bl = this.convert(CoordSysType.WGS_84);
        else if (type1 == CoordSysType.CK_42_GK) bl = this.convert(CoordSysType.CK_42);
      if (bl != true) return false;

      if (pnt2.type != this.type) bl = pnt2.convert(this.type);
      if (bl != true) return false;

      EllParams eps = {0};
      if (Gp.Point.get_ell_params( this.type, ref eps) != true) return false;


      b = pnt2.y - this.y;
      l = pnt2.x - this.x;
      bm = (this.y + pnt2.y) / 2;

      cos_bm = Math.cos(bm);
      sin_bm = Math.sin(bm);

      nm2 = eps.es2 * cos_bm * cos_bm;
      n = eps.ax_c / Math.sqrt(1 + nm2);
      m = n / (1 + nm2);

      t1 = l * l * cos_bm * cos_bm;
      t2 = l * l * sin_bm * sin_bm;

      q = b * m * (1 - (eps.es2 - 2 * nm2) * b * b / 8 - (1 + nm2) * t1 / 12 - t2 / 8);
      p = l * n * cos_bm * (1 + (1 - 9 * eps.es2 + 8 * nm2) * b * b / 24 - t2 / 24);
      a = l * sin_bm * (1 + (3 + 2 * nm2) * b * b / 24 + (1 + nm2) * t1 / 12);

      am = Math.atan(p / q);
      s = Math.sqrt(q * q + p * p);
      a1 = am - a/2;
      a2 = am + a/2;

      if (a1 >= 2 * Math.PI) a1 -= 2 * Math.PI;
        else if (a1 <= -2 * Math.PI) a1 += 2 * Math.PI;
      if (a1 < 0) a1 += 2 * Math.PI;

      if (a2 >= 2 * Math.PI) a2 -= 2 * Math.PI;
        else if (a2 <= -2 * Math.PI) a2 += 2 * Math.PI;
      if (a2 < 0) a2 += 2 * Math.PI;

      bl = true;
      if (type1 != this.type) bl = this.convert(type1);

      return bl;
    }


    /**
    * Радиус (м) параллели на широте b (рад).
    */
    public static double lat_to_r (CoordSysType type, double b)
    {
      EllParams eps = {0};
      Gp.Point.get_ell_params( type, ref eps);

      double r, s = Math.sin(b);
      r = eps.ax_a * Math.cos(b) / Math.sqrt(1 - eps.e2 * s * s);

      return r;
    }


    /**
    * Длина дуги (м) меридиана от экватора до параллели под широтой b (рад).
    */
    public static double lat_to_arc (CoordSysType type, double b)
    {
      ArcParams aps = {0};
      Gp.Point.get_arc_params( type, ref aps);

      double arc = aps.c0 * b - aps.c2 * Math.sin(2 * b) +
                   aps.c4 * Math.sin(4 * b) - aps.c6 * Math.sin(6 * b);

      return arc;
    }


    /**
    * Широта (рад), соответствующая длине дуги (м) по меридиану от экватора.
    */
    public static double arc_to_lat (CoordSysType type, double arc)
    {
      ArcParams aps = {0};
      Gp.Point.get_arc_params( type, ref aps);

      double b0 = arc / aps.c0;
      double b = b0 + aps.d2 * Math.sin(2 * b0) + aps.d4 * Math.sin(4 * b0) + aps.d6 * Math.sin(6 * b0);

      return b;
    }
  }
}

