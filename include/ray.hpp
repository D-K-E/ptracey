#pragma once
#include <color/specutils.hpp>
#include <common.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {
class ray {
public:
  ray() {}
  ray(const point3 &o, const vec3 &d)
      : orig(o), dir(d), tm(0),
        wlength(static_cast<WaveLength>(random_int(
            VISIBLE_LAMBDA_START, VISIBLE_LAMBDA_END))) {}

  ray(const point3 &o, const vec3 &d, Real time,
      WaveLength wl)
      : orig(o), dir(d), tm(time), wlength(wl) {}

  point3 origin() const { return orig; }
  vec3 direction() const { return dir; }
  WaveLength wavelength() const { return wlength; }
  Real time() const { return tm; }

  point3 at(Real t) const { return orig + t * dir; }

public:
  point3 orig;
  vec3 dir;
  Real tm;
  WaveLength wlength;
};
}
