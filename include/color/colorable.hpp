#pragma once
//
#include <common.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

class colorable {
public:
  virtual vec3 to_xyz() const = 0;
  virtual vec3 to_rgb() const = 0;

  template <typename T>
  void evaluate(const WaveLength &wn, T &t) const {}
};
//
}
