#pragma once
#include <d3d11.h>

// The optical kernel records the capture UV at each main-view lens pixel.
// Inputs belong to the current SVP camera; never tag these as main-camera data.
struct PersonalFgScope
{
    ID3D11Texture2D* mapping=nullptr;
    ID3D11Texture2D* depth=nullptr;
    ID3D11Texture2D* motion=nullptr;
    ID3D11Texture2D* mainMotion=nullptr; // deferred glass coverage, when required
    float reprojection[16]{};
    float planes[4]{}; // scope near/far, main near/far
    float mainJitterDelta[2]{};
    bool reset=true;
    bool requireGlass=false;
};
