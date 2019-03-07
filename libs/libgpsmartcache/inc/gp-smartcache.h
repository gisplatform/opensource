/*
 * GP_SMART_CACHE - data caching library, this library is part of GRTL.
 *
 * Copyright 2010 Zankov Peter
 *
 * This file is part of GRTL.
 *
 * GRTL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GRTL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GRTL. If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * \file gp-smartcache.h
 *
 * \author Zankov Peter
 * \date 27.09.2010
 *
 * \brief Заголовочный файл библиотеки кэширования данных.
 *
 * \defgroup gpsmartcache gpsmartcache - Библиотека кэширования данных
 *
 * Библиотека gpsmartcache предназначена для кеширования данных.
 * Доступ к данным осуществляется по двум идентификаторам ( номеру группы и номеру данных ), что
 * позволяет очищать данные одной указанной группы. Для упрощения генерации уникальных номеров групп имеются
 * вызовы #gp_smart_cache_reg_group и #gp_smart_cache_unreg_group.
 * Кэш хранит указатель на данные и их размер. При необходимости кэш удаляет
 * хранимые данные вызовом g_free.
 * Данные хранятся в отсортированном по идентификатору массиве. Для удаления устаревших
 * данных при нехватке места в кэше имеется двунаправленный связный список
 * идентификаторов, отсортированный по времени обращения к данным.
 *
 * Библиотека обеспечивает многопоточную работу.
 *
 * Основные функции библиотеки:
 *
 * - #gp_smart_cache_new - создать объект GpSmartCache
 * - #gp_smart_cache_set_size - установить размер кэша
 * - #gp_smart_cache_get_size - узнать размер кэша
 * - #gp_smart_cache_get_free - узнать размер свободного места
 * - #gp_smart_cache_reg_group - получить уникальный идентификатор группы
 * - #gp_smart_cache_unreg_group - освободить идентификатор группы
 * - #gp_smart_cache_set - сохранить данные
 * - #gp_smart_cache_get - считать данные
 * - #gp_smart_cache_clean - очистить все данные указанной группы
 * - #gp_smart_cache_clean_by_condition - очистить данные, удовлетворяющие условию
 *
 * \name Основные функции библиотеки.
 *
 *
 */

#ifndef _gp_smartcache_h
#define _gp_smartcache_h

#include <glib-object.h>

/* \cond */

G_BEGIN_DECLS

// Описание класса.
#define GP_SMART_CACHE_TYPE                  gp_smart_cache_get_type()
#define GP_SMART_CACHE( obj )                ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), GP_SMART_CACHE_TYPE, GpSmartCache ) )
#define GP_SMART_CACHE_CLASS( vtable )       ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), GP_SMART_CACHE_TYPE, GpSmartCacheClass ) )
#define GP_SMART_CACHE_GET_CLASS( obj )      ( G_TYPE_INSTANCE_GET_CLASS( ( obj ), GP_SMART_CACHE_TYPE, GpSmartCacheClass )

GType gp_smart_cache_get_type (void);

typedef struct _GpSmartCache GpSmartCache;
typedef struct _GpSmartCacheClass GpSmartCacheClass;

// Структура объекта.
struct _GpSmartCache
{
   GObject parent;
};

// Структура класса.
struct _GpSmartCacheClass
{
   GObjectClass parent;
};

/**
 * GP_SMART_CACHE_FREE_K:
 * Коэффициент запаса высвобождаемой памяти для хранения данных.
*/
static const gsize GP_SMART_CACHE_FREE_K = 10;
/* \endcond */


/**
 * gp_smart_cache_new:
 *
 * Создание объекта #GpSmartCache.
 *
 * Returns: Новый объект #GpSmartCache.
*/
GpSmartCache *gp_smart_cache_new ();

/**
 * gp_smart_cache_set_size:
 * @self: указатель на объект.
 * @size: размер, байт.
 *
 * Функция устанавливает максимальный размер кэша.
 * Содержимое кэша очищается.
 */
void gp_smart_cache_set_size (GpSmartCache *self, gsize size);

/**
 * gp_smart_cache_get_size:
 * @self: Объект #GpSmartCache.
 *
 * Функция вернет максимальный размер кэша.
 *
 * Returns: Размер кэша, байт.
 */
gsize gp_smart_cache_get_size (GpSmartCache *self);

/**
 * gp_smart_cache_get_free:
 * @self: Объект #GpSmartCache.
 *
 * Функция вернет размер свободной памяти кэша.
 *
 * Returns: Количество неиспользованной памяти, байт.
 */
gsize gp_smart_cache_get_free (GpSmartCache *self);

/**
 * gp_smart_cache_reg_group:
 * @self: Объект #GpSmartCache.
 *
 * Функция генерирует уникальный идентификатор групы.
 *
 * Вызов предназначен для получения уникального идентификатор группы
 * для хранения данных в кэше. Если группа не используется, необходимо вызвать
 * #gp_smart_cache_unreg_group для освобождения данного номера.
 *
 * Returns: Новый уникальный номер группы.
 */
guint gp_smart_cache_reg_group (GpSmartCache *self);

/**
 * gp_smart_cache_unreg_group:
 * @self: Объект #GpSmartCache.
 * @group: Идентификатор группы.
 *
 * Функция освобождает уникальный идентификатор групы.
 *
 * Помечает указанный идентификатор группы как незанятый и разрешает его
 * выдачу при запросе #gp_smart_cache_reg_group.
 */
void gp_smart_cache_unreg_group (GpSmartCache *self, guint group);

/**
 * gp_smart_cache_set:
 * @self: Объект #GpSmartCache.
 * @group: Идентификатор группы.
 * @index: Идентификатор данных.
 * @data: (transfer full) (element-type guint8) (array length=size): Указатель на данные.
 * @size: Размер данных.
 *
 * Функция добавляет данные в кэш.
 * После передачи данных кэшу он может при необходимости освободить выделенную под
 * них память вызовом g_free.
 */
void gp_smart_cache_set (GpSmartCache *self, guint group, guint index, gpointer data, guint size);

/**
 * gp_smart_cache_set_unowned:
 * @self: Объект #GpSmartCache.
 * @group: Идентификатор группы.
 * @index: Идентификатор данных.
 * @data: (transfer none) (element-type guint8) (array length=size): Указатель на данные.
 * @size: Размер данных.
 *
 * Функция добавляет данные в кэш.
 * В отличие от gp_smart_cache_set, в эту функцию можно передать указатель на данные
 * в памяти, выделенной любым способом. GpSmartCache не будет освобождать память
 * по указателю data с помощью g_free.
 * Вместо этого GpSmartCache самостоятельно выделит под данные память внутри себя.
 */
void gp_smart_cache_set_unowned(GpSmartCache *self, guint group, guint index, gpointer data, guint size);

/**
 * gp_smart_cache_get:
 * @self: Объект списка.
 * @group: Идентификатор группы.
 * @index: Идентификатор данных.
 * @size: (out) (allow-none): переменная для сохранения размера данных, хранимых в кэше, или NULL.
 * @buff: (allow-none) (transfer none) (element-type guint8) (array length=buff_size): Буфер для данных, или NULL.
 * @buff_size: Размер буфера.
 *
 * Функция получает данные из кэша.
 * Если данных нет, вернет FALSE.
 * Если данные нашлись, запишет в переменную size их размер,
 * а сами данные скопирует в буфер buff.
 *
 * Returns: TRUE - если данные есть в кэше, иначе FALSE.
 *
 */
gboolean gp_smart_cache_get(GpSmartCache *self, guint group, guint index, guint *size, gpointer buff, guint buff_size);

/**
 * gp_smart_cache_get2:
 * @self: Объект списка.
 * @group: Идентификатор группы.
 * @index: Идентификатор данных.
 * @size: (out) (allow-none): Переменная для сохранения размера данных, хранимых в кэше, или NULL.
 * @buff1: (allow-none) (transfer none) (element-type guint8) (array length=buff1_size): Первый буфер для данных, или NULL.
 * @buff1_size: Размер первого буфера.
 * @buff2: (allow-none) (transfer none) (element-type guint8) (array length=buff2_size): Второй буфер для данных, или NULL.
 * @buff2_size: Размер второго буфера.
 *
 * Функция получает данные из кэша, раскладывая их в два буфера.
 * Если данных нет, вернет FALSE.
 * Если данные нашлись, запишет в переменную size их размер в кэше,
 * а сами данные скопирует в два буфера:
 * buff1_size байт положит в buff1,
 * из оставшихся данных buff2_size байт положит в buff2.
 *
 * Функция может быть полезна, например, в случае хранения в кэше дополнительных параметров к данным.
 * Пример: если кроме самих данных, мы храним в кэше время (time), к которым эти данные относятся:
 * |[<!-- language="C" -->
 * GTimeVal time;
 * gp_smart_cache_get2(cache, group, index, &size, &time, sizeof(time), data_buff, data_buff_size);
 * ]|
 *
 * Returns: TRUE - если данные есть в кэше, иначе FALSE.
 *
 */
gboolean gp_smart_cache_get2 (GpSmartCache *self, guint group, guint index, guint *size, gpointer buff1,
                              guint buff1_size, gpointer buff2, guint buff2_size);

/**
 * GpSmartCacheAccessFunc:
 * @data: (transfer none) (element-type guint8) (array length=size): Элемент данных в кэше.
 * @size: Размер данных.
 * @user_data: Указатель на пользовательские данные.
 *
 * Тип функции для доступа к элементу данных в кэше.
 *
 * Returns: true или false. Например: true если данные удовлетворяют какому-то условию,
 * или же что они были модифицированы, иначе -- false.
 *
 */
typedef gboolean (*GpSmartCacheAccessFunc)(gconstpointer data, guint size, gpointer user_data);

/**
 * gp_smart_cache_modify:
 * @self: Объект GpSmartCache.
 * @group: Идентификатор группы.
 * @condition: (allow-none) (closure condition_user_data) (scope call): Функция-условие -- для каждого элемента данных кэша указанной группы
 * вызывается данная функция, первым аргументом ей передается указатель на данные в кэше,
 * вторым -- размер данных, третьим -- condition_user_data.
 * @condition_user_data: Указатель на пользовательские данные, который будет передан
 * в функцию condition.
 * @modifier: (closure modifier_user_data) (scope call): Функция-модификатор: функция modifier вызывается для всех элементов данных,
 * которые удовлетворяют условию (condition вернула TRUE).
 * Ей также первым аргументом передается указатель на данные в кэше,
 * вторым -- размер данных, третьим -- modifier_user_data.
 * @modifier_user_data: указатель на пользовательские данные, который будет передан
 * в функцию modifier третьим аргументом.
 *
 * Функция позволяет модифицировать данные в кэше, не копируя.
 *
 * Не позволяет удалять данные или менять их размер.
 *
 * Если проверка условия не требуется, можно, к примеру,
 * воспользоваться функцией gtk_true в качестве condition.
 *
 */
void gp_smart_cache_modify (GpSmartCache *self, guint group,
                            GpSmartCacheAccessFunc condition, gpointer condition_user_data,
                            GpSmartCacheAccessFunc modifier, gpointer modifier_user_data);

/**
 * gp_smart_cache_clean:
 * @self: Объект #GpSmartCache.
 * @group: Идентификатор группы.
 *
 * Функция удаляет все данные указанной группы кэша.
 */
void gp_smart_cache_clean (GpSmartCache *self, guint group);

/**
 * gp_smart_cache_clean_by_condition:
 * @self: Объект #GpSmartCache.
 * @group: Идентификатор группы.
 * @condition: (allow-none) (scope call): Функция-условие: для каждого элемента данных кэша указанной группы
 * вызывается данная функция, первым аргументом ей передается указатель на данные в кэше,
 * вторым -- размер данных, третьим -- user_data.
 * Удаляются только те элементы данных, которые удовлетворяют условию (condition вернула TRUE).
 * Если вместо condition передан NULL, то функция удалит все данные группы,
 * как если бы была вызвана #gp_smart_cache_clean.
 * @user_data: Указатель на пользовательские данные, который будет передан
 * в функцию  condition третьим аргументом.
 *
 * Функция удаляет данные указанной группы кэша,
 * удовлетворящие определенному условию.
 */
void gp_smart_cache_clean_by_condition (GpSmartCache *self, guint group, GpSmartCacheAccessFunc condition,
                                        gpointer user_data);

/**
 * gp_smart_cache_get_sys_memory:
 * Функция позволяет узнать объем оперативной памяти в системе.
 *
 * Returns: Объем ОЗУ в байтах.
 */
guint64 gp_smart_cache_get_sys_memory();

/* \cond */

G_END_DECLS

/* \endcond */

#endif // _gp_smartcache_h
