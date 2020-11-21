#pragma once
//
#include <common.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

class colorable {
public:
  virtual vec3 to_xyz(const T &el) const = 0;
};
//
}
