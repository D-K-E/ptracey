#pragma once
#include <common.hpp>
#include <spectrum.hpp>
#include <tocolor.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

void write_color(std::ostream &out, spectrum pix_spec,
                 int samples_per_pixel) {
  Real scale = 1.0 / samples_per_pixel;
  spectrum pspec = pix_spec;
  color pixel_color = to_color(pspec);
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
