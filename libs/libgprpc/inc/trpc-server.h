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
 * \file trpc-server.h
 *
 * \author Andrei Fadeev
 * \date 18.03.2014
 * \brief Заголовочный файл класса сервера удалённых вызовов процедур через tcp сокет.
 *
 */

#ifndef _trpc_server_h
#define _trpc_server_h

#include <glib-object.h>

#include <gp-rpc-server.h>

G_BEGIN_DECLS


// Описание класса.
#define G_TYPE_TRPC_SERVER                trpc_server_get_type()
#define TRPC_SERVER( obj )                ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_TRPC_SERVER, TRpcServer ) )
#define TRPC_SERVER_CLASS( vtable )       ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_TRPC_SERVER, TRpcServerClass ) )
#define IS_TRPC_SERVER( obj )             ( G_TYPE_CHECK_INSTANCE_TYPE ( ( obj ), G_TYPE_TRPC_SERVER ) )
#define TRPC_SERVER_GET_CLASS( obj )      ( G_TYPE_INSTANCE_GET_INTERFACE (( obj ), G_TYPE_TRPC_SERVER, TRpcServerClass ) )

GType trpc_server_get_type( void );


typedef GObject TRpcServer;
typedef GObjectClass TRpcServerClass;


G_END_DECLS

#endif // _trpc_server_h
