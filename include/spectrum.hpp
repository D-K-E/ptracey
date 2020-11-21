#pragma once
// implements spectrum type from pbrt-book
/*
   Illuminant
   X = \int S(\lambda) \bar{x}(\lambda)
   Y = \int S(\lambda) \bar{y}(\lambda)
   Z = \int S(\lambda) \bar{z}(\lambda)

   Reflectance
   X = \int R(\lambda) S(\lambda) \bar{x}(\lambda)
   Y = \int R(\lambda) S(\lambda) \bar{y}(\lambda)
   Z = \int R(\lambda) S(\lambda) \bar{z}(\lambda)
   R(\lambda) material param

 */
// mostly adapted from
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.h
#include <common.hpp>
#include <spd.hpp>
#include <specdata.hpp>
#include <utils.hpp>
#include <vec3.hpp>
#include <wave.hpp>
#include <wave.hpp>
// CONSTANTS
using namespace ptracey;
namespace ptracey {

static const auto CIE_Y_INTEGRAL = 106.856895;

// utility functions for spectrums

inline void rgb2xyz(const Real rgb[3], Real xyz[3]) {
  xyz[0] = 0.412453 * rgb[0] + 0.357580 * rgb[1] +
           0.180423 * rgb[2];
  xyz[1] = 0.212671 * rgb[0] + 0.715160 * rgb[1] +
           0.072169 * rgb[2];
  xyz[2] = 0.019334 * rgb[0] + 0.119193 * rgb[1] +
           0.950227f * rgb[2];
}

enum class SpectrumType { Reflectance, Illuminant, RGB };

class color : public vec3 {
public:
  SpectrumType type;

public:
  color() : vec3() {}
  color(Real e0, Real e1, Real e2,
        SpectrumType stype = SpectrumType::RGB)
      : vec3(e0, e1, e2), type(stype) {}
  color(Real e0, SpectrumType stype = SpectrumType::RGB)
      : vec3(e0), type(stype) {}
  color(const Real e1[3],
        SpectrumType stype = SpectrumType::RGB)
      : vec3(e1), type(stype) {}
  color(const vec3 &v,
        SpectrumType stype = SpectrumType::RGB)
      : vec3(v.x(), v.y(), v.z()), type(stype) {}
  color(const path &csv_path,
        const std::string &wave_col_name = "wavelength",
        const std::string &power_col_name = "power",
        const std::string &sep = ",",
        const unsigned int stride = SPD_STRIDE,
        SpectrumType stype = SpectrumType::RGB)
      : type(SpectrumType::RGB) {
    auto rgb_spd = spd<Real>(csv_path, wave_col_name,
                             power_col_name, sep, stride);
    auto rgb = to_color(rgb_spd);
    *this = rgb;
  }

  color evaluate(unsigned int wave_length) { return *this; }
  void update(unsigned int wave_length, const color &v) {
    *this = v;
  }
  color operator=(vec3 v) {
    return color(v.x(), v.y(), v.z());
  }
  void insert(unsigned int wl, Real v) { return; }
  shared_ptr<color> add(unsigned int wlength,
                        const shared_ptr<Real> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v + ss;
    return make_shared<color>(sv);
  }
  shared_ptr<color> add(const shared_ptr<Real> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v + ss;
    return make_shared<color>(sv);
  }

  color add(unsigned int wlength, Real s) {
    vec3 v(r(), g(), b());
    auto sv = v + s;
    return color(sv);
  }
  color add(Real s) {
    vec3 v(r(), g(), b());
    auto sv = v + s;
    return color(sv);
  }
  shared_ptr<color> add(unsigned int wlength,
                        const shared_ptr<vec3> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v + ss;
    return make_shared<color>(sv);
  }
  shared_ptr<color> add(const shared_ptr<vec3> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v + ss;
    return make_shared<color>(sv);
  }

  color add(unsigned int wlength, vec3 s) {
    vec3 v(r(), g(), b());
    auto sv = v + s;
    return color(sv);
  }
  color add(vec3 s) {
    vec3 v(r(), g(), b());
    auto sv = v + s;
    return color(sv);
  }

  shared_ptr<color> subt(unsigned int wlength,
                         const shared_ptr<Real> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v - ss;
    return make_shared<color>(sv);
  }
  shared_ptr<color> subt(const shared_ptr<Real> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v - ss;
    return make_shared<color>(sv);
  }

  color subt(unsigned int wlength, Real s) {
    vec3 v(r(), g(), b());
    auto sv = v - s;
    return color(sv);
  }
  color subt(Real s) {
    vec3 v(r(), g(), b());
    auto sv = v - s;
    return color(sv);
  }
  shared_ptr<color> subt(unsigned int wlength,
                         const shared_ptr<vec3> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v - ss;
    return make_shared<color>(sv);
  }
  shared_ptr<color> subt(const shared_ptr<vec3> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v - ss;
    return make_shared<color>(sv);
  }

  color subt(unsigned int wlength, vec3 s) {
    vec3 v(r(), g(), b());
    auto sv = v - s;
    return color(sv);
  }
  color subt(vec3 s) {
    vec3 v(r(), g(), b());
    auto sv = v - s;
    return color(sv);
  }

  shared_ptr<color>
  multip(unsigned int wlength,
         const shared_ptr<Real> &s) const {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v * ss;
    return make_shared<color>(sv);
  }
  shared_ptr<color>
  multip(const shared_ptr<Real> &s) const {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v * ss;
    return make_shared<color>(sv);
  }

  color multip(unsigned int wlength, const Real &s) const {
    vec3 v(r(), g(), b());
    auto sv = v * s;
    return color(sv);
  }
  color multip(const Real &s) const {
    vec3 v(r(), g(), b());
    auto sv = v * s;
    return color(sv);
  }

  shared_ptr<color>
  multip(unsigned int wlength,
         const shared_ptr<vec3> &s) const {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v * ss;
    return make_shared<color>(sv);
  }
  shared_ptr<color>
  multip(const shared_ptr<vec3> &s) const {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v * ss;
    return make_shared<color>(sv);
  }

  color multip(unsigned int wlength, const vec3 &s) const {
    vec3 v(r(), g(), b());
    auto sv = v * s;
    return color(sv);
  }
  color multip(const vec3 &s) const {
    vec3 v(r(), g(), b());
    auto sv = v * s;
    return color(sv);
  }

  shared_ptr<color> div(unsigned int wlength,
                        const shared_ptr<Real> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v / ss;
    return make_shared<color>(sv);
  }
  shared_ptr<color> div(const shared_ptr<Real> &s) {
    auto ss = *s;
    vec3 v(r(), g(), b());
    auto sv = v / ss;
    return make_shared<color>(sv);
  }

  color div(unsigned int wlength, Real s) {
    vec3 v(r(), g(), b());
    auto sv = v / s;
    return color(sv);
  }
  color div(Real s) {
    vec3 v(r(), g(), b());
    auto sv = v / s;
    return color(sv);
  }
  shared_ptr<color> div(unsigned int wlength,
                        const shared_ptr<vec3> &s) {
    auto ss = *s;
    vec3 v(r() / ss.r(), g() / ss.g(), b() / ss.b());
    return make_shared<color>(v);
  }
  shared_ptr<color> div(const shared_ptr<vec3> &s) {
    auto ss = *s;
    vec3 v(r() / ss.r(), g() / ss.g(), b() / ss.b());
    return make_shared<color>(v);
  }

  color div(unsigned int wlength, vec3 s) {
    vec3 v(r() / s.r(), g() / s.g(), b() / s.b());
    return color(v);
  }
  color div(vec3 s) {
    vec3 v(r() / s.r(), g() / s.g(), b() / s.b());
    return color(v);
  }
};

class sampled_spectrum {
public:
  spd<Real> spect;
  SpectrumType type;

public:
  sampled_spectrum()
      : spect(spd<Real>()),
        type(SpectrumType::Reflectance) {}
  sampled_spectrum(
      const path &csv_path,
      const std::string &wave_col_name = "wavelength",
      const std::string &power_col_name = "power",
      const std::string &sep = ",",
      const unsigned int stride = SPD_STRIDE,
      SpectrumType stype = SpectrumType::Reflectance)
      : spect(spd<Real>(csv_path, wave_col_name,
                        power_col_name, sep, stride)),
        type(stype) {}
  sampled_spectrum(const spd<Real> &s_lambda,
                   SpectrumType stype)
      : spect(s_lambda), type(stype) {}
  sampled_spectrum(
      const Real &r, const Real &g, const Real &b,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype) {
    spect = from_rgb(color(r, g, b));
  }
  sampled_spectrum(
      const Real &r,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype) {
    spect = from_rgb(color(r, r, r));
  }

  sampled_spectrum(
      const color &_rgb,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype) {
    spect = from_rgb(_rgb);
  }
  sampled_spectrum(
      const vec3 &_rgb,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype) {
    spect = from_rgb(_rgb);
  }
  spd<Real> from_rgb(const vec3 &rgb) {
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
  spd<Real> from_rgb(const color &rgb) {
    return from_rgb(vec3(rgb.r(), rgb.g(), rgb.b()));
  }
  static sampled_spectrum
  random(SpectrumType stype = SpectrumType::Reflectance) {
    auto sp1 = spd<Real>::random();
    return sampled_spectrum(sp1, stype);
  }
  static sampled_spectrum
  random(Real mn, Real mx,
         SpectrumType stype = SpectrumType::Reflectance) {
    auto sp1 = spd<Real>::random(mn, mx);
    return sampled_spectrum(sp1, stype);
  }

  void insert(unsigned int wavelength, Real power) {
    spect.insert(wavelength, power);
  }

  shared_ptr<sampled_spectrum>
  add(const shared_ptr<Real> &s) const {
    auto ss = *s;
    auto sp1 = spect + ss;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  sampled_spectrum add(const Real &s) const {
    auto sp1 = spect + s;
    return sampled_spectrum(sp1, type);
  }
  shared_ptr<sampled_spectrum>
  add(unsigned int wlength, const shared_ptr<Real> &s) {
    auto ss = *s;
    spect.add(wlength, ss);
    return make_shared<sampled_spectrum>(spect, type);
  }
  sampled_spectrum add(unsigned int wlength, Real s) {
    spect.add(wlength, s);
    return sampled_spectrum(spect, type);
  }
  shared_ptr<sampled_spectrum>
  add(const shared_ptr<sampled_spectrum> &s) const {
    auto sp1 = spect + s->spect;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  sampled_spectrum add(const sampled_spectrum &s) const {
    try {
      auto sp1 = spect + s.spect;
      return sampled_spectrum(sp1, type);
    } catch (std::runtime_error &err) {
        //
    }
  }
  shared_ptr<sampled_spectrum>
  subt(const shared_ptr<Real> &s) const {
    auto ss = *s;
    auto sp1 = spect - ss;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  shared_ptr<sampled_spectrum>
  subt(const shared_ptr<sampled_spectrum> &s) const {
    auto sp1 = spect - s->spect;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  sampled_spectrum subt(const Real &s) const {
    auto sp1 = spect - s;
    return sampled_spectrum(sp1, type);
  }
  sampled_spectrum subt(const sampled_spectrum &s) const {
    auto sp1 = spect - s.spect;
    return sampled_spectrum(sp1, type);
  }
  shared_ptr<sampled_spectrum>
  subt(unsigned int wlength, const shared_ptr<Real> &s) {
    auto ss = *s;
    spect.subt(wlength, ss);
    return make_shared<sampled_spectrum>(spect, type);
  }
  sampled_spectrum subt(unsigned int wlength, Real s) {
    spect.subt(wlength, s);
    return sampled_spectrum(spect, type);
  }

  shared_ptr<sampled_spectrum>
  multip(const shared_ptr<Real> &s) const {
    auto ss = *s;
    auto sp1 = spect * ss;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  shared_ptr<sampled_spectrum>
  multip(const shared_ptr<sampled_spectrum> &s) const {
    auto sp1 = spect * s->spect;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  sampled_spectrum multip(const Real &s) const {
    auto sp1 = spect * s;
    return sampled_spectrum(sp1, type);
  }
  sampled_spectrum multip(const sampled_spectrum &s) const {
    auto sp1 = spect * s.spect;
    return sampled_spectrum(sp1, type);
  }
  shared_ptr<sampled_spectrum>
  multip(unsigned int wlength, const shared_ptr<Real> &s) {
    auto ss = *s;
    spect.multip(wlength, ss);
    return make_shared<sampled_spectrum>(spect, type);
  }
  sampled_spectrum multip(unsigned int wlength, Real s) {
    spect.multip(wlength, s);
    return sampled_spectrum(spect, type);
  }
  sampled_spectrum multip(unsigned int wlength,
                          sampled_spectrum s) {
    spect.multip(wlength, s.spect[wlength]);
    return sampled_spectrum(spect, type);
  }

  shared_ptr<sampled_spectrum>
  div(const shared_ptr<Real> &s) const {
    auto ss = *s;
    auto sp1 = spect / ss;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  shared_ptr<sampled_spectrum>
  div(const shared_ptr<sampled_spectrum> &s) const {
    auto sp1 = spect / s->spect;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  sampled_spectrum div(const Real &s) const {
    auto sp1 = spect / s;
    return sampled_spectrum(sp1, type);
  }
  sampled_spectrum div(const sampled_spectrum &s) const {
    auto sp1 = spect / s.spect;
    return sampled_spectrum(sp1, type);
  }
  shared_ptr<sampled_spectrum>
  div(unsigned int wlength, const shared_ptr<Real> &s) {
    auto ss = *s;
    spect.div(wlength, ss);
    return make_shared<sampled_spectrum>(spect, type);
  }
  sampled_spectrum div(unsigned int wlength, Real s) {
    spect.div(wlength, s);
    return sampled_spectrum(spect, type);
  }

  Real evaluate(unsigned int wavelength) {
    return spect.eval_wavelength(wavelength);
  }

  void update(unsigned int wlength, Real value) {
    spect.update(wlength, value);
  }
  void update(unsigned int wlength,
              sampled_spectrum value) {
    spect.update(wlength, value.spect[wlength]);
  }
};

// end static variable initialize

typedef color spectrum;
// typedef sampled_spectrum spectrum;
}
