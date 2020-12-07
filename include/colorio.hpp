#pragma once
#include <color/colorable.hpp>
#include <color/rgb_model.hpp>
#include <color/spectrum.hpp>
#include <common.hpp>
#include <math3d/vec3.hpp>

using namespace ptracey;
namespace ptracey {

void write_color(std::ostream &out, spectrum pix_spec,
                 int samples_per_pixel) {
  Real anti_aliasing_scale = 1.0 / (Real)samples_per_pixel;
  pix_spec.scale(anti_aliasing_scale);
  // std::cerr << "spectrum: " << pix_spec << std::endl;
  vec3 pixel_color = pix_spec.to_rgb();
  // pixel_color *= anti_aliasing_scale;
  // std::cerr << "color: " << pixel_color << std::endl;
  auto r = pixel_color.r(); // scale;
  auto g = pixel_color.g(); // scale;
  auto b = pixel_color.b(); // scale;

  // Replace NaN components with zero. See explanation in
  // Ray Tracing: The Rest of Your Life.

  if (r != r || r < 0.0)
    r = 0.0;
  if (g != g || g < 0.0)
    g = 0.0;
  if (b != b || b < 0.0)
    b = 0.0;

  // Divide the color by the number of samples and
  // gamma-correct for gamma=2.0.
  r = sqrt(r);
  g = sqrt(g);
  b = sqrt(b);

  //
  r = clamp(r, 0.0, 0.999);
  g = clamp(g, 0.0, 0.999);
  b = clamp(b, 0.0, 0.999);

  // Write the translated [0,255] value of each color
  // component.
  out << static_cast<int>(256 * r) << ' '
      << static_cast<int>(256 * g) << ' '
      << static_cast<int>(256 * b) << std::endl;
}
void write_color(
    const std::vector<std::vector<spectrum>> &imvec,
    int image_height, int image_width,
    int samples_per_pixel) {
  for (int j = image_height - 1; j >= 0; j -= 1) {
    std::cerr << "\rKalan Tarama Çizgisi:" << ' ' << j
              << ' ' << std::flush;
    for (int i = 0; i < image_width; i += 1) {
      write_color(std::cout, imvec[i][j],
                  samples_per_pixel);
    }
  }
}

void write_color(
    const char *fpath,
    const std::vector<std::vector<spectrum>> &specs,
    int image_height, int image_width) {}
}
