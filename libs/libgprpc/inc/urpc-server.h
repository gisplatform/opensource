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
 * \file urpc-server.h
 *
 * \author Andrei Fadeev
 * \date 07.03.2014
 * \brief Заголовочный файл класса сервера удалённых вызовов процедур через udp сокет.
 *
 */

#ifndef _urpc_server_h
#define _urpc_server_h

#include <glib-object.h>

#include <gp-rpc-server.h>

G_BEGIN_DECLS


// Описание класса.
#define G_TYPE_URPC_SERVER                urpc_server_get_type()
#define URPC_SERVER( obj )                ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_URPC_SERVER, URpcServer ) )
#define URPC_SERVER_CLASS( vtable )       ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_URPC_SERVER, URpcServerClass ) )
#define IS_URPC_SERVER( obj )             ( G_TYPE_CHECK_INSTANCE_TYPE ( ( obj ), G_TYPE_URPC_SERVER ) )
#define URPC_SERVER_GET_CLASS( obj )      ( G_TYPE_INSTANCE_GET_INTERFACE (( obj ), G_TYPE_URPC_SERVER, URpcServerClass ) )

GType urpc_server_get_type( void );


typedef GObject URpcServer;
typedef GObjectClass URpcServerClass;


G_END_DECLS

#endif // _urpc_server_h
