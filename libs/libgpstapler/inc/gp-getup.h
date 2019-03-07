/*
 * GpGetup is a grid library with GpIcaRendererInterface.
 *
 * Copyright (C) 2013 Andrey Vodilov.
 *
 * GpGetup is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * GpGetup is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with GpGetup. If not, see <http://www.gnu.org/licenses/>.
 *
*/

#ifndef _gp_getup_h
#define _gp_getup_h

#include "gp-icarenderer.h"
#include "gp-icastaterenderer.h"

G_BEGIN_DECLS

enum { GP_GETUP_AXIS = 0, GP_GETUP_AXIS_PART, GP_GETUP_BORDER, GP_GETUP_RENDERERS_NUM };

#define G_TYPE_GP_GETUP                     gp_getup_get_type()
#define GP_GETUP( obj )                     ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_GP_GETUP, GpGetup ) )
#define GP_GETUP_CLASS( vtable )            ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_GP_GETUP, GpGetupClass ) )
#define GP_GETUP_GET_CLASS( obj )           ( G_TYPE_INSTANCE_GET_INTERFACE ( ( obj ), G_TYPE_GP_GETUP, GpGetupClass ) )


typedef struct _GpGetup GpGetup;
typedef struct _GpGetupClass GpGetupClass;

GType gp_getup_get_type( void );


struct _GpGetup
{
  GInitiallyUnowned parent;
};

struct _GpGetupClass
{
  GInitiallyUnownedClass parent_class;
};


GpGetup *gp_getup_new( GpIcaStateRenderer *state_renderer );


G_END_DECLS

#endif // _gp_getup_h
