#pragma once

namespace ImGui
{
	void LockMousePos();
}

// Print ImGui errors to console on verified build
#ifdef USE_VERIFY_IN_RELEASE
extern void Msg(const char* format, ...);
#define IM_ASSERT(_EXPR) do {if (!(_EXPR)) Msg("![ImGui Error] %s", #_EXPR);} while(0)
#endif