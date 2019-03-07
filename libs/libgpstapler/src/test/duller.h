/*!
 * \file duller.h
 *
 * \author Sergey Volkhin
 * \date 02.12.2013
 *
 * Объект Duller -- тестовый AsyncTiler.
 *
*/

#ifndef _duller_h
#define _duller_h

#include <glib-object.h>
#include "gp-smartcache.h"
#include "gp-tiler.h"


G_BEGIN_DECLS


#define DULLER_TYPE       (duller_get_type())
#define DULLER(obj)       (G_TYPE_CHECK_INSTANCE_CAST((obj), DULLER_TYPE, Duller))
#define DULLER_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass), DULLER_TYPE, DullerClass))
#define DULLER_IS(obj)      (G_TYPE_CHECK_INSTANCE_TYPE((obj), DULLER_TYPE))
#define DULLER_CLASS_IS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), DULLER_TYPE))
#define DULLER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_INTERFACE((obj), DULLER_TYPE, DullerClass))



typedef struct _Duller Duller;
typedef struct _DullerClass DullerClass;
typedef struct _DullerPriv DullerPriv;


GType duller_get_type(void);



struct _Duller
{
  GpAsyncTiler parent;

  // Private data.
  DullerPriv *priv;
};

struct _DullerClass
{
  GpAsyncTilerClass parent_class;
};



/// Создание объекта Duller. Данная функция создает тестовый объект для генерации "плиток".
///
/// \param cache - кэш для хранения данных, или NULL;
/// \param dispatcher - диспетчер обработки плиток.
///
/// \return указатель на созданый объект.
Duller *duller_new(GpSmartCache *cache, GpDispatcher *dispatcher);


/// Позволяет получить тип плиток,
/// генерируемых данным GpAsyncTiler'ом.
///
/// \param self -указатель на объект Duller.
///
/// \return тип плиток.
gint duller_get_tiles_type(Duller *self);



G_END_DECLS


#endif /* _duller_h */

