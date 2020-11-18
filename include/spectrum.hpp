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
// CONSTANTS
using namespace ptracey;
namespace ptracey {

static const auto CIE_Y_INTEGRAL = 106.856895;

path CSV_PARENT = "./media/data/spectrum";
const std::string WCOL_NAME = "wavelength";
const std::string PCOL_NAME = "power";
const std::string SEP = ",";

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

  shared_ptr<sampled_spectrum>
  add(const shared_ptr<Real> &s) const {
    auto ss = *s;
    auto sp1 = spect + ss;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  shared_ptr<sampled_spectrum>
  add(const shared_ptr<sampled_spectrum> &s) const {
    auto sp1 = spect + s->spect;
    return make_shared<sampled_spectrum>(sp1, type);
  }
  sampled_spectrum add(const Real &s) const {
    auto sp1 = spect + s;
    return sampled_spectrum(sp1, type);
  }
  sampled_spectrum add(const sampled_spectrum &s) const {
    auto sp1 = spect + s.spect;
    return sampled_spectrum(sp1, type);
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

public:
  static spd<Real> cie_xbar;
  static spd<Real> cie_ybar;
  static spd<Real> cie_zbar;
  static spd<Real> rho_r;
  static spd<Real> rho_g;
  static spd<Real> rho_b;
  static spd<Real> standard_d65;
};

auto rho_rspd =
    spd<Real>(CSV_PARENT / "rho-r-2012.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);

spd<Real> sampled_spectrum::rho_r = rho_rspd.normalized();
auto rho_gspd =
    spd<Real>(CSV_PARENT / "rho-g-2012.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);
spd<Real> sampled_spectrum::rho_g = rho_gspd.normalized();
auto rho_bspd =
    spd<Real>(CSV_PARENT / "rho-b-2012.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);
spd<Real> sampled_spectrum::rho_b = rho_bspd.normalized();

auto cie_xbarspd =
    spd<Real>(CSV_PARENT / "cie-x-bar-1964.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);
spd<Real> sampled_spectrum::cie_xbar =
    cie_xbarspd.normalized();

auto cie_ybarspd =
    spd<Real>(CSV_PARENT / "cie-y-bar-1964.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);
spd<Real> sampled_spectrum::cie_ybar =
    cie_ybarspd.normalized();

auto cie_zbarspd =
    spd<Real>(CSV_PARENT / "cie-z-bar-1964.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);
spd<Real> sampled_spectrum::cie_zbar =
    cie_zbarspd.normalized();

auto stand_d65 =
    spd<Real>(CSV_PARENT / "cie-d65-standard.csv",
              WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
spd<Real> sampled_spectrum::standard_d65 =
    stand_d65.normalized();

// end static variable initialize

// typedef color spectrum;
typedef sampled_spectrum spectrum;
}
