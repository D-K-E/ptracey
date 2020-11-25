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

spd FromRGB(const vec3 &rgb, SpectrumType type) {
  spd r = spd::zeros_like(spd::rgbRefl2SpectWhite);
  if (type == SpectrumType::Reflectance) {
    // Convert reflectance spectrum to RGB
    if (rgb.x() <= rgb.y() && rgb.x() <= rgb.z()) {
      // Compute reflectance _SampledSpectrum_ with _rgb[0]_
      // as minimum
      r += rgb.x() * spd::rgbRefl2SpectWhite;
      if (rgb.y() <= rgb.z()) {
        r += (rgb.y() - rgb.x()) * spd::rgbRefl2SpectCyan;
        r += (rgb.z() - rgb.y()) * spd::rgbRefl2SpectBlue;
      } else {
        r += (rgb.z() - rgb.x()) * spd::rgbRefl2SpectCyan;
        r += (rgb.y() - rgb.z()) * spd::rgbRefl2SpectGreen;
      }
    } else if (rgb.y() <= rgb.x() && rgb.y() <= rgb.z()) {
      // Compute reflectance _SampledSpectrum_ with _rgb[1]_
      // as minimum
      r += rgb.y() * spd::rgbRefl2SpectWhite;
      if (rgb.x() <= rgb.z()) {
        r +=
            (rgb.x() - rgb.y()) * spd::rgbRefl2SpectMagenta;
        r += (rgb.z() - rgb.x()) * spd::rgbRefl2SpectBlue;
      } else {
        r +=
            (rgb.z() - rgb.y()) * spd::rgbRefl2SpectMagenta;
        r += (rgb.x() - rgb.z()) * spd::rgbRefl2SpectRed;
      }
    } else {
      // Compute reflectance _SampledSpectrum_ with _rgb[2]_
      // as minimum
      r += rgb.z() * spd::rgbRefl2SpectWhite;
      if (rgb.x() <= rgb.y()) {
        r += (rgb.x() - rgb.z()) * spd::rgbRefl2SpectYellow;
        r += (rgb.y() - rgb.x()) * spd::rgbRefl2SpectGreen;
      } else {
        r += (rgb.y() - rgb.z()) * spd::rgbRefl2SpectYellow;
        r += (rgb.x() - rgb.y()) * spd::rgbRefl2SpectRed;
      }
    }
    r *= .94;
  } else {
    // Convert illuminant spectrum to RGB
    if (rgb.x() <= rgb.y() && rgb.x() <= rgb.z()) {
      // Compute illuminant _SampledSpectrum_ with _rgb[0]_
      // as minimum
      r += rgb.x() * spd::rgbIllum2SpectWhite;
      if (rgb.y() <= rgb.z()) {
        r += (rgb.y() - rgb.x()) * spd::rgbIllum2SpectCyan;
        r += (rgb.z() - rgb.y()) * spd::rgbIllum2SpectBlue;
      } else {
        r += (rgb.z() - rgb.x()) * spd::rgbIllum2SpectCyan;
        r += (rgb.y() - rgb.z()) * spd::rgbIllum2SpectGreen;
      }
    } else if (rgb.y() <= rgb.x() && rgb.y() <= rgb.z()) {
      // Compute illuminant _SampledSpectrum_ with _rgb[1]_
      // as minimum
      r += rgb.y() * spd::rgbIllum2SpectWhite;
      if (rgb.x() <= rgb.z()) {
        r += (rgb.x() - rgb.y()) *
             spd::rgbIllum2SpectMagenta;
        r += (rgb.z() - rgb.x()) * spd::rgbIllum2SpectBlue;
      } else {
        r += (rgb.z() - rgb.y()) *
             spd::rgbIllum2SpectMagenta;
        r += (rgb.x() - rgb.z()) * spd::rgbIllum2SpectRed;
      }
    } else {
      // Compute illuminant _SampledSpectrum_ with _rgb.z()_
      // as minimum
      r += rgb.z() * spd::rgbIllum2SpectWhite;
      if (rgb.x() <= rgb.y()) {
        r +=
            (rgb.x() - rgb.z()) * spd::rgbIllum2SpectYellow;
        r += (rgb.y() - rgb.x()) * spd::rgbIllum2SpectGreen;
      } else {
        r +=
            (rgb.y() - rgb.z()) * spd::rgbIllum2SpectYellow;
        r += (rgb.x() - rgb.y()) * spd::rgbIllum2SpectRed;
      }
    }
    r *= 0.86445;
  }
  auto sp = r.clamp();
  auto minw = sp.min_wave();
  auto maxw = sp.max_wave();
  sp.resample(minw, maxw, SPD_NB_SAMPLE);
  return sp;
}

class sampled_spectrum : public colorable {
public:
  spd spect; //
  //
  spd sX, sY, sZ;

  // spd sRgbRefl2SpectWhite, sRgbRefl2SpectCyan;
  // spd sRgbRefl2SpectMagenta, sRgbRefl2SpectYellow;
  // spd sRgbRefl2SpectRed, sRgbRefl2SpectGreen;
  // spd sRgbRefl2SpectBlue;
  // spd sRgbIllum2SpectWhite, sRgbIllum2SpectCyan;
  // spd sRgbIllum2SpectMagenta, sRgbIllum2SpectYellow;
  // spd sRgbIllum2SpectRed, sRgbIllum2SpectGreen;
  // spd sRgbIllum2SpectBlue;

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
        type(stype) {
    Init();
  }
  sampled_spectrum(const spd &s_lambda, SpectrumType stype)
      : spect(s_lambda), type(stype) {}
  sampled_spectrum(
      const Real &r, const Real &g, const Real &b,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype) {
    spect = from_rgb(rgb_model(r, g, b));
    Init();
  }
  sampled_spectrum(
      const Real &r,
      SpectrumType stype = SpectrumType::Reflectance)
      : type(stype) {
    spect = from_rgb(rgb_model(r, r, r));
    Init();
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
    Init();
  }
  spd _from_rgb(const vec3 &rgb) {
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

  spd from_rgb(const vec3 &rgb) {
    //
    return FromRGB(rgb, type);
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
    vec3 xyz(0.0);
    auto waves = spect.wavelengths();
    for (auto wave : waves) {
      xyz.e[0] = sX[wave] * spect[wave];
      xyz.e[1] = sY[wave] * spect[wave];
      xyz.e[2] = sZ[wave] * spect[wave];
    }
    auto wsize = (uint)waves.size();
    auto scale = (spect.wave_end - spect.wave_start) /
                 Real(CIE_Y_integral * wsize);
    xyz *= scale;
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
  void Init() {
    //
    auto wsize = (uint)spect.wavelengths().size();
    auto wstart = spect.wave_start;
    auto wend = spect.wave_end;
    sX = spd::X.resample_c(wstart, wend, wsize);
    sY = spd::Y.resample_c(wstart, wend, wsize);
    sZ = spd::Z.resample_c(wstart, wend, wsize);

    // sRgbRefl2SpectWhite =
    //     spd::rgbRefl2SpectWhite.resample_c(wstart, wend,
    //                                        wsize);
    // sRgbRefl2SpectBlue =
    // spd::rgbRefl2SpectBlue.resample_c(
    //     wstart, wend, wsize);
    // sRgbRefl2SpectGreen =
    //     spd::rgbRefl2SpectGreen.resample_c(wstart, wend,
    //                                        wsize);
    // sRgbRefl2SpectRed = spd::rgbRefl2SpectRed.resample_c(
    //     wstart, wend, wsize);

    // sRgbRefl2SpectMagenta =
    //     spd::rgbRefl2SpectMagenta.resample_c(wstart,
    //     wend,
    //                                          wsize);
    // sRgbRefl2SpectCyan =
    // spd::rgbRefl2SpectCyan.resample_c(
    //     wstart, wend, wsize);

    // sRgbIllum2SpectWhite =
    //     spd::rgbIllum2SpectWhite.resample_c(wstart, wend,
    //                                         wsize);
    // sRgbIllum2SpectBlue =
    //     spd::rgbIllum2SpectBlue.resample_c(wstart, wend,
    //                                        wsize);
    // sRgbIllum2SpectGreen =
    //     spd::rgbIllum2SpectGreen.resample_c(wstart, wend,
    //                                         wsize);
    // sRgbIllum2SpectRed =
    // spd::rgbIllum2SpectRed.resample_c(
    //     wstart, wend, wsize);

    // sRgbIllum2SpectMagenta =
    //     spd::rgbIllum2SpectMagenta.resample_c(wstart,
    //     wend,
    //                                           wsize);
    // sRgbIllum2SpectCyan =
    //     spd::rgbIllum2SpectCyan.resample_c(wstart, wend,
    //                                        wsize);
  }
};
inline std::ostream &
operator<<(std::ostream &out, const sampled_spectrum &ss) {
  return out << ss.spect << std::endl;
}
}
