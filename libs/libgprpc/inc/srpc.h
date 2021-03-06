/*
 * GPRPC - rpc (remote procedure call) library, this library is part of GRTL.
 *
 * Copyright 2010 Andrei Fadeev
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
 * \file srpc.h
 *
 * \author Andrei Fadeev
 * \date 20.03.2009
 * \brief Заголовочный файл класса удалённых вызовов процедур через разделяемую память.
 *
 */

#ifndef _srpc_h
#define _srpc_h

#include <glib-object.h>

#include <gp-rpc.h>

G_BEGIN_DECLS


// Описание класса.
#define G_TYPE_SRPC                     srpc_get_type()
#define SRPC( obj )                     ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_SRPC, SRpc ) )
#define SRPC_CLASS( vtable )            ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_SRPC, SRpcClass ) )
#define IS_SRPC( obj )                  ( G_TYPE_CHECK_INSTANCE_TYPE ( ( obj ), G_TYPE_SRPC ) )
#define SRPC_GET_CLASS( obj )           ( G_TYPE_INSTANCE_GET_INTERFACE (( obj ), G_TYPE_SRPC, SRpcClass ) )

GType srpc_get_type( void );


typedef GObject SRpc;
typedef GObjectClass SRpcClass;


G_END_DECLS

#endif // _srpc_h
