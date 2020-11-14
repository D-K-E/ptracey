#pragma once
#include <vec3.hpp>

class ray {
public:
  ray() {}
  ray(const point3 &origin, const vec3 &direction)
      : orig(origin), dir(direction), tm(0) {}

  ray(const point3 &origin, const vec3 &direction,
      Real time)
      : orig(origin), dir(direction), tm(time) {}

  point3 origin() const { return orig; }
  vec3 direction() const { return dir; }
  Real time() const { return tm; }

  point3 at(Real t) const { return orig + t * dir; }

public:
  point3 orig;
  vec3 dir;
  Real tm;
};
