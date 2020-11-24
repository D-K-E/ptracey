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
      : type(SpectrumType::Illuminant), vdata(0.0),
        pdata(p) {}
  color(const Real &p1, const Real &p2, const Real &p3)
      : type(SpectrumType::RGB), vdata(p1, p2, p3),
        pdata(0) {}
  color(const vec3 &rgb)
      : type(SpectrumType::RGB), vdata(rgb), pdata(0) {}

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
