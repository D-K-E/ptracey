#pragma once
#include <common.hpp>
#include <spectrum.hpp>
#include <tocolor.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

void write_color(std::ostream &out, spectrum pix_spec,
                 int samples_per_pixel) {
  pix_spec = pix_spec.div((Real)samples_per_pixel);
  auto pixel_color = to_color(pix_spec);
  auto r = pixel_color.r();
  auto g = pixel_color.g();
  auto b = pixel_color.b();

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

  // Write the translated [0,255] value of each color
  // component.
  out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(b, 0.0, 0.999))
      << '\n';
}
}
