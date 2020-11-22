#pragma once
// spectrum utils
#include <common.hpp>
#include <matrix.hpp>
#include <vec3.hpp>

using namespace ptracey;

namespace ptracey {
enum class SpectrumType { Reflectance, Illuminant, RGB };

vec3 xyz2rgb_cie(const vec3 xyz) {
  //
  matrix mat;
  mat.x[0][0] = 2.3706743;
  mat.x[0][1] = -0.5138850;
  mat.x[0][2] = 0.0052982;
  mat.x[0][3] = 0.0;
  //
  mat.x[1][0] = -0.9000405;
  mat.x[1][1] = 1.4253036;
  mat.x[1][2] = -0.0146949;
  mat.x[1][3] = 0.0;
  //
  mat.x[2][0] = -0.4706338;
  mat.x[2][1] = 0.0885814;
  mat.x[2][2] = 1.0093968;
  mat.x[2][3] = 0.0;
  //
  mat.x[3][0] = 0.0;
  mat.x[3][1] = 0.0;
  mat.x[3][2] = 0.0;
  mat.x[3][3] = 1.0;
  return mat * xyz;
}
vec3 xyz2rgb_srgb(const vec3 xyz) {
  //
  matrix mat;
  mat.x[0][0] = 3.2404542;
  mat.x[0][1] = -0.9692660;
  mat.x[0][2] = 0.0556434;
  mat.x[0][3] = 0.0;
  //
  mat.x[1][0] = -1.5371385;
  mat.x[1][1] = 1.8760108;
  mat.x[1][2] = -0.2040259;
  mat.x[1][3] = 0.0;
  //
  mat.x[2][0] = -0.4985314;
  mat.x[2][1] = 0.0415560;
  mat.x[2][2] = 1.0572252;
  mat.x[2][3] = 0.0;
  //
  mat.x[3][0] = 0.0;
  mat.x[3][1] = 0.0;
  mat.x[3][2] = 0.0;
  mat.x[3][3] = 1.0;
  return mat * xyz;
}
vec3 rgb2xyz_srgb(const vec3 rgb) {
  matrix mat;
  mat.x[0][0] = 0.4124564;
  mat.x[0][1] = 0.2126729;
  mat.x[0][2] = 0.0193339;
  mat.x[0][3] = 0.0;
  //
  mat.x[1][0] = 0.3575761;
  mat.x[1][1] = 0.7151522;
  mat.x[1][2] = 0.1191920;
  mat.x[1][3] = 0.0;
  //
  mat.x[2][0] = 0.1804375;
  mat.x[2][1] = 0.0721750;
  mat.x[2][2] = 0.9503041;
  mat.x[2][3] = 0.0;
  //
  mat.x[3][0] = 0.0;
  mat.x[3][1] = 0.0;
  mat.x[3][2] = 0.0;
  mat.x[3][3] = 1.0;
  return mat * rgb;
}
vec3 rgb2xyz_cie(const vec3 rgb) {
  //
  matrix mat;
  mat.x[0][0] = 0.4887180;
  mat.x[0][1] = 0.1762044;
  mat.x[0][2] = 0.0;
  mat.x[0][3] = 0.0;
  //
  mat.x[1][0] = 0.3106803;
  mat.x[1][1] = 0.8129847;
  mat.x[1][2] = 0.0102048;
  mat.x[1][3] = 0.0;
  //
  mat.x[2][0] = 0.2006017;
  mat.x[2][1] = 0.0108109;
  mat.x[2][2] = 0.9897952;
  mat.x[2][3] = 0.0;
  //
  mat.x[3][0] = 0.0;
  mat.x[3][1] = 0.0;
  mat.x[3][2] = 0.0;
  mat.x[3][3] = 1.0;
  return mat * rgb;
}
}
