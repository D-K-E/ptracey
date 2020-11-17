#pragma once
// implements spectrum type from pbrt-book
// mostly adapted from
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.h
#include <common.hpp>
#include <spd.hpp>
#include <specdata.hpp>
#include <utils.hpp>
#include <vec3.hpp>
#include <wave.hpp>
// CONSTANTS
using namespace ptracey;
namespace ptracey {
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
  vec3 xyz;
  color rgb;

public:
  sampled_spectrum()
      : spect(spd<Real>()), type(SpectrumType::Reflectance),
        reflectance(spd<Real>()) {}
  sampled_spectrum(const spd<Real> &s)
      : spect(s), type(SpectrumType::Illuminant) {
    update_spect();
  }
  sampled_spectrum(const spd<Real> &s, const spd<Real> &r)
      : spect(s), reflectance(r),
        type(SpectrumType::Reflectance) {
    update_refl_spect();
  }
  void update_spect() {
    auto X = get_cie_val(spect, cie_xbar);
    auto Y = get_cie_val(spect, cie_ybar);
    auto Z = get_cie_val(spect, cie_zbar);
    auto xyzsum = X + Y + Z;
    xyzsum = xyzsum == 0 ? 1 : xyzsum;
    auto xval = X / xyzsum;
    auto yval = Y / xyzsum;
    auto zval = 1.0 - (xval + yval);
    xyz = vec3(xval, yval, zval);
  }
  void update_refl_spect() {
    auto X = get_cie_val(spect, reflectance, cie_xbar);
    auto Y = get_cie_val(spect, reflectance, cie_ybar);
    auto Z = get_cie_val(spect, reflectance, cie_zbar);
    auto xyzsum = X + Y + Z;
    xyzsum = xyzsum == 0 ? 1 : xyzsum;
    auto xval = X / xyzsum;
    auto yval = Y / xyzsum;
    auto zval = 1.0 - (xval + yval);
    xyz = vec3(xval, yval, zval);
  }
  sampled_spectrum(
      const Real &r, const Real &g, const Real &b,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype), rgb(color(r, g, b)) {
    switch (type) {
    case SpectrumType::Illuminant: {
      spect = from_rgb(color(r, g, b));
      update_spect();
    }
    case SpectrumType::Reflectance: {
      reflectance = from_rgb(color(r, g, b));
      spect = sampled_spectrum::standard_d65;
      update_refl_spect();
    }
    }
  }
  sampled_spectrum(
      const Real &r,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype), rgb(color(r)) {
    switch (type) {
    case SpectrumType::Illuminant: {
      spect = from_rgb(color(r, r, r));
      update_spect();
    }
    case SpectrumType::Reflectance: {
      reflectance = from_rgb(color(r, r, r));
      spect = standard_d65;
      update_refl_spect();
    }
    }
  }

  sampled_spectrum(
      const color &_rgb,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype), rgb(_rgb) {
    switch (type) {
    case SpectrumType::Illuminant: {
      spect = from_rgb(_rgb);
      update_spect();
    }
    case SpectrumType::Reflectance: {
      reflectance = from_rgb(_rgb);
      spect = standard_d65;
      update_refl_spect();
    }
    }
  }

  spd<Real> from_rgb(const color &rgb) {
    // from
    // http://scottburns.us/fast-rgb-to-spectrum-conversion-for-reflectances/
    // convert sRGB to linear rgb in range [0,1]
    Real rgbm = rgb.max();
    Real ins, ine;
    if (rgbm > 1.0) {
      ins = 0.0;
      ine = 256.0;
    } else {
      ins = 0.0;
      ine = 1.0;
    }
    auto lr = interp<Real>(rgb.r(), ins, ine, 0.0, 1.0);
    auto lg = interp<Real>(rgb.g(), ins, ine, 0.0, 1.0);
    auto lb = interp<Real>(rgb.b(), ins, ine, 0.0, 1.0);
    //
    auto rho_r_wave = rho_r.power();
    auto rho_g_wave = rho_g.power();
    auto rho_b_wave = rho_b.power();
    auto rho = lr * rho_r_wave;
    rho += lg * rho_g_wave;
    rho += lb * rho_b_wave;
    return spd<Real>(rho, rho_r.wavelengths());
  }
  Real r() const override { return rgb.r(); }
  Real g() const override { return rgb.g(); }
  Real b() const override { return rgb.b(); }
  Real x() const override { return x(); }
  Real y() const override { return y(); }
  Real z() const override { return z(); }

  static sampled_spectrum
  random(SpectrumType stype = SpectrumType::Reflectance) {
    switch (stype) {
    case SpectrumType::Illuminant: {
      auto sp1 = spd<Real>::random();
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spd<Real>::random();
      auto sp2 = spd<Real>::random();
      return sampled_spectrum(sp1, sp2);
    }
    }
  }
  static sampled_spectrum
  random(Real mn, Real mx,
         SpectrumType stype = SpectrumType::Reflectance) {
    switch (stype) {
    case SpectrumType::Illuminant: {
      auto sp1 = spd<Real>::random(mn, mx);
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spd<Real>::random(mn, mx);
      auto sp2 = spd<Real>::random(mn, mx);
      return sampled_spectrum(sp1, sp2);
    }
    }
  }
  void get_spds(spd<Real> &sspect,
                spd<Real> &rspect) const {
    switch (type) {
    case SpectrumType::Illuminant: {
      sspect = spect;
      rspect = spect;
    }
    case SpectrumType::Reflectance: {
      sspect = spect;
      rspect = reflectance;
    }
    }
  }

  shared_ptr<sampled_spectrum>
  add(const shared_ptr<Real> &s) const {
    auto ss = *s;
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect + ss;
      return make_shared<sampled_spectrum>(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect + ss;
      auto sp2 = reflectance + ss;
      return make_shared<sampled_spectrum>(sp1, sp2);
    }
    }
  }
  shared_ptr<sampled_spectrum>
  add(const shared_ptr<sampled_spectrum> &s) const {

    spd<Real> sspect;
    spd<Real> rspect;
    s->get_spds(sspect, rspect);
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect + sspect;
      return make_shared<sampled_spectrum>(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect + sspect;
      auto sp2 = reflectance + rspect;
      return make_shared<sampled_spectrum>(sp1, sp2);
    }
    }
  }
  sampled_spectrum add(const Real &s) const {
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect + s;
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect + s;
      auto sp2 = reflectance + s;
      return sampled_spectrum(sp1, sp2);
    }
    }
  }
  sampled_spectrum add(const sampled_spectrum &s) const {
    spd<Real> sspect;
    spd<Real> rspect;
    s.get_spds(sspect, rspect);
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect + sspect;
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect + sspect;
      auto sp2 = reflectance + rspect;
      return sampled_spectrum(sp1, sp2);
    }
    }
  }

  shared_ptr<sampled_spectrum>
  subt(const shared_ptr<Real> &s) const {
    auto ss = *s;
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect - ss;
      return make_shared<sampled_spectrum>(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect - ss;
      auto sp2 = reflectance - ss;
      return make_shared<sampled_spectrum>(sp1, sp2);
    }
    }
  }
  shared_ptr<sampled_spectrum>
  subt(const shared_ptr<sampled_spectrum> &s) const {

    spd<Real> sspect;
    spd<Real> rspect;
    s->get_spds(sspect, rspect);
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect - sspect;
      return make_shared<sampled_spectrum>(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect - sspect;
      auto sp2 = reflectance - rspect;
      return make_shared<sampled_spectrum>(sp1, sp2);
    }
    }
  }
  sampled_spectrum subt(const Real &s) const {
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect - s;
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect - s;
      auto sp2 = reflectance - s;
      return sampled_spectrum(sp1, sp2);
    }
    }
  }
  sampled_spectrum subt(const sampled_spectrum &s) const {
    spd<Real> sspect;
    spd<Real> rspect;
    s.get_spds(sspect, rspect);
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect - sspect;
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect - sspect;
      auto sp2 = reflectance - rspect;
      return sampled_spectrum(sp1, sp2);
    }
    }
  }

  shared_ptr<sampled_spectrum>
  multip(const shared_ptr<Real> &s) const {
    auto ss = *s;
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect * ss;
      return make_shared<sampled_spectrum>(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect * ss;
      auto sp2 = reflectance * ss;
      return make_shared<sampled_spectrum>(sp1, sp2);
    }
    }
  }
  shared_ptr<sampled_spectrum>
  multip(const shared_ptr<sampled_spectrum> &s) const {

    spd<Real> sspect;
    spd<Real> rspect;
    s->get_spds(sspect, rspect);
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect * sspect;
      return make_shared<sampled_spectrum>(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect * sspect;
      auto sp2 = reflectance * rspect;
      return make_shared<sampled_spectrum>(sp1, sp2);
    }
    }
  }
  sampled_spectrum multip(const Real &s) const {
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect * s;
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect * s;
      auto sp2 = reflectance * s;
      return sampled_spectrum(sp1, sp2);
    }
    }
  }
  sampled_spectrum multip(const sampled_spectrum &s) const {
    spd<Real> sspect;
    spd<Real> rspect;
    s.get_spds(sspect, rspect);
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect * sspect;
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect * sspect;
      auto sp2 = reflectance * rspect;
      return sampled_spectrum(sp1, sp2);
    }
    }
  }

  shared_ptr<sampled_spectrum>
  div(const shared_ptr<Real> &s) const {
    auto ss = *s;
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect / ss;
      return make_shared<sampled_spectrum>(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect / ss;
      auto sp2 = reflectance / ss;
      return make_shared<sampled_spectrum>(sp1, sp2);
    }
    }
  }
  shared_ptr<sampled_spectrum>
  div(const shared_ptr<sampled_spectrum> &s) const {

    spd<Real> sspect;
    spd<Real> rspect;
    s->get_spds(sspect, rspect);
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect / sspect;
      return make_shared<sampled_spectrum>(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect / sspect;
      auto sp2 = reflectance / rspect;
      return make_shared<sampled_spectrum>(sp1, sp2);
    }
    }
  }
  sampled_spectrum div(const Real &s) const {
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect / s;
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect / s;
      auto sp2 = reflectance / s;
      return sampled_spectrum(sp1, sp2);
    }
    }
  }
  sampled_spectrum div(const sampled_spectrum &s) const {
    spd<Real> sspect;
    spd<Real> rspect;
    s.get_spds(sspect, rspect);
    switch (type) {
    case SpectrumType::Illuminant: {
      auto sp1 = spect / sspect;
      return sampled_spectrum(sp1);
    }
    case SpectrumType::Reflectance: {
      auto sp1 = spect / sspect;
      auto sp2 = reflectance / rspect;
      return sampled_spectrum(sp1, sp2);
    }
    }
  }

protected:
  static spd<Real> cie_xbar;
  static spd<Real> cie_ybar;
  static spd<Real> cie_zbar;
  static spd<Real> rho_r;
  static spd<Real> rho_g;
  static spd<Real> rho_b;
  static spd<Real> standard_d65;
};

path csv_parent = "./media/data/spectrum";
auto rho_rspd = spd<Real>(csv_parent / "rho-r-2012.csv");
spd<Real> sampled_spectrum::rho_r = rho_rspd.normalized();
auto rho_gspd = spd<Real>(csv_parent / "rho-g-2012.csv");
spd<Real> sampled_spectrum::rho_g = rho_gspd.normalized();
auto rho_bspd = spd<Real>(csv_parent / "rho-b-2012.csv");
spd<Real> sampled_spectrum::rho_b = rho_bspd.normalized();

auto cie_xbarspd =
    spd<Real>(csv_parent / "cie-x-bar-1964.csv");
spd<Real> sampled_spectrum::cie_xbar =
    cie_xbarspd.normalized();

auto cie_ybarspd =
    spd<Real>(csv_parent / "cie-y-bar-1964.csv");
spd<Real> sampled_spectrum::cie_ybar =
    cie_ybarspd.normalized();

auto cie_zbarspd =
    spd<Real>(csv_parent / "cie-z-bar-1964.csv");
spd<Real> sampled_spectrum::cie_zbar =
    cie_zbarspd.normalized();

auto stand_d65 =
    spd<Real>(csv_parent / "cie-d65-standard.csv");
spd<Real> sampled_spectrum::standard_d65 =
    stand_d65.normalized();

// end static variable initialize

typedef color spectrum;
// typedef sampled_spectrum spectrum;
}
