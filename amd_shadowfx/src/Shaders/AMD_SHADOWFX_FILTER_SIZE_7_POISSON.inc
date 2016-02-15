#ifndef AMD_SHADOWS_FILTER_SIZE_7_POISSON_INC
#define AMD_SHADOWS_FILTER_SIZE_7_POISSON_INC

static const uint g_PoissonSamplesCount = 18;

static const float2 g_PoissonSamples[g_PoissonSamplesCount] =
{
  { -0.7393085f, 3.280662f },
  { -2.47004f, 2.328731f },
  { -0.6732481f, 1.042242f },
  { 0.6072469f, 1.525136f },
  { 0.9831414f, 3.14807f },
  { 1.894908f, 0.6981092f },
  { 0.5978739f, 0.0825575f },
  { 2.06167f, -0.6915861f },
  { -1.294738f, -0.2353872f },
  { 3.23345f, 1.27049f },
  { -2.976625f, 0.1078734f },
  { -1.566728f, -2.490001f },
  { 0.022746f, -1.93031f },
  { -2.484528f, -1.378844f },
  { 1.984003f, -2.342571f },
  { -0.2734823f, -3.234874f },
  { 3.35825f, -0.3363621f },
  { 2.090277f, 2.286526f }
};

#endif  // AMD_SHADOWS_FILTER_SIZE_7_POISSON_INC