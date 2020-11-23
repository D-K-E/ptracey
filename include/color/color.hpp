#pragma once
//
#include <color/specutils.hpp>
#include <common.hpp>
#include <vec3.hpp>
using namespace ptracey;
namespace ptracey {

struct color {
  SpectrumType type;
  vec3 vdata;
  Power pdata;
  color() {}
  color(const Power &p)
      : pdata(p), type(SpectrumType::Illuminant) {}
  color(const Real &p1, const Real &p2, const Real &p3)
      : vdata(p1, p2, p3), type(SpectrumType::RGB) {}
  color(const vec3 &rgb)
      : vdata(rgb), type(SpectrumType::RGB) {}

  static vec3 random() { return vec3::random(); }
  static vec3 random(Real t1, Real t2) {
    return vec3::random(t1, t2);
  }
  color &operator*=(const Real &t) {
    pdata *= t;
    vdata *= t;
    return *this;
  }
  color operator*(const color &c) const {
    auto nvdata = vdata * c.vdata;
    auto npdata = pdata * c.pdata;
    color nc;
    nc.type = type;
    nc.vdata = nvdata;
    nc.pdata = npdata;
    return nc;
  }
  color operator/(const color &c) const {
    auto nvdata = vdata / c.vdata;
    auto npdata = pdata / c.pdata;
    color nc;
    nc.type = type;
    nc.vdata = nvdata;
    nc.pdata = npdata;
    return nc;
  }
  color operator/(const Real &c) const {
    auto nvdata = vdata / c;
    auto npdata = pdata / c;
    color nc;
    nc.type = type;
    nc.vdata = nvdata;
    nc.pdata = npdata;
    return nc;
  }
  color operator+(const Real &c) const {
    auto nvdata = vdata + c;
    auto npdata = pdata + c;
    color nc;
    nc.type = type;
    nc.vdata = nvdata;
    nc.pdata = npdata;
    return nc;
  }
  color operator+(const color &c) const {
    auto nvdata = vdata + c.vdata;
    auto npdata = pdata + c.pdata;
    color nc;
    nc.type = type;
    nc.vdata = nvdata;
    nc.pdata = npdata;
    return nc;
  }
};
}
