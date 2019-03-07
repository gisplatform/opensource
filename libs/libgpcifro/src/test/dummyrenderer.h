#ifndef _dummy_renderer_h
#define _dummy_renderer_h

#include "gp-icarenderer.h"
#include "gp-icastaterenderer.h"

G_BEGIN_DECLS

enum { DUMMY_RENDERER_AXIS = 0, DUMMY_RENDERER_AXIS_PART, DUMMY_RENDERER_BORDER, DUMMY_RENDERER_PROGRESS, DUMMY_RENDERER_RENDERERS_NUM };

#define G_TYPE_DUMMY_RENDERER                     dummy_renderer_get_type()
#define DUMMY_RENDERER( obj )                     ( G_TYPE_CHECK_INSTANCE_CAST ( ( obj ), G_TYPE_DUMMY_RENDERER, DummyRenderer ) )
#define DUMMY_RENDERER_CLASS( vtable )            ( G_TYPE_CHECK_CLASS_CAST ( ( vtable ), G_TYPE_DUMMY_RENDERER, DummyRendererClass ) )
#define DUMMY_RENDERER_GET_CLASS( obj )           ( G_TYPE_INSTANCE_GET_INTERFACE ( ( obj ), G_TYPE_DUMMY_RENDERER, DummyRendererClass ) )

GType dummy_renderer_get_type( void );

typedef GInitiallyUnowned DummyRenderer;
typedef GInitiallyUnownedClass DummyRendererClass;


DummyRenderer *dummy_renderer_new( GpIcaStateRenderer *state_renderer );


G_END_DECLS

#endif // _dummy_renderer_h
