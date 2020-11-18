#pragma once
#include <common.hpp>
#include <spectrum.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

inline color xyz2rgb(const vec3 xyz) {
  auto r = 3.240479 * xyz.x() - 1.537150 * xyz.y() -
           0.498535 * xyz.z();
  auto g = -0.969256 * xyz.x() + 1.875991 * xyz.y() +
           0.041556 * xyz.z();
  auto b = 0.055648 * xyz.x() - 0.204043 * xyz.y() +
           1.057311 * xyz.z();
  return color(r, g, b);
}
vec3 to_xyz(const sampled_spectrum &s_lambda) {
  auto X = get_cie_val(s_lambda.spect,
                       sampled_spectrum::cie_xbar);
  auto Y = get_cie_val(s_lambda.spect,
                       sampled_spectrum::cie_ybar);
  auto Z = get_cie_val(s_lambda.spect,
                       sampled_spectrum::cie_zbar);
  auto xyzsum = X + Y + Z;
  xyzsum = xyzsum == 0 ? 1 : xyzsum;
  auto xval = X / xyzsum;
  auto yval = Y / xyzsum;
  auto zval = 1.0 - (xval + yval);
  vec3 xyz = vec3(xval, yval, zval);
  return xyz;
}
vec3 to_xyz(const sampled_spectrum &s_lambda,
            const sampled_spectrum &r_lambda) {
  auto X = get_cie_val(s_lambda.spect, r_lambda.spect,
                       sampled_spectrum::cie_xbar);
  auto Y = get_cie_val(s_lambda.spect, r_lambda.spect,
                       sampled_spectrum::cie_ybar);
  auto Z = get_cie_val(s_lambda.spect, r_lambda.spect,
                       sampled_spectrum::cie_zbar);
  auto xyzsum = X + Y + Z;
  xyzsum = xyzsum == 0 ? 1 : xyzsum;
  auto xval = X / xyzsum;
  auto yval = Y / xyzsum;
  auto zval = 1.0 - (xval + yval);
  vec3 xyz = vec3(xval, yval, zval);
  return xyz;
}
color to_color(const sampled_spectrum &s_lambda) {
  return xyz2rgb(to_xyz(s_lambda));
}
color to_color(const sampled_spectrum &s_lambda,
               const sampled_spectrum &r_lambda) {
  return xyz2rgb(to_xyz(s_lambda, r_lambda));
}
color to_color(const color &rgb) { return rgb; }
color to_color(const color &s_lambda,
               const color &r_lambda) {
  return r_lambda;
}
color to_color(const shared_ptr<spectrum> &spec_ptr) {
  return to_color(*spec_ptr);
}
color to_color(const shared_ptr<spectrum> &spec_ptr,
               const shared_ptr<spectrum> &ref_ptr) {
  return to_color(*spec_ptr, *ref_ptr);
}

void write_color(std::ostream &out,
                 shared_ptr<spectrum> pix_spec,
                 int samples_per_pixel) {
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
  auto scale = 1.0 / samples_per_pixel;
  r = sqrt(scale * r);
  g = sqrt(scale * g);
  b = sqrt(scale * b);

  // Write the translated [0,255] value of each color
  // component.
  out << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << ' '
      << static_cast<int>(256 * clamp(b, 0.0, 0.999))
      << '\n';
}
}
