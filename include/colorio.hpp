#pragma once
#include <color/color.hpp>
#include <color/rgb_model.hpp>
#include <color/spectrum.hpp>
#include <common.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

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

void write_color(std::ostream &out,
                 shared_ptr<color> pix_spec,
                 int samples_per_pixel) {
  Real scale = 1.0 / samples_per_pixel;
  vec3 pix_xyz = pix_spec->to_xyz();
  vec3 pixel_color = xyz2rgb_cie(pix_xyz);
  auto r = pixel_color.r() * scale;
  auto g = pixel_color.g() * scale;
  auto b = pixel_color.b() * scale;

  // Replace NaN components with zero. See explanation in
  // Ray Tracing: The Rest of Your Life.
  if (r != r)
    r = 0.0;
  if (g != g)
    g = 0.0;
  if (b != b)
    b = 0.0;

  // Divide the color by the number of samples and
  // gamma-correct for gamma=2.0.
  r = sqrt(r);
  g = sqrt(g);
  b = sqrt(b);

  // Write the translated [0,255] value of each color
  // component.
  out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(b, 0.0, 0.999))
      << '\n';
}
}
