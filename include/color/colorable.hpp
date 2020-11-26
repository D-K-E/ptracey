#pragma once
//
#include <common.hpp>
#include <math3d/vec3.hpp>

using namespace ptracey;
namespace ptracey {

class colorable {
public:
  virtual vec3 to_xyz() const = 0;
  virtual vec3 to_rgb() const = 0;
};
//
}
