#pragma once
// sampled spectrum object
#include <color/color.hpp>
#include <color/colorable.hpp>
#include <color/pbr_spectrum.hpp>
#include <color/spd.hpp>
#include <color/specdata.hpp>
#include <color/specutils.hpp>
#include <color/wave.hpp>
#include <common.hpp>
#include <utils.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {
class sampled_spectrum : public colorable {
public:
  spd spect;
  SpectrumType type;

public:
  sampled_spectrum()
      : spect(spd()), type(SpectrumType::Reflectance) {}
  sampled_spectrum(
      const path &csv_path,
      const std::string &wave_col_name = "wavelength",
      const std::string &power_col_name = "power",
      const char &sep = ',',
      const unsigned int stride = SPD_STRIDE,
      SpectrumType stype = SpectrumType::Reflectance)
      : spect(spd(csv_path, wave_col_name, power_col_name,
                  sep, stride)),
        type(stype) {}
  sampled_spectrum(const spd &s_lambda, SpectrumType stype)
      : spect(s_lambda), type(stype) {}
  sampled_spectrum(
      const Real &r, const Real &g, const Real &b,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype) {
    spect = from_rgb(rgb_model(r, g, b));
  }
  sampled_spectrum(
      const Real &r,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype) {
    spect = from_rgb(rgb_model(r, r, r));
  }
  sampled_spectrum(
      const rgb_model &_rgb,
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
  spd from_rgb(const vec3 &rgb) {
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
    auto rho_r_wave = spd::rho_r.powers();
    auto rho_g_wave = spd::rho_g.powers();
    auto rho_b_wave = spd::rho_b.powers();
    auto rho = lr * rho_r_wave;
    rho += lg * rho_g_wave;
    rho += lb * rho_b_wave;
    return spd(rho, spd::rho_r.wavelengths());
  }
  spd from_rgb(const rgb_model &rgb) {
    return from_rgb(vec3(rgb.r(), rgb.g(), rgb.b()));
  }
  static sampled_spectrum
  random(SpectrumType stype = SpectrumType::Reflectance) {
    auto sp1 = spd::random();
    return sampled_spectrum(sp1, stype);
  }
  static sampled_spectrum
  random(Real mn, Real mx,
         SpectrumType stype = SpectrumType::Reflectance) {
    auto sp1 = spd::random(mn, mx);
    return sampled_spectrum(sp1, stype);
  }
  void insert(unsigned int wavelength, Real power) {
    spect.insert(wavelength, power);
  }
  vec3 to_xyz() const override {
    vec3 xyz;
    if (type == SpectrumType::Reflectance) {
      get_cie_values(spd::standard_d65, spect, xyz);
    } else {
      get_cie_values(spect, xyz);
      // get_cie_values(spect, xyz);
    }
    return xyz;
  }
  Power evaluate(const WaveLength &w) const {
    return spect[w];
  }
  void add(const color &r_color, const WaveLength &w) {
    spect.add(w, r_color.pdata);
  }
  void scale(Real coeff) { spect.scale(coeff); }
  vec3 to_rgb() const override {
    return xyz2rgb_cie(to_xyz());
  }
};
inline std::ostream &
operator<<(std::ostream &out, const sampled_spectrum &ss) {
  return out << ss.spect << std::endl;
}
}
