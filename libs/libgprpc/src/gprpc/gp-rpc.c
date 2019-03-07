/*
 * GPRPC - rpc (remote procedure call) library, this library is part of GRTL.
 *
 * Copyright 2009,2010,2014 Andrei Fadeev
 *
 * This file is part of GRTL.
 *
 * GRTL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GRTL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GRTL. If not, see <http://www.gnu.org/licenses/>.
 *
*/

/*!
 * \file gp-rpc.c
 *
 * \author Andrei Fadeev
 * \date 20.03.2009
 * \brief Исходный файл интерфейса удалённых вызовов процедур.
 *
*/

#include <gio/gio.h>
#include <glib/gi18n-lib.h>

#include "urpc.h"
#include "trpc.h"
#include "srpc.h"
#include "gp-rpc-common.h"


G_DEFINE_INTERFACE( GpRpc, gp_rpc, G_TYPE_OBJECT );

static void gp_rpc_default_init (GpRpcInterface *iface){ }


GpRpcData *gp_rpc_lock (GpRpc *gp_rpc)
{

  return GP_RPC_GET_CLASS(gp_rpc)->lock(gp_rpc);

}


gboolean gp_rpc_exec (GpRpc *gp_rpc, guint32 proc_id, guint32 obj_id, GError **error)
{
  return GP_RPC_GET_CLASS(gp_rpc)->exec(gp_rpc, proc_id, obj_id, error);
}


void gp_rpc_unlock (GpRpc *gp_rpc)
{

  GP_RPC_GET_CLASS(gp_rpc)->unlock(gp_rpc);

}


gboolean gp_rpc_connected (GpRpc *gp_rpc)
{

  return GP_RPC_GET_CLASS(gp_rpc)->connected(gp_rpc);

}


gchar *gp_rpc_get_self_uri (GpRpc *gp_rpc)
{

  return GP_RPC_GET_CLASS(gp_rpc)->get_self_uri(gp_rpc);

}


gchar *gp_rpc_get_server_uri (GpRpc *gp_rpc)
{

  return GP_RPC_GET_CLASS(gp_rpc)->get_server_uri(gp_rpc);

}


GpRpc *gp_rpc_create (const gchar *uri, guint data_size, gdouble exec_timeout, guint restart, GpRpcAuth *auth, GError **error)
{

  GType client_type;

  if(gp_rpc_get_transport_type (uri) == NULL )
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Failed to determine protocol from URI."));
    return NULL;
  }
  else if( g_pattern_match_simple( "udp", gp_rpc_get_transport_type (uri) ) ) client_type = G_TYPE_URPC;
  else if( g_pattern_match_simple( "tcp", gp_rpc_get_transport_type (uri) ) ) client_type = G_TYPE_TRPC;
  else if( g_pattern_match_simple( "shm", gp_rpc_get_transport_type (uri) ) ) client_type = G_TYPE_SRPC;
  else
  {
    g_critical("gp_rpc_get_transport_type returned unexpected value: '%s'.", gp_rpc_get_transport_type(uri));
    return NULL;
  }

  return g_initable_new(client_type, NULL, error,
    "uri", uri,
    "data-size", data_size,
    "exec-timeout", exec_timeout,
    "restart", restart,
    "auth", auth,
    NULL);
}
