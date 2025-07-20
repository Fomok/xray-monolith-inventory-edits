#pragma once

class CGameMtlLibrary;
IC CGameMtlLibrary& GMLibrary()
{
	VERIFY(PGMLib);
	return *PGMLib;
}
