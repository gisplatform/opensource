/*
 * GPRPC - rpc (remote procedure call) library, this library is part of GRTL.
 *
 * Copyright 2014 Andrei Fadeev
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
 * \file gp-rpc-server.c
 *
 * \author Andrei Fadeev
 * \date 12.02.2014
 * \brief Исходный файл серверного интерфейса удалённых вызовов процедур.
 *
*/

#include <gio/gio.h>
#include <glib/gi18n-lib.h>

#include "urpc-server.h"
#include "trpc-server.h"
#include "srpc-server.h"
#include "gp-rpc-common.h"


G_DEFINE_INTERFACE( GpRpcServer, gp_rpc_server, G_TYPE_OBJECT );

static void gp_rpc_server_default_init (GpRpcServerInterface *iface){ ; }


gchar *gp_rpc_server_get_self_uri (GpRpcServer *gp_rpc_server)
{

  return GP_RPC_SERVER_GET_CLASS(gp_rpc_server)->get_self_uri(gp_rpc_server);

}


GpRpcServer *gp_rpc_server_create (const gchar *uri, guint threads_num, guint data_size, gdouble data_timeout,
                                   GpRpcAuth *auth, GpRpcServerAclCallback acl, GpRpcManager *manager, GError **error)
{

  GType server_type;

  if(gp_rpc_get_transport_type (uri) == NULL )
  {
    g_set_error(error, GP_RPC_ERROR, GP_RPC_ERROR_BAD_URI, _("Failed to determine protocol from URI."));
    return NULL;
  }
  else if( g_pattern_match_simple( "udp", gp_rpc_get_transport_type (uri) ) ) server_type = G_TYPE_URPC_SERVER;
  else if( g_pattern_match_simple( "tcp", gp_rpc_get_transport_type (uri) ) ) server_type = G_TYPE_TRPC_SERVER;
  else if( g_pattern_match_simple( "shm", gp_rpc_get_transport_type (uri) ) ) server_type = G_TYPE_SRPC_SERVER;
  else
  {
    g_critical("gp_rpc_get_transport_type returned unexpected value: '%s'.", gp_rpc_get_transport_type(uri));
    return NULL;
  }

  return g_initable_new( server_type, NULL, error, "uri", uri, "threads-num", threads_num, "data-size", data_size, "data-timeout", data_timeout, "auth", auth, "acl", acl, "manager", manager, NULL );

}

