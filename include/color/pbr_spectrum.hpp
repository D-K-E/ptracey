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
Power averageSpectrum(const sampled_wave<Power> &powers,
                      const std::vector<WaveLength> &waves,
                      WaveLength waveLStart,
                      WaveLength waveLEnd) {
  // adapted
  // from
  // https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.cpp
  //

  uint end = (uint)waves.size() - 1;
  if (waveLStart <= waves[0])
    return powers[0];
  if (waveLEnd >= waves[end])
    return powers[end];
  if ((uint)wavels.size() == 1)
    return powers[0];
  //
  Power psum = 0.0;
  if (waveLStart < waves[0])
    psum += powers[0] * (waves[0] - waveLStart);

  if (waveLEnd > waves[end])
    psum += powers[end] * (waveLEnd - waves[end]);
  //
  uint i = 0;
  while (waveLStart > waves[i + 1])
    i++;
  COMP_CHECK((i + 1) == (uint)waves.size(), (i + 1),
             (uint)waves.size());
  auto interpSeg = [waves, powers](auto w, uint j) {
    auto t1 = (w - waves[j]) / (waves[j + 1] - waves[j]);
    auto t2 = powers[j];
    auto t3 = powers[j + 1];
    return mix(t1, t2, t3);
  };
  //
  for (uint j = i;
       j + 1 < (uint)waves.size() && waveLEnd >= waves[j];
       j++) {
    auto segLambdaStart = std::max(waveLStart, waves[j]);
    auto segLambdaEnd = std::min(waveLEnd, waves[j + 1]);
    auto interp_seg_start = interpSeg(segLambdaStart, j);
    auto interp_seg_end = interpSeg(segLambdaEnd, j);
    psum += 0.5 * (interp_seg_start + interp_seg_end) *
            (segLambdaEnd - segLambdaStart);
  }
  return psum / (waveLEnd - waveLStart);
}

Power averageSpectrum(const spd &in_spd,
                      WaveLength waveLStart,
                      WaveLength waveLEnd) {
  auto powers2 = in_spd.powers();
  auto waves = in_spd.wavelengths();
  return averageSpectrum(powers2, waves, waveLStart,
                         waveLEnd);
}

void resample_wave_power(
    const sampled_wave<Power> &in_powers,
    const std::vector<WaveLength> &in_waves,
    sampled_wave<Power> &out_powers,
    std::vector<WaveLength> &out_waves,
    WaveLength waveLStart, WaveLength waveLEnd,
    uint out_size) {
  //
  uint wsize = (uint)in_waves.size();
  COMP_CHECK(wsize > 2, wsize, 2);
  auto mwave = waveLStart;
  auto miwave = waveLEnd;
  auto waves = in_waves;
  auto pwrs = in_powers;
  //
  WaveLength delta = (mwave - miwave) / (wsize - 1);

  //
  auto wlstartClamp = [miwave, mwave, waves, wsize,
                       delta](int index) -> WaveLength {
    //
    COMP_CHECK(index >= -1 && index <= wsize, index, wsize);
    if (index == -1) {
      //
      COMP_CHECK(miwave - delta < waves[0], miwave - delta,
                 waves[0]);
      return miwave - delta;
    } else if (index == wsize) {
      COMP_CHECK(mwave + delta > waves[wsize - 1],
                 mwave + delta, waves[wsize - 1]);
      return mwave + delta;
    }
    return waves[index];
  };
  auto pwClamped = [wsize, pwrs](int index) -> Power {
    COMP_CHECK(index >= -1 && index <= wsize, index, wsize);
    return pwrs[dclamp<uint>(index, 0, wsize - 1)];
  };
  //
  auto samplingfn = [waves, pwrs, delta, wsize, pwClamped,
                     wlstartClamp](WaveLength wl) -> Power {
    if (wl + delta / 2 <= waves[0])
      return pwrs[0];
    if (wl - delta / 2 >= waves[wsize - 1])
      return pwrs[wsize - 1];
    //
    if (wsize == 1)
      return pwrs[0];
    //

    int start, end;

    if (wl - delta < waves[0]) {
      start = -1;
    } else {
      auto intervalfn = [waves, delta, wl](int i) -> bool {
        return waves[i] <= wl - delta;
      };
      start = findInterval(wsize, intervalfn);
      bool sb1 = start >= 0;
      bool sb2 = start < wsize;
      COMP_CHECK(sb1 && sb2, start, wsize);
    }
    //
    if (wl + delta > waves[wsize - 1]) {
      end = (int)wsize;
    } else {
      end = start > 0 ? start : 0;
      while (end < (int)wsize && wl + delta > waves[end])
        end++;
    }
    bool cond1 = end - start == 2;
    bool cond2 = (wlstartClamp(start) <= (wl - delta));
    bool cond3 = waves[start + 1] == wl;
    bool cond4 = wlstartClamp(end) >= wl + delta;
    if (cond1 && cond2 && cond3 && cond4) {
      return pwrs[start + 1];
    } else if (end - start == 1) {
      //
      WaveLength t =
          (wl - wlstartClamp(start)) /
          (wlstartClamp(end) - wlstartClamp(start));
      COMP_CHECK(t >= 0 && t <= 1, t, t);
      return mix(t, pwClamped(start), pwClamped(end));
    } else {
      return averageSpectrum(pwrs, waves, wl - delta / 2,
                             wl + delta / 2);
    }
  };

  for (int outOffset = 0; outOffset < out_size;
       outOffset++) {
    //
    auto wave = mix<WaveLength>(WaveLength(outOffset) /
                                    (out_size - 1),
                                waveLStart, waveLEnd);
    out_waves.push_back(wave);
    Power pwr = sample(
  }
}

spd spd::resample_c(const spd &s) const {
  //
  auto waves1 = wavelengths();
  auto waves2 = s.wavelengths();
  auto powers1 = powers();
  auto powers2 = s.powers();

  auto wsize1 = waves1.size();
  auto wsize2 = waves2.size();
  auto wsize = wsize1 > wsize2 ? wsize1 : wsize2;
  COMP_CHECK(wsize > 2, wsize, 2);
  //
  auto waves = wsize == wsize1 ? waves1 : waves2;
  auto pwrs = wsize == wsize1 ? powers1 : powers2;
  //
  auto mwave1 = max_wave();
  auto mwave2 = s.max_wave();
  auto mwave = mwave1 > mwave2 ? mwave1 : mwave2;
  //
  auto miwave1 = min_wave();
  auto miwave2 = s.min_wave();
  auto miwave = miwave1 > miwave2 ? miwave1 : miwave2;
  //
  WaveLength delta = (mwave - miwave) / (wsize - 1);
  //
  auto wlstartClamp = [miwave, mwave, waves, wsize,
                       delta](int index) -> WaveLength {
    //
    COMP_CHECK(index >= -1 && index <= wsize, index, wsize);
    if (index == -1) {
      //
      COMP_CHECK(miwave - delta < waves[0], miwave - delta,
                 waves[0]);
      return miwave - delta;
    } else if (index == wsize) {
      COMP_CHECK(mwave + delta > waves[wsize - 1],
                 mwave + delta, waves[wsize - 1]);
      return mwave + delta;
    }
    return waves[index];
  };

  //
  auto pwClamped = [wsize, pwrs](int index) -> Power {
    COMP_CHECK(index >= -1 && index <= wsize, index, wsize);
    return pwrs[dclamp<uint>(index, 0, wsize - 1)];
  };

  auto sample = [waves, pwrs, delta, wsize, pwClamped,
                 wlstartClamp](WaveLength wl) -> Power {
    if (wl + delta / 2 <= waves[0])
      return pwrs[0];
    if (wl - delta / 2 >= waves[wsize - 1])
      return pwrs[wsize - 1];
    //
    if (wsize == 1)
      return pwrs[0];
    //

    int start, end;

    if (wl - delta < waves[0]) {
      start = -1;
    } else {
      auto intervalfn = [waves, delta, wl](int i) -> bool {
        return waves[i] <= wl - delta;
      };
      start = findInterval(wsize, intervalfn);
      bool sb1 = start >= 0;
      bool sb2 = start < wsize;
      COMP_CHECK(sb1 && sb2, start, wsize);
    }
    //
    if (wl + delta > waves[wsize - 1]) {
      end = (int)wsize;
    } else {
      end = start > 0 ? start : 0;
      while (end < (int)wsize && wl + delta > waves[end])
        end++;
    }
    //
    bool cond1 = end - start == 2;
    bool cond2 = (wlstartClamp(start) <= (wl - delta));
    bool cond3 = waves[start + 1] == wl;
    bool cond4 = wlstartClamp(end) >= wl + delta;
    if (cond1 && cond2 && cond3 && cond4) {
      return pwrs[start + 1];
    } else if (end - start == 1) {
      //
      WaveLength t =
          (wl - wlstartClamp(start)) /
          (wlstartClamp(end) - wlstartClamp(start));
      COMP_CHECK(t >= 0 && t <= 1, t, t);
      return mix(t, pwClamped(start), pwClamped(end));
    } else {
      //
      return averageSpectrum();
    }
  };
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
