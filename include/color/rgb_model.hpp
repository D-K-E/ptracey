#pragma once
#include <color/colorable.hpp>
#include <color/spd.hpp>
#include <color/specutils.hpp>
#include <common.hpp>
#include <utils.hpp>
#include <vec3.hpp>

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
      const std::string &sep = ",",
      const uint stride = SPD_STRIDE,
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
  vec3 evaluate(const WaveLength &w) const {
    return vec3(x(), y(), z());
  }
  void add(const rgb_model &rgbm, const WaveLength &w) {
    e[0] += rgbm.x();
    e[1] += rgbm.y();
    e[2] += rgbm.z();
  }
};
}
