#pragma once
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {
class ray {
public:
  ray() {}
  ray(const point3 &origin, const vec3 &direction)
      : orig(origin), dir(direction), tm(0),
        wlength(static_cast<unsigned int>(
            random_int(360, 750))) {}

  ray(const point3 &origin, const vec3 &direction,
      Real time, unsigned int wl)
      : orig(origin), dir(direction), tm(time),
        wlength(wl) {}

  point3 origin() const { return orig; }
  vec3 direction() const { return dir; }
  unsigned int wavelength() const { return wlength; }
  Real time() const { return tm; }

  point3 at(Real t) const { return orig + t * dir; }

public:
  point3 orig;
  vec3 dir;
  Real tm;
  unsigned int wlength;
};
}
