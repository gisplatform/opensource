/*
 * Nein -- is a NEtwork INformation library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef _gp_nein_h
#define _gp_nein_h

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _GpNein GpNein;

#define GP_TYPE_NEIN          (gp_nein_get_type)
#define GP_NEIN_CAST(object)  ((GpNein*)(object))
#define GP_NEIN(object)       (GP_NEIN_CAST(object))

GType gp_nein_get_type(void) G_GNUC_CONST;
GpNein *gp_nein_new(const gchar *iface, const gchar *ip);
GpNein *gp_nein_copy(GpNein *src);
void gp_nein_free(GpNein *nein);

struct _GpNein
{
  gchar *iface;
  gchar *ip;
};


/**
 * gp_nein_collect:
 * @family: Семейство адресов: G_SOCKET_FAMILY_IPV4 для IPv4, G_SOCKET_FAMILY_IPV6 для IPv6,
 * G_SOCKET_FAMILY_INVALID для обоих).
 *
 * Функция собирает информацию об IP-адресах хоста.
 *
 * Приоритетные (согласно GSSDP) адреса идут в начале списка.
 *
 * Returns: (array length=len)(transfer full): IP-адреса количеством в #len в случае успеха,
 * в случае ошибки возвращается NULL, в len пишется 0 и бросается исключение типа GpNeinError.
 */
GpNein *gp_nein_collect(GSocketFamily family, guint *len, GError **error);

/**
* GpNeinError:
* @GP_NEIN_FAILED: Общее исключение.
*
* Виды исключений GpNein.
*/
typedef enum
{
  GP_NEIN_ERROR_FAILED,
}
GpNeinError;

/**
* GP_NEIN_ERROR:
*
* Макрос для получения #GError quark для ошибок библиотеки libgpnein.
*/
#define GP_NEIN_ERROR (gp_nein_error_quark())
GQuark gp_nein_error_quark(void) G_GNUC_CONST;

G_END_DECLS

#endif // _gp_nein_h
