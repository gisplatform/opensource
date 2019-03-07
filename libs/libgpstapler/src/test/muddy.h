/*!
 * \file muddy.h
 *
 * \author Sergey Volkhin
 * \date 02.12.2013
 *
 * Объект Muddy -- тестовый Tiler.
 *
*/

#ifndef _muddy_h
#define _muddy_h

#include <glib-object.h>
#include <gtk/gtk.h>
#include "gp-smartcache.h"
#include "gp-tiler.h"


G_BEGIN_DECLS


#define MUDDY_TYPE       (muddy_get_type())
#define MUDDY(obj)       (G_TYPE_CHECK_INSTANCE_CAST((obj), MUDDY_TYPE, Muddy))
#define MUDDY_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass), MUDDY_TYPE, MuddyClass))
#define MUDDY_IS(obj)      (G_TYPE_CHECK_INSTANCE_TYPE((obj), MUDDY_TYPE))
#define MUDDY_CLASS_IS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), MUDDY_TYPE))
#define MUDDY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_INTERFACE((obj), MUDDY_TYPE, MuddyClass))



typedef struct _Muddy Muddy;
typedef struct _MuddyClass MuddyClass;
typedef struct _MuddyPriv MuddyPriv;


GType muddy_get_type(void);



struct _Muddy
{
  GpTiler parent;

  // Private data.
  MuddyPriv *priv;
};

struct _MuddyClass
{
  GpTilerClass parent_class;
};



/// Создание объекта Muddy. Данная функция создает тестовый объект для генерации "плиток".
///
/// \param cache - кэш для хранения данных, или NULL;
///
/// \return указатель на созданый объект.
Muddy *muddy_new(GpSmartCache *cache);


/// Позволяет получить тип плиток,
/// генерируемых данным GpTiler'ом.
///
/// \param self -указатель на объект Muddy.
///
/// \return тип плиток.
gint muddy_get_tiles_type(Muddy *self);

void muddy_set_model(Muddy *muddy, GtkTreeModel *model, gint tiler_col, gint visible_col);

void muddy_set_sum_type(Muddy *muddy, gint sum_type);

G_END_DECLS


#endif /* _muddy_h */

