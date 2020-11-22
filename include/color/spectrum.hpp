#pragma once
// implements spectrum type from pbrt-book
/*
   Illuminant
   X = \int S(\lambda) \bar{x}(\lambda)
   Y = \int S(\lambda) \bar{y}(\lambda)
   Z = \int S(\lambda) \bar{z}(\lambda)

   Reflectance
   X = \int R(\lambda) S(\lambda) \bar{x}(\lambda)
   Y = \int R(\lambda) S(\lambda) \bar{y}(\lambda)
   Z = \int R(\lambda) S(\lambda) \bar{z}(\lambda)
   R(\lambda) material param

 */
// mostly adapted from
#include <color/rgb_model.hpp>
#include <color/sampled_spectrum.hpp>
// CONSTANTS
using namespace ptracey;
namespace ptracey {

// end static variable initialize

typedef rgb_model spectrum;
typedef rgb_model color;
// typedef sampled_spectrum spectrum;
// typedef Power color;
}
