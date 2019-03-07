/*!
 * \file weigher.h
 *
 * \author Sergey Volkhin
 * \date 02.12.2013
 *
 * Объект Weigher -- тестовый Tiler.
 *
*/

#ifndef _weigher_h
#define _weigher_h

#include <glib-object.h>
#include "gp-smartcache.h"
#include "gp-tiler.h"


G_BEGIN_DECLS


#define WEIGHER_TYPE       (weigher_get_type())
#define WEIGHER(obj)       (G_TYPE_CHECK_INSTANCE_CAST((obj), WEIGHER_TYPE, Weigher))
#define WEIGHER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass), WEIGHER_TYPE, WeigherClass))
#define WEIGHER_IS(obj)      (G_TYPE_CHECK_INSTANCE_TYPE((obj), WEIGHER_TYPE))
#define WEIGHER_CLASS_IS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), WEIGHER_TYPE))
#define WEIGHER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_INTERFACE((obj), WEIGHER_TYPE, WeigherClass))



typedef struct _Weigher Weigher;
typedef struct _WeigherClass WeigherClass;
typedef struct _WeigherPriv WeigherPriv;


GType weigher_get_type(void);



struct _Weigher
{
  GpTiler parent;

  // Private data.
  WeigherPriv *priv;
};

struct _WeigherClass
{
  GpTilerClass parent_class;
};



/// Создание объекта Weigher. Данная функция создает тестовый объект для генерации "плиток".
///
/// \param cache - кэш для хранения данных, или NULL;
///
/// \return указатель на созданый объект.
Weigher *weigher_new(GpSmartCache *cache);


/// Позволяет получить тип плиток,
/// генерируемых данным GpTiler'ом.
///
/// \param self -указатель на объект Weigher.
///
/// \return тип плиток.
gint weigher_get_tiles_type(Weigher *self);

void weigher_set_area(Weigher *weigher, gdouble x, gdouble y, gdouble w, gdouble h, gdouble step);

void weigher_set_bottom_type(Weigher *weigher, gint bottom_type);

gdouble weigher_calc_weight(Weigher *weigher, gdouble x, gdouble y);

void weigher_set_data_type(Weigher *weigher, gint data_type);

gboolean is_graphical(Weigher *weigher, gint type);
G_END_DECLS


#endif /* _weigher_h */

