///////////////////////////////////////////////////////////////
// DummyArtifact.cpp
// DummyArtefact - артефакт пустышка
///////////////////////////////////////////////////////////////

#include "DummyArtifact.h"
#include "../xrphysics/PhysicsShell.h"


CDummyArtefact::CDummyArtefact(void)
{
}

CDummyArtefact::~CDummyArtefact(void)
{
}

void CDummyArtefact::Load(LPCSTR section)
{
	inherited::Load(section);
}
