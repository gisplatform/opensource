/**
 * \file magnetic_model.h
 *
 * \author Zankov Peter
 * \date 20.10.2011
 *
 * \defgroup magnetic Модель магнитного поля Земли
 * @{
 */

#ifndef MAGNETIC_MODEL_AND_GEO_POINT_H_
#define MAGNETIC_MODEL_AND_GEO_POINT_H_

G_BEGIN_DECLS

typedef struct _GeoPoint GeoPoint;

/**
 * Геодезическая координата точки.
 **/

struct _GeoPoint
{
   /**
    * Долгота точки в градусах.
    *
    * Восточная долгота положительна.
    **/
   gdouble longitude;

   /**
    * Широта точки в градусах.
    *
    * Северная широта положительна.
    **/
   gdouble latitude;
};


/**
 * Вычисление магнитного склонения.
 *
 * \param position координата WGS84
 * \param height высота над уровнем WGS84, м
 * \param date дата, для которой высчитывается магнитное склонение
 *
 * \return значение магнитного склонения в радианах
 */
gdouble magnetic_declination( GeoPoint position, gdouble height, GDate *date );

G_END_DECLS

#endif /* MAGNETIC_MODEL__AND_GEO_POINTH_ */

/* @} */
