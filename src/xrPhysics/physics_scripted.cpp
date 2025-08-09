#include "physics_scripted.h"
#include "xrDebug.h"
#include "xrMemory.h"

cphysics_scripted::~cphysics_scripted()
{
	xr_delete(m_game_scripted);
}

void cphysics_scripted::set(iphysics_game_scripted* g)
{
	R_ASSERT(g);
	R_ASSERT(!m_game_scripted);
	R_ASSERT(&(g->iphysics_impl()) == this) ;
	m_game_scripted = g;
}
