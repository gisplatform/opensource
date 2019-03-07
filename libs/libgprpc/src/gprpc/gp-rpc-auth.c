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
 * \file gp-rpc-auth.c
 *
 * \author Andrei Fadeev
 * \date 20.02.2014
 * \brief Исходный файл интерфейса аутентификации.
 *
*/

#include "gp-rpc-auth.h"


G_DEFINE_INTERFACE( GpRpcAuth, gp_rpc_auth, G_TYPE_OBJECT );

static void gp_rpc_auth_default_init (GpRpcAuthInterface *iface){ }


GpRpcAuthStatus gp_rpc_auth_check (GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data)
{

  return GP_RPC_AUTH_GET_CLASS(gp_rpc_auth)->check(gp_rpc_auth, gp_rpc_data);

}


gboolean gp_rpc_auth_authenticate (GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data)
{

  return GP_RPC_AUTH_GET_CLASS(gp_rpc_auth)->authenticate(gp_rpc_auth, gp_rpc_data);

}


gpointer gp_rpc_auth_get_user_data (GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data)
{

  return GP_RPC_AUTH_GET_CLASS(gp_rpc_auth)->get_user_data(gp_rpc_auth, gp_rpc_data);

}


gboolean gp_rpc_auth_clean_session (GpRpcAuth *gp_rpc_auth, GpRpcData *gp_rpc_data)
{

  return GP_RPC_AUTH_GET_CLASS(gp_rpc_auth)->clean_session(gp_rpc_auth, gp_rpc_data);

}
