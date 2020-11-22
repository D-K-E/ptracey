#pragma once
#include <color/colorable.hpp>
#include <color/rgb_model.hpp>
#include <color/spectrum.hpp>
#include <common.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

void write_color(std::ostream &out, spectrum pix_spec,
                 int samples_per_pixel) {
  Real scale = 1.0 / samples_per_pixel;
  vec3 pix_xyz = pix_spec.to_xyz();
  pix_xyz *= scale;
  vec3 pixel_color = xyz2rgb_cie(pix_xyz);
  auto r = pixel_color.r(); // scale;
  auto g = pixel_color.g(); // scale;
  auto b = pixel_color.b(); // scale;

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
