#pragma once
#include <color/color.hpp>
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
  vec3 to_xyz() const override {
    return rgb2xyz_cie(vec3(x(), y(), z()));
  }
  template <> vec3 evaluate(const WaveLength &w) const {
    return vec3(x(), y(), z());
  }
};
}
