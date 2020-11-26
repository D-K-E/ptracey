#pragma once
#include <color/color.hpp>
#include <color/colorable.hpp>
#include <color/spd.hpp>
#include <color/specutils.hpp>
#include <common.hpp>
#include <utils.hpp>
#include <math3d/vec3.hpp>

using namespace ptracey;
namespace ptracey {

class rgb_model : public vec3, public colorable {
public:
  SpectrumType type = SpectrumType::RGB;

public:
  rgb_model() : vec3() {}
  rgb_model(Real e0, Real e1, Real e2) : vec3(e0, e1, e2) {}
  rgb_model(Real e0) : vec3(e0) {}
  rgb_model(const Real e1[3]) : vec3(e1) {}
  rgb_model(const vec3 &v) : vec3(v.x(), v.y(), v.z()) {}
  rgb_model(
      const path &path_to_csv,
      const std::string &wave_col_name,
      const std::string &power_col_name,
      const char &sep = ',', const uint stride = SPD_STRIDE,
      SpectrumType stype = SpectrumType::Reflectance) {
    spd csv_spect = spd(path_to_csv, wave_col_name,
                        power_col_name, sep, stride);
    vec3 xyz;
    get_cie_values(csv_spect, xyz);
    vec3 rgb = xyz2rgb_cie(xyz);
    e[0] = rgb.x();
    e[1] = rgb.y();
    e[2] = rgb.z();
  }
  vec3 to_xyz() const override {
    return rgb2xyz_cie(vec3(x(), y(), z()));
  }
  vec3 to_vec3() const { return vec3(x(), y(), z()); }
  vec3 to_rgb() const override {
    return vec3(x(), y(), z());
  }
  vec3 evaluate(const WaveLength &w) const {
    return vec3(x(), y(), z());
  }
  void add(const color &c, const WaveLength &w) {
    e[0] += c.vdata.x();
    e[1] += c.vdata.y();
    e[2] += c.vdata.z();
  }
  rgb_model operator*(Real coeff) const {
    auto ps = to_vec3() * coeff;
    auto rm = rgb_model(ps);
    return rm;
  }
  void scale(Real coeff) {
    e[0] *= coeff;
    e[1] *= coeff;
    e[2] *= coeff;
  }
};
}
