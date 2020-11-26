#pragma once
//
#include <color/spectrum.hpp>
#include <common.hpp>
#include <texture/perlin.hpp>
#include <camera/ray.hpp>
#include <math3d/vec3.hpp>
using namespace ptracey;
namespace ptracey {
class texture {
public:
  virtual color value(Real u, Real v, const vec3 &p,
                      const WaveLength &w) const = 0;
};
class solid_color : public texture {
public:
  solid_color() {}
  solid_color(const spectrum &c) : color_value(c) {}

  solid_color(Real red, Real green, Real blue)
      : solid_color(spectrum(red, green, blue)) {}

  color value(Real u, Real v, const vec3 &p,
              const WaveLength &w) const override {
    return color_value.evaluate(w);
  }

private:
  spectrum color_value;
};
class checker_texture : public texture {
public:
  checker_texture() {}

  checker_texture(shared_ptr<texture> _odd,
                  shared_ptr<texture> _even)
      : odd(_odd), even(_even) {}

  checker_texture(const spectrum &c1, const spectrum &c2)
      : odd(make_shared<solid_color>(c1)),
        even(make_shared<solid_color>(c2)) {}

  color value(Real u, Real v, const vec3 &p,
              const WaveLength &w) const override {
    auto sines =
        sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
    if (sines < 0)
      return odd->value(u, v, p, w);
    else
      return even->value(u, v, p, w);
  }

public:
  shared_ptr<texture> odd;
  shared_ptr<texture> even;
};
class noise_texture : public texture {
public:
  noise_texture() {}
  noise_texture(Real sc, const spectrum &sp)
      : scale(sc), spec(sp) {}

  color value(Real u, Real v, const vec3 &p,
              const WaveLength &w) const override {
    Real coeff =
        0.5 * (1 + sin(scale * p.z() + 10 * noise.turb(p)));
    color eval = spec.evaluate(w);
    eval *= coeff;
    return eval;
  }

public:
  perlin noise;
  Real scale;
  spectrum spec;
};
class image_texture : public texture {
public:
  const static int bytes_per_pixel = 3;
  image_texture()
      : data(nullptr), width(0), height(0),
        bytes_per_scanline(0) {}
  image_texture(const char *filename) {
    auto components_per_pixel = bytes_per_pixel;

    data = stbi_load(filename, &width, &height,
                     &components_per_pixel,
                     components_per_pixel);

    if (!data) {
      std::cerr
          << "ERROR: Could not load texture image file '"
          << filename << "'.\n";
      width = height = 0;
    }

    bytes_per_scanline = bytes_per_pixel * width;
  }
  ~image_texture() { STBI_FREE(data); }
  color value(Real u, Real v, const vec3 &p,
              const WaveLength &wl) const override {
    // If we have no texture data, then return solid cyan as
    // a debugging aid.
    if (data == nullptr) {
      return spectrum(0, 1, 1).evaluate(wl);
    }

    // Clamp input texture coordinates to [0,1] x [1,0]
    u = clamp(u, 0.0, 1.0);
    v = 1.0 -
        clamp(v, 0.0, 1.0); // Flip V to image coordinates

    auto i = static_cast<int>(u * width);
    auto j = static_cast<int>(v * height);

    // Clamp integer mapping, since actual coordinates
    // should be less than 1.0
    if (i >= width)
      i = width - 1;
    if (j >= height)
      j = height - 1;

    const auto color_scale = 1.0 / 255.0;
    auto pixel =
        data + j * bytes_per_scanline + i * bytes_per_pixel;

    spectrum pix_rgb(color_scale * pixel[0],
                     color_scale * pixel[1],
                     color_scale * pixel[2]);
    return pix_rgb.evaluate(wl);
  }

private:
  unsigned char *data;
  int width, height;
  int bytes_per_scanline;
};
class material_texture : public texture {
  // use material refractive spectral power distribution
  // for color
public:
  material_texture(const spectrum &s)
      : spect(make_shared<solid_color>(s)) {}
  material_texture(shared_ptr<texture> t) : spect(t) {}
  material_texture(
      const path &path_to_csv,
      const std::string &wave_col_name,
      const std::string &power_col_name,
      const char &sep = ',',
      const unsigned int stride = SPD_STRIDE,
      SpectrumType stype = SpectrumType::Reflectance) {
    auto spd_material =
        spectrum(CSV_PARENT / path_to_csv, wave_col_name,
                 power_col_name, sep, stride, stype);
    spect = make_shared<solid_color>(spd_material);
  }

  color value(Real u, Real v, const vec3 &p,
              const WaveLength &w) const {
    return spect->value(u, v, p, w);
  }

public:
  shared_ptr<texture> spect;
};
}
