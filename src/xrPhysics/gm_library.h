#pragma once

#include <xrDebug.h>

class CGameMtlLibrary;
IC CGameMtlLibrary& GMLibrary()
{
	VERIFY(PGMLib);
	return *PGMLib;
}
