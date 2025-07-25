#ifndef noiseH
#define noiseH

#include "xrCore.h"

float noise3(const Fvector& vec);
float fractalsum3(const Fvector& v, float freq, int octaves);
float turbulence3(const Fvector& v, float freq, int octaves);

#endif
