#pragma once
struct ID3D11Texture2D;
namespace personal_fg
{
void BeginSimulation();
void BeginRender();
void EndPresent();
void CaptureScene(ID3D11Texture2D* depth, ID3D11Texture2D* motion);
void CaptureHudless();
void Reset();
bool ReflexLimitsFrames();
bool NeedsHudless();
bool ProtectEmbeddedUi();
}
