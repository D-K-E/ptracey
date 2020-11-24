#pragma once
#include <color/spd.hpp>
#include <color/specdata.hpp>
#include <color/specutils.hpp>
#include <color/wave.hpp>
#include <common.hpp>
#include <utils.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {
//

Power averageSpectrum(spd in_spd, WaveLength waveLStart,
                      WaveLength waveLEnd) {
  // adapted
  // from
  // https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.cpp
  //
  auto wavels = in_spd.wavelengths();
  auto powers = in_spd.powers();
  auto minw = in_spd.min_wave();
  auto maxw = in_spd.max_wave();
  if (waveLStart <= minw)
    return in_spd[minw];
  if (waveLEnd >= maxw)
    return in_spd[maxw];
  if ((uint)wavels.size() == 1)
    return in_spd[minw];
  //
  Power psum = 0.0;
  //
  if (waveLStart < minw)
    psum += in_spd[minw] * (minw - waveLStart);

  if (waveLEnd > maxw)
    psum += in_spd[maxw] * (maxw - waveLEnd);

  // advance to relative segment
  uint i = 0;
  while (waveLStart > wavels[i])
    i++;
  COMP_CHECK((i + 1) == (uint)wavels.size(), (i + 1),
             (uint)wavels.size());

  auto interpSeg = [](auto w, uint j, auto ws, auto ps) {
    auto t1 = (w - ws[j]) / (ws[j + 1] - ws[j]);
    auto t2 = ps[j];
    auto t3 = ps[j + 1];
    return mix(t1, t2, t3);
  };

  for (uint j = i;
       j + 1 < (uint)wavels.size() && waveLEnd >= wavels[j];
       j++) {
    auto segLambdaStart = std::max(waveLStart, wavels[j]);
    auto segLambdaEnd = std::min(waveLEnd, wavels[j + 1]);
    auto interp_seg_start =
        interpSeg(segLambdaStart, j, wavels, powers);
    auto interp_seg_end =
        interpSeg(segLambdaEnd, j, wavels, powers);
    psum += 0.5 * (interp_seg_start + interp_seg_end) *
            (segLambdaEnd - segLambdaStart);
  }
  return psum / (waveLEnd - waveLStart);
}

//

spd rgb_to_spect(std::vector<Real> rgb_wavelength,
                 std::vector<Real> rgb_spect) {
  //
  COMP_CHECK(rgb_spect.size() == rgb_wavelength.size(),
             rgb_spect.size(), rgb_wavelength.size());
  std::vector<WaveLength> waves =
      cast_vec<Real, WaveLength>(
          rgb_wavelength, [](auto rgbw) {
            return static_cast<WaveLength>(round(rgbw));
          });
  std::vector<Power> powers =
      cast_vec<Real, Power>(rgb_spect, [](auto rgbp) {
        return static_cast<Power>(rgbp);
      });

  sampled_wave sampled_powers(powers);
  auto sp = spd(sampled_powers, waves);
  return sp;
}

spd spd::rgbRefl2SpectWhite =
    rgb_to_spect(RGB2SpectLambda, RGBRefl2SpectWhite);

spd spd::rgbRefl2SpectCyan =
    rgb_to_spect(RGB2SpectLambda, RGBRefl2SpectCyan);

spd spd::rgbRefl2SpectMagenta =
    rgb_to_spect(RGB2SpectLambda, RGBRefl2SpectMagenta);

spd spd::rgbRefl2SpectYellow =
    rgb_to_spect(RGB2SpectLambda, RGBRefl2SpectYellow);

spd spd::rgbRefl2SpectRed =
    rgb_to_spect(RGB2SpectLambda, RGBRefl2SpectRed);

spd spd::rgbRefl2SpectGreen =
    rgb_to_spect(RGB2SpectLambda, RGBRefl2SpectGreen);

spd spd::rgbRefl2SpectBlue =
    rgb_to_spect(RGB2SpectLambda, RGBRefl2SpectBlue);

spd spd::rgbIllum2SpectWhite =
    rgb_to_spect(RGB2SpectLambda, RGBIllum2SpectWhite);

spd spd::rgbIllum2SpectCyan =
    rgb_to_spect(RGB2SpectLambda, RGBIllum2SpectCyan);

spd spd::rgbIllum2SpectBlue =
    rgb_to_spect(RGB2SpectLambda, RGBIllum2SpectBlue);

spd spd::rgbIllum2SpectGreen =
    rgb_to_spect(RGB2SpectLambda, RGBIllum2SpectGreen);

spd spd::rgbIllum2SpectRed =
    rgb_to_spect(RGB2SpectLambda, RGBIllum2SpectRed);

spd spd::rgbIllum2SpectMagenta =
    rgb_to_spect(RGB2SpectLambda, RGBIllum2SpectMagenta);

spd spd::rgbIllum2SpectYellow =
    rgb_to_spect(RGB2SpectLambda, RGBIllum2SpectYellow);

spd spd::X = rgb_to_spect(CIE_LAMBDA_REAL, CIE_X);

spd spd::Y = rgb_to_spect(CIE_LAMBDA_REAL, CIE_Y);

spd spd::Z = rgb_to_spect(CIE_LAMBDA_REAL, CIE_Z);

void spd::Init() {}

//

spd FromRGB(const vec3 &rgb, SpectrumType type) {
  spd r;
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
  return r.clamp();
}
}
