#pragma once

#include <xrAPI.h>
#include <xrDebug.h>

class CGameMtlLibrary;
IC CGameMtlLibrary& GMLibrary()
{
	VERIFY(PGMLib);
	return *PGMLib;
}
