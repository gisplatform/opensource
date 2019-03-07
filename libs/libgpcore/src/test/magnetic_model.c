/*! \file magnetic_model.c
 *
 *  Старый алгоритм расчета магнитного склонения (на Си).
 *
 *  Created on: 20.10.2011
 *      Author: zankov.p
 */

#include <glib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "magnetic_model_and_geo_point.h"

/*
 * The WMM2010 coefficients.
 * Fields are: n, m, g, h, dg/dt, dh/dt.
 * Units are nT for g and h and nT/year for dg/dt and dh/dt.
 */
static gchar *wmm_2010 = ""
      "1  0  -29496.6       0.0       11.6        0.0;"
      "1  1   -1586.3    4944.4       16.5      -25.9;"
      "2  0   -2396.6       0.0      -12.1        0.0;"
      "2  1    3026.1   -2707.7       -4.4      -22.5;"
      "2  2    1668.6    -576.1        1.9      -11.8;"
      "3  0    1340.1       0.0        0.4        0.0;"
      "3  1   -2326.2    -160.2       -4.1        7.3;"
      "3  2    1231.9     251.9       -2.9       -3.9;"
      "3  3     634.0    -536.6       -7.7       -2.6;"
      "4  0     912.6       0.0       -1.8        0.0;"
      "4  1     808.9     286.4        2.3        1.1;"
      "4  2     166.7    -211.2       -8.7        2.7;"
      "4  3    -357.1     164.3        4.6        3.9;"
      "4  4      89.4    -309.1       -2.1       -0.8;"
      "5  0    -230.9       0.0       -1.0        0.0;"
      "5  1     357.2      44.6        0.6        0.4;"
      "5  2     200.3     188.9       -1.8        1.8;"
      "5  3    -141.1    -118.2       -1.0        1.2;"
      "5  4    -163.0       0.0        0.9        4.0;"
      "5  5      -7.8     100.9        1.0       -0.6;"
      "6  0      72.8       0.0       -0.2        0.0;"
      "6  1      68.6     -20.8       -0.2       -0.2;"
      "6  2      76.0      44.1       -0.1       -2.1;"
      "6  3    -141.4      61.5        2.0       -0.4;"
      "6  4     -22.8     -66.3       -1.7       -0.6;"
      "6  5      13.2       3.1       -0.3        0.5;"
      "6  6     -77.9      55.0        1.7        0.9;"
      "7  0      80.5       0.0        0.1        0.0;"
      "7  1     -75.1     -57.9       -0.1        0.7;"
      "7  2      -4.7     -21.1       -0.6        0.3;"
      "7  3      45.3       6.5        1.3       -0.1;"
      "7  4      13.9      24.9        0.4       -0.1;"
      "7  5      10.4       7.0        0.3       -0.8;"
      "7  6       1.7     -27.7       -0.7       -0.3;"
      "7  7       4.9      -3.3        0.6        0.3;"
      "8  0      24.4       0.0       -0.1        0.0;"
      "8  1       8.1      11.0        0.1       -0.1;"
      "8  2     -14.5     -20.0       -0.6        0.2;"
      "8  3      -5.6      11.9        0.2        0.4;"
      "8  4     -19.3     -17.4       -0.2        0.4;"
      "8  5      11.5      16.7        0.3        0.1;"
      "8  6      10.9       7.0        0.3       -0.1;"
      "8  7     -14.1     -10.8       -0.6        0.4;"
      "8  8      -3.7       1.7        0.2        0.3;"
      "9  0       5.4       0.0        0.0        0.0;"
      "9  1       9.4     -20.5       -0.1        0.0;"
      "9  2       3.4      11.5        0.0       -0.2;"
      "9  3      -5.2      12.8        0.3        0.0;"
      "9  4       3.1      -7.2       -0.4       -0.1;"
      "9  5     -12.4      -7.4       -0.3        0.1;"
      "9  6      -0.7       8.0        0.1        0.0;"
      "9  7       8.4       2.1       -0.1       -0.2;"
      "9  8      -8.5      -6.1       -0.4        0.3;"
      "9  9     -10.1       7.0       -0.2        0.2;"
      "10  0      -2.0       0.0        0.0        0.0;"
      "10  1      -6.3       2.8        0.0        0.1;"
      "10  2       0.9      -0.1       -0.1       -0.1;"
      "10  3      -1.1       4.7        0.2        0.0;"
      "10  4      -0.2       4.4        0.0       -0.1;"
      "10  5       2.5      -7.2       -0.1       -0.1;"
      "10  6      -0.3      -1.0       -0.2        0.0;"
      "10  7       2.2      -3.9        0.0       -0.1;"
      "10  8       3.1      -2.0       -0.1       -0.2;"
      "10  9      -1.0      -2.0       -0.2        0.0;"
      "10 10      -2.8      -8.3       -0.2       -0.1;"
      "11  0       3.0       0.0        0.0        0.0;"
      "11  1      -1.5       0.2        0.0        0.0;"
      "11  2      -2.1       1.7        0.0        0.1;"
      "11  3       1.7      -0.6        0.1        0.0;"
      "11  4      -0.5      -1.8        0.0        0.1;"
      "11  5       0.5       0.9        0.0        0.0;"
      "11  6      -0.8      -0.4        0.0        0.1;"
      "11  7       0.4      -2.5        0.0        0.0;"
      "11  8       1.8      -1.3        0.0       -0.1;"
      "11  9       0.1      -2.1        0.0       -0.1;"
      "11 10       0.7      -1.9       -0.1        0.0;"
      "11 11       3.8      -1.8        0.0       -0.1;"
      "12  0      -2.2       0.0        0.0        0.0;"
      "12  1      -0.2      -0.9        0.0        0.0;"
      "12  2       0.3       0.3        0.1        0.0;"
      "12  3       1.0       2.1        0.1        0.0;"
      "12  4      -0.6      -2.5       -0.1        0.0;"
      "12  5       0.9       0.5        0.0        0.0;"
      "12  6      -0.1       0.6        0.0        0.1;"
      "12  7       0.5       0.0        0.0        0.0;"
      "12  8      -0.4       0.1        0.0        0.0;"
      "12  9      -0.4       0.3        0.0        0.0;"
      "12 10       0.2      -0.9        0.0        0.0;"
      "12 11      -0.8      -0.2       -0.1        0.0;"
      "12 12       0.0       0.9        0.1        0.0";

/*
 * Алгоритм расчета взят из "The US/UK World Magnetic Model for 2010-2015", http://www.ngdc.noaa.gov/geomag/WMM/
 */
gdouble magnetic_declination( GeoPoint position, gdouble height, GDate *date )
{
   static gboolean initialized = FALSE;

   static gdouble g_t0[13][13]; // Gauss main coefficients at time t0
   static gdouble h_t0[13][13]; // Gauss main coefficients at time t0
   static gdouble dg_t0[13][13]; // Gauss secular variation coefficients at time t0
   static gdouble dh_t0[13][13]; // Gauss secular variation coefficients at time t0
   static gdouble root[13]; // some equations with sqrt
   static gdouble roots[13][13][2]; // some equations with sqrt
   static GDate base_date; // base date of the magnetic model

   g_assert( date );
   g_assert( g_date_valid( date ) );

   if ( !initialized)
   {
      gint n, m;
      gint i = 0;
      gint length;
      gchar **strings;

      initialized = TRUE;

      g_date_clear( &base_date, 1 );
      g_date_set_dmy( &base_date, 1, G_DATE_JANUARY, 2010 );

      memset( g_t0, 0, sizeof( g_t0 ) );
      memset( h_t0, 0, sizeof( h_t0 ) );
      memset( dg_t0, 0, sizeof( dg_t0 ) );
      memset( dh_t0, 0, sizeof( dh_t0 ) );

      strings = g_strsplit( wmm_2010, ";", 0 );
      length = g_strv_length( strings );

      for ( n = 1; n <= 12; n++ )
         for ( m = 0; m <= n; m++, i++ )
         {
            gint scanned_n, scanned_m;

            g_assert( i < length );

            gint scanf_rval = sscanf(strings[i],
              "%i %i %lf %lf %lf %lf",
              &scanned_n, &scanned_m, &g_t0[n][m], &h_t0[n][m], &dg_t0[n][m], &dh_t0[n][m]);

            g_assert_cmpint(scanf_rval, ==, 6);

            g_assert( n == scanned_n && m == scanned_m );
         }

      g_strfreev( strings );

      memset( root, 0, sizeof( root ) );
      memset( roots, 0, sizeof( roots ) );

      for ( n = 2; n <= 12; n++ )
         root[n] = sqrt( ( 2.0 * n - 1 ) / ( 2.0 * n ) );

      for ( m = 0; m <= 12; m++ )
         for ( n = MAX( m + 1, 2 ); n <= 12; n++ )
         {
            roots[m][n][0] = sqrt( ( n - 1 ) * ( n - 1 ) - m * m );
            roots[m][n][1] = 1.0 / sqrt( n * n - m * m );
         }
   }

   gint n, m;

   gint days; // days count between the base and and the given date

   gdouble g[13][13]; // Gauss coefficients at given time
   gdouble h[13][13]; // Gauss coefficients at given time
   gdouble P[13][13]; // Schmidt semi-normalized associated Legendre functions values at sin( fi_ )
   gdouble dP[13][13]; // dP/dfi_

   gdouble a = 6371200; // geomagnetic reference radius, m
   gdouble A = 6378137; // semi-major axis, m
   gdouble f = 1 / 298.257223563; // 1 / ( reciprocal flattening )
   gdouble e2; // squared eccentricity
   gdouble Rc; // radius of East-West curvature, m

   gdouble fi; // geodetic latitude, rad
   gdouble lam; // geocentric spherical longitude, rad
   gdouble fi_; // geocentric spherical latitude, rad
   gdouble r; // geocentric spherical radius, m
   gdouble p, z; // p = sqrt( x*x + y*y ) and z are the coordinates of a geocentric Cartesian coordinate system in which the positive x and z axes point in the directions of the prime meridian (λ=0) and the Earth’s rotation axis, respectively, m

   gdouble X_, Z_, Y_; // field components in geocentric coordinates, nT

   gdouble X; // North field component, nT
   gdouble Y; // East field component, nT

   gdouble D; // magnetic declination, rad

   gdouble sin_fi_; // sin( fi_ )
   gdouble cos_fi_; // cos( fi_ )
   gdouble sin_mlam[13]; // sin( m * lam )
   gdouble cos_mlam[13]; // cos( m * lam )

   /*
    * Compute Gauss coefficients at given time
    */

   days = (gint) g_date_get_julian( date ) - (gint) g_date_get_julian( &base_date );

   for ( n = 0; n <= 12; n++ )
      for ( m = 0; m <= 12; m++ )
      {
         g[n][m] = g_t0[n][m] + (gdouble) days / 365.25 * dg_t0[n][m];
         h[n][m] = h_t0[n][m] + (gdouble) days / 365.25 * dh_t0[n][m];
      }

   /*
    * Compute geodetic coordinates
    */

   fi = position.latitude / 180 * M_PI;
   lam = position.longitude / 180 * M_PI; // longitude is the same in both geodetic and geocentric spherical coordinates

   /*
    * Compute geocentric coordinates
    */

   e2 = f * ( 2 - f );
   Rc = A / sqrt( 1 - e2 * sin( fi ) * sin( fi ) );

   p = ( Rc + height ) * cos( fi );
   z = ( Rc * ( 1 - e2 ) + height ) * sin( fi );
   r = sqrt( p * p + z * z );
   fi_ = asin( z / r );

   /*
    *
    */

   sin_fi_ = sin( fi_ );
   cos_fi_ = cos( fi_ );

   for ( m = 0; m <= 12; m++ )
   {
      sin_mlam[m] = sin( m * lam );
      cos_mlam[m] = cos( m * lam );
   }

   /*
    * Compute spherical functions
    */

   memset( P, 0, sizeof( P ) );
   memset( dP, 0, sizeof( dP ) );

   // Diagonal elements

   P[0][0] = 1;
   P[1][1] = cos_fi_;
   dP[0][0] = 0;
   dP[1][1] = -sin_fi_;
   P[1][0] = sin_fi_;
   dP[1][0] = cos_fi_;

   for ( n = 2; n <= 12; n++ )
   {
      P[n][n] = P[n - 1][n - 1] * cos_fi_ * root[n];
      dP[n][n] = ( dP[n - 1][n - 1] * cos_fi_ - P[n - 1][n - 1] * sin_fi_ ) * root[n];
   }

   // Lower triangle elements

   for ( m = 0; m <= 12; m++ )
      for ( n = MAX( m + 1, 2 ); n <= 12; n++ )
      {
         P[n][m] = ( P[n - 1][m] * sin_fi_ * ( 2.0 * n - 1 ) - P[n - 2][m] * roots[m][n][0] ) * roots[m][n][1];
         dP[n][m] = ( ( dP[n - 1][m] * sin_fi_ + P[n - 1][m] * cos_fi_ ) * ( 2.0 * n - 1 )
               - dP[n - 2][m] * roots[m][n][0] ) * roots[m][n][1];
      }

   /*
    * Compute geocentric field
    */

   X_ = 0;
   Y_ = 0;
   Z_ = 0;

   for ( n = 1; n <= 12; n++ )
   {
      gdouble sum_x = 0;
      gdouble sum_y = 0;
      gdouble sum_z = 0;
      gdouble ar;

      for ( m = 0; m <= n; m++ )
      {
         sum_x += ( g[n][m] * cos_mlam[m] + h[n][m] * sin_mlam[m] ) * dP[n][m];

         sum_y += m * ( g[n][m] * sin_mlam[m] - h[n][m] * cos_mlam[m] ) * P[n][m];

         sum_z += ( g[n][m] * cos_mlam[m] + h[n][m] * sin_mlam[m] ) * P[n][m];
      }

      ar = pow( a / r, n + 2 );

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

   X = X_ * cos( fi_ - fi ) - Z_ * sin( fi_ - fi );
   Y = Y_;

   /*
    * Compute declination
    */

   D = atan2( Y, X );

   return D;
}
