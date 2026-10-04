#pragma once

struct ID3DUserDefinedAnnotation;

// The Event Browser tints a region only on the D3DPERF path
enum dx10_marker_color : u32
{
    dx10_marker_frame = 0xff8a8f98,
    dx10_marker_gbuffer = 0xff4c8bf5,
    dx10_marker_lights = 0xfff5b74c,
    dx10_marker_combine = 0xff5fc27e,
    dx10_marker_post = 0xffb06fd6,
};

#define PIX_EVENT(Name) dxPixEventWrapper pixEvent##Name(L#Name)
#define PIX_EVENT_C(Name, Color) dxPixEventWrapper pixEvent##Name(L#Name, Color)

void dx10_annotate_frame();

class dxPixEventWrapper
{
    ID3DUserDefinedAnnotation* annotation;
    bool perf_event;

public:
    dxPixEventWrapper(LPCWSTR wszName, u32 color = 0);
    ~dxPixEventWrapper();
};

// uniqued by __LINE__ so several can share one block
#define PIX_EVENT_F_CAT_(a, b) a##b
#define PIX_EVENT_F_CAT(a, b) PIX_EVENT_F_CAT_(a, b)
#define PIX_EVENT_F(...) dxPixEventScope PIX_EVENT_F_CAT(pixScope_, __LINE__)(__VA_ARGS__)

class dxPixEventScope
{
    bool active;

public:
    explicit dxPixEventScope(const char* fmt, ...);
    ~dxPixEventScope();
};


#include "dx10DebugName.h"
