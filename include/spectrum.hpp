#pragma once
// implements spectrum type from pbrt-book
// mostly adapted from
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.h
#include "common.hpp"
#include <spd.hpp>
#include <specdata.hpp>
#include <utils.hpp>
#include <vec3.hpp>
#include <wave.hpp>
// CONSTANTS

static const int NB_SPECTRAL_SAMPLES = 60;
static const int VISIBLE_LAMBDA_START = 350;
static const int VISIBLE_LAMBDA_END = 800;
static const auto CIE_Y_INTEGRAL = 106.856895;

// utility functions for spectrums

inline void xyz2rgb(const Real xyz[3], Real rgb[3]) {
  rgb[0] = 3.240479 * xyz[0] - 1.537150 * xyz[1] -
           0.498535 * xyz[2];
  rgb[1] = -0.969256 * xyz[0] + 1.875991 * xyz[1] +
           0.041556 * xyz[2];
  rgb[2] = 0.055648 * xyz[0] - 0.204043 * xyz[1] +
           1.057311 * xyz[2];
}
inline void rgb2xyz(const Real rgb[3], Real xyz[3]) {
  xyz[0] = 0.412453 * rgb[0] + 0.357580 * rgb[1] +
           0.180423 * rgb[2];
  xyz[1] = 0.212671 * rgb[0] + 0.715160 * rgb[1] +
           0.072169 * rgb[2];
  xyz[2] = 0.019334 * rgb[0] + 0.119193 * rgb[1] +
           0.950227f * rgb[2];
}

enum class SpectrumType { Reflectance, Illuminant };
enum RGB_AXIS : int { RGB_R, RGB_G, RGB_B };

class abstract_spectrum {
public:
  abstract_spectrum() {}
  virtual Real r() const = 0;
  virtual Real g() const = 0;
  virtual Real b() const = 0;
  virtual Real x() const = 0;
  virtual Real y() const = 0;
  virtual Real z() const = 0;
};

class sampled_spectrum : public abstract_spectrum {
public:
  spd<Real> spect;
  spd<Real> reflectance;
  SpectrumType type;

public:
  sampled_spectrum(
      const spd<Real> &s,
      SpectrumType stype = SpectrumType::Illuminant)
      : spect(s), type(stype) {}
  sampled_spectrum(
      const spd<Real> &s, const spd<Real> &r,
      SpectrumType stype = SpectrumType::Reflectance)
      : spect(s), reflectance(r), type(stype) {}

  sampled_spectrum(const Real &r, const Real &g,
                   const Real &b, SpectrumType stype)
      : type(stype) {
    // from
    // http://scottburns.us/fast-rgb-to-spectrum-conversion-for-reflectances/
    // convert sRGB to linear rgb in range [0,1]
    auto lr = interp<Real>(r, 0.0, 1.0);
    auto lg = interp<Real>(g, 0.0, 1.0);
    auto lb = interp<Real>(b, 0.0, 1.0);
    //
    auto rho_r_wave = rho_r.power();
    auto rho_g_wave = rho_g.power();
    auto rho_b_wave = rho_b.power();
    auto rho =
        lr * rho_r_wave + lg * rho_g_wave + lb * rho_b_wave;
    spect = spd<Real>(rho, rho_r.wavelengths());
  }
  sampled_spectrum(const Real &r, SpectrumType stype)
      : type(stype) {
    auto lr = interp<Real>(r, 0.0, 1.0);
    auto rho_r_wave = rho_r.power();
    auto rho_g_wave = rho_g.power();
    auto rho_b_wave = rho_b.power();
    sampled_wave<Real> rho =
        lr * rho_r_wave + lr * rho_g_wave + lr * rho_b_wave;
    spect = spd<Real>(rho, rho_r.wavelengths());
  }
  template <typename T> static T random() {}
  template <typename T> T add(const T &s) const {
    return s;
  };

  template <typename T> T add(const Real &s) const {}
  template <typename T> T subt(const T &s) const {}
  template <typename T> T subt(const Real &s) const {}
  template <typename T>
  T multip(const sampled_spectrum &s) const {}
  template <typename T> T multip(const Real &s) const {}
  template <typename T>
  T div(const sampled_spectrum &s) const {}
  template <typename T> T div(const Real &s) const {}

protected:
  static spd<Real> cie_xbar;
  static spd<Real> cie_ybar;
  static spd<Real> cie_zbar;
  static spd<Real> rho_r;
  static spd<Real> rho_g;
  static spd<Real> rho_b;
};
path csv_parent = "./media/data/spectrum";
spd<Real> sampled_spectrum::rho_r =
    spd<Real>(csv_parent / "rho-r-2012.csv");
spd<Real> sampled_spectrum::rho_g =
    spd<Real>(csv_parent / "rho-g-2012.csv");
spd<Real> sampled_spectrum::rho_b =
    spd<Real>(csv_parent / "rho-b-2012.csv");
spd<Real> sampled_spectrum::cie_xbar =
    spd<Real>(csv_parent / "cie-x-bar-1964.csv");
spd<Real> sampled_spectrum::cie_ybar =
    spd<Real>(csv_parent / "cie-y-bar-1964.csv");
spd<Real> sampled_spectrum::cie_zbar =
    spd<Real>(csv_parent / "cie-z-bar-1964.csv");

// end static variable initialize

class illuminant_spectrum : public sampled_spectrum {
public:
  vec3 xyz;
  color rgb;

public:
  illuminant_spectrum(const spd<Real> &s)
      : sampled_spectrum(s, SpectrumType::Illuminant) {}
  illuminant_spectrum(const Real &r, const Real &g,
                      const Real &b)
      : sampled_spectrum(r, g, b, SpectrumType::Illuminant),
        rgb(r, g, b) {
    update_spect();
  }
  void update_spect() {
    auto X = get_cie_val(spect, cie_xbar);
    auto Y = get_cie_val(spect, cie_ybar);
    auto Z = get_cie_val(spect, cie_zbar);
    auto xval = X / (X + Y + Z);
    auto yval = Y / (X + Y + Z);
    auto zval = 1.0 - (xval + yval);
    xyz = vec3(xval, yval, zval);
  }
  illuminant_spectrum(const Real &v)
      : sampled_spectrum(v), rgb(v) {
    update_spect();
  }

  Real r() const override { return rgb.r(); }
  Real g() const override { return rgb.g(); }
  Real b() const override { return rgb.b(); }
  Real x() const override { return xyz.x(); }
  Real y() const override { return xyz.y(); }
  Real z() const override { return xyz.z(); }
  illuminant_spectrum operator+(const Real &s) const {
    illuminant_spectrum ispec = *this;
    ispec.spect += s;
  }
  illuminant_spectrum operator-(const Real &s) const {}
  illuminant_spectrum operator*(const Real &s) const {}
  illuminant_spectrum operator/(const Real &s) const {}

  static illuminant_spectrum random() {
    auto sp = spd<Real>();
    return illuminant_spectrum(sp);
  }

  illuminant_spectrum
  add(const illuminant_spectrum &s) const {
    auto sspect = spect + s.spect;
    return illuminant_spectrum(sspect);
  }
  shared_ptr<sampled_spectrum>
  add(const shared_ptr<sampled_spectrum> &s) const {
    auto sspect = spect + s->spect;
    return make_shared<illuminant_spectrum>(sspect);
  }

  illuminant_spectrum add(const Real &s) const {
    auto sspect = spect + s;
    return illuminant_spectrum(sspect);
  }
  illuminant_spectrum
  subt(const illuminant_spectrum &s) const {
    auto sspect = spect - s.spect;
    return illuminant_spectrum(sspect);
  }
  shared_ptr<sampled_spectrum>
  subt(const shared_ptr<sampled_spectrum> &s) const {
    auto sspect = spect - s->spect;
    return make_shared<illuminant_spectrum>(sspect);
  }

  illuminant_spectrum subt(const Real &s) const {
    auto sspect = spect - s;
    return illuminant_spectrum(sspect);
  }
  illuminant_spectrum
  multip(const illuminant_spectrum &s) const {
    auto sspect = spect * s.spect;
    return illuminant_spectrum(sspect);
  }

  shared_ptr<sampled_spectrum>
  multip(shared_ptr<sampled_spectrum> s) {
    auto sspect = spect * s->spect;
    return make_shared<illuminant_spectrum>(sspect);
  }
  illuminant_spectrum multip(const Real &s) const {
    auto sspect = spect * s;
    return illuminant_spectrum(sspect);
  }
  illuminant_spectrum div(const illuminant_spectrum &s) {
    auto sspect = spect / s.spect;
    return illuminant_spectrum(sspect);
  }
  shared_ptr<sampled_spectrum>
  div(shared_ptr<sampled_spectrum> s) {
    auto sspect = spect / s->spect;
    return make_shared<illuminant_spectrum>(sspect);
  }

  illuminant_spectrum div(sampled_spectrum s) {
    auto sspect = spect / s.spect;
    return illuminant_spectrum(sspect);
  }
  illuminant_spectrum div(const Real &s) {
    auto sspect = spect / s;
    return illuminant_spectrum(sspect);
  }
};
class reflectance_spectrum : public sampled_spectrum {
public:
  vec3 xyz;
  color rgb;

public:
  reflectance_spectrum(const spd<Real> &s,
                       const spd<Real> &r)
      : sampled_spectrum(s, r, SpectrumType::Reflectance) {
    auto X = get_cie_val(spect, reflectance, cie_xbar);
    auto Y = get_cie_val(spect, reflectance, cie_ybar);
    auto Z = get_cie_val(spect, reflectance, cie_zbar);
    auto xval = X / (X + Y + Z);
    auto yval = Y / (X + Y + Z);
    auto zval = 1.0 - (X + Y);
    xyz = vec3(xval, yval, zval);
  }
  reflectance_spectrum(const Real &r, const Real &g,
                       const Real &b)
      : sampled_spectrum(r, g, b,
                         SpectrumType::Reflectance),
        rgb(r, g, b) {}
  reflectance_spectrum(const Real &r)
      : sampled_spectrum(r, SpectrumType::Reflectance),
        rgb(r) {}
  Real r() const override { return rgb.r(); }
  Real g() const override { return rgb.g(); }
  Real b() const override { return rgb.b(); }
  Real x() const override { return r(); }
  Real y() const override { return g(); }
  Real z() const override { return b(); }

  static reflectance_spectrum random() {
    auto sp1 = spd<Real>();
    auto sp2 = spd<Real>();
    return reflectance_spectrum(sp1, sp2);
  }
  reflectance_spectrum
  add(const illuminant_spectrum &s) const {
    auto sspect = spect + s.spect;
    auto refspect = reflectance + s.spect;
    return reflectance_spectrum(sspect, refspect);
  }
  reflectance_spectrum
  add(const reflectance_spectrum &s) const {
    auto sspect = spect + s.spect;
    auto refspect = reflectance + s.reflectance;
    return reflectance_spectrum(sspect, refspect);
  }
  shared_ptr<sampled_spectrum>
  add(const shared_ptr<sampled_spectrum> &s) {
    switch (s->type) {
    case SpectrumType::Illuminant: {
      auto sspect = spect + s->spect;
      auto refspect = reflectance + s->spect;
      return make_shared<reflectance_spectrum>(sspect,
                                               refspect);
    }
    case SpectrumType::Reflectance: {
      auto sspect = spect + s->spect;
      auto refspect = reflectance + s->reflectance;
      return make_shared<reflectance_spectrum>(sspect,
                                               refspect);
    }
    }
  }
  reflectance_spectrum add(const Real &s) const {
    auto sspect = spect + s;
    auto reflspect = reflectance + s;
    return reflectance_spectrum(sspect, reflspect);
  }
  reflectance_spectrum
  subt(const illuminant_spectrum &s) const {
    auto sspect = spect - s.spect;
    auto refspect = reflectance - s.spect;
    return reflectance_spectrum(sspect, refspect);
  }
  reflectance_spectrum
  subt(const reflectance_spectrum &s) const {
    auto sspect = spect - s.spect;
    auto refspect = reflectance - s.reflectance;
    return reflectance_spectrum(sspect, refspect);
  }

  shared_ptr<sampled_spectrum>
  subt(const shared_ptr<sampled_spectrum> &s) {
    switch (s->type) {
    case SpectrumType::Illuminant: {
      auto sspect = spect - s->spect;
      auto refspect = reflectance - s->spect;
      return make_shared<reflectance_spectrum>(sspect,
                                               refspect);
    }
    case SpectrumType::Reflectance: {
      auto sspect = spect - s->spect;
      auto refspect = reflectance - s->reflectance;
      return make_shared<reflectance_spectrum>(sspect,
                                               refspect);
    }
    }
  }
  reflectance_spectrum subt(const Real &s) const {
    auto sspect = spect - s;
    auto reflspect = reflectance - s;
    return reflectance_spectrum(sspect, reflspect);
  }
  reflectance_spectrum
  multip(const illuminant_spectrum &s) const {
    auto sspect = spect * s.spect;
    auto reflspect = reflectance * s.spect;
    return reflectance_spectrum(sspect, reflspect);
  }
  reflectance_spectrum
  multip(const reflectance_spectrum &s) {
    auto sspect = spect * s.spect;
    auto reflspect = reflectance * s.reflectance;
    return reflectance_spectrum(sspect, reflspect);
  }
  shared_ptr<sampled_spectrum>
  multip(const shared_ptr<sampled_spectrum> &s) {
    switch (s->type) {
    case SpectrumType::Illuminant: {
      auto sspect = spect * s->spect;
      auto refspect = reflectance * s->spect;
      return make_shared<reflectance_spectrum>(sspect,
                                               refspect);
    }
    case SpectrumType::Reflectance: {
      auto sspect = spect * s->spect;
      auto refspect = reflectance * s->reflectance;
      return make_shared<reflectance_spectrum>(sspect,
                                               refspect);
    }
    }
  }
  reflectance_spectrum multip(const Real &s) const {
    auto sspect = spect * s;
    auto reflspect = reflectance * s;
    return reflectance_spectrum(sspect, reflspect);
  }
  reflectance_spectrum div(reflectance_spectrum s) {
    auto sspect = spect / s.spect;
    auto reflspect = reflectance / s.reflectance;
    return reflectance_spectrum(sspect, reflspect);
  }
  reflectance_spectrum div(illuminant_spectrum s) {
    auto sspect = spect / s.spect;
    auto reflspect = reflectance / s.spect;
    return reflectance_spectrum(sspect, reflspect);
  }
  shared_ptr<sampled_spectrum>
  div(const shared_ptr<sampled_spectrum> &s) {
    switch (s->type) {
    case SpectrumType::Illuminant: {
      auto sspect = spect / s->spect;
      auto refspect = reflectance / s->spect;
      return make_shared<reflectance_spectrum>(sspect,
                                               refspect);
    }
    case SpectrumType::Reflectance: {
      auto sspect = spect / s->spect;
      auto refspect = reflectance / s->reflectance;
      return make_shared<reflectance_spectrum>(sspect,
                                               refspect);
    }
    }
  }

  reflectance_spectrum div(const Real &s) {
    auto sspect = spect / s;
    auto reflspect = reflectance / s;
    return reflectance_spectrum(sspect, reflspect);
  }
};

typedef color spectrum;
// typedef sampled_spectrum spectrum;
// typedef rgb_spectrum spectrum;
