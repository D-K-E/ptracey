#pragma once
#include <color/colorable.hpp>
#include <color/rgb_model.hpp>
#include <color/spdio.hpp>
#include <color/spectrum.hpp>
#include <color/specutils.hpp>
#include <common.hpp>
#include <math3d/vec3.hpp>
#include <stdexcept>

using namespace ptracey;
namespace ptracey {

void transform_color(spectrum pix_spec,
                     int samples_per_pixel, vec3 &rgbval) {
  Real anti_aliasing_scale = 1.0 / (Real)samples_per_pixel;
  pix_spec.scale(anti_aliasing_scale);
  vec3 pixel_color = pix_spec.to_rgb();

  // Replace NaN components with zero. See explanation in
  // Ray Tracing: The Rest of Your Life.
  pixel_color.conditional_set(0.0, [](auto i) {
    if (i != i || i < 0.0) {
      return true;
    } else {
      return false;
    }
  });

  // Divide the color by the number of samples and
  // gamma-correct for gamma=2.0.
  vec3 rgb = sqrt_vec(pixel_color);
  //
  rgb = clamp(rgb, 0.0, 0.999);
  rgbval = rgb * 256;
  // Write the translated [0,255] value of each color
  // component.
}
void write_color(std::ostream &out, spectrum pix_spec,
                 int samples_per_pixel) {
  vec3 rgbval;
  transform_color(pix_spec, samples_per_pixel, rgbval);
  auto r = rgbval.x();
  auto g = rgbval.y();
  auto b = rgbval.z();
  // Write the translated [0,255] value of each color
  // component.
  out << static_cast<int>(r) << ' ' << static_cast<int>(g)
      << ' ' << static_cast<int>(b) << std::endl;
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
void write_color_spb(
    const char *fpath,
    const std::vector<std::vector<sampled_spectrum>> &specs,
    int image_height, int image_width,
    int samples_per_pixel) {
  Real anti_aliasing_scale = 1.0 / (Real)samples_per_pixel;
  SpbIo sio(specs);
  uint32_t width = (uint32_t)image_width;
  uint32_t height = (uint32_t)image_height;
  uint32_t nb_channels = sio.nb_channels();
  float first_wavelength = sio.first_wave();
  float last_wavelength = sio.last_wave();
  float wavelength_resolution = sio.wavelength_resolution();
  auto vdata = sio.data([anti_aliasing_scale](auto i) {
    return (float)(i * anti_aliasing_scale);
  });
  float *data = vdata.data();

  spb::write_file(fpath, width, height, nb_channels,
                  first_wavelength, wavelength_resolution,
                  last_wavelength, data);
}
void write_color_json(
    const char *fpath,
    const std::vector<std::vector<sampled_spectrum>> &specs,
    int image_height, int image_width,
    int samples_per_pixel) {
  Real anti_aliasing_scale = 1.0 / (Real)samples_per_pixel;
  SpdJsonIo sio(specs, anti_aliasing_scale);
  nlohmann::json jdata = sio.data();
  std::ofstream o(fpath);
  o << std::setw(2) << jdata << std::endl;
}

void write_color(
    const char *fpath,
    const std::vector<std::vector<sampled_spectrum>> &specs,
    int image_height, int image_width,
    int samples_per_pixel) {
  std::string fspath(fpath);
  if (fspath.find(".spb") != std::string::npos) {
    write_color_spb(fpath, specs, image_height, image_width,
                    samples_per_pixel);
  } else if (fspath.find(".json") != std::string::npos) {
    write_color_json(fpath, specs, image_height,
                     image_width, samples_per_pixel);
  } else {
    throw std::runtime_error("unknown file format: " +
                             fspath);
  }
}
}
