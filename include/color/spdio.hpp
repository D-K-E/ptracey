#pragma once

#include <color/spectrum.hpp>
#include <color/specutils.hpp>
#include <color/specutils.hpp>
#include <common.hpp>
#include <string>
#include <thirdparty/json.hpp>
#include <thirdparty/spb/spbwriter.hpp>

using namespace ptracey;
using json = nlohmann::json;

namespace ptracey {

using imspec = std::vector<std::vector<sampled_spectrum>>;

class SpbIo {
public:
  imspec imgspec;

public:
  SpbIo(const imspec &s) : imgspec(s) {
    for (auto &imcol : imgspec) {
      for (auto &spec : imcol) {
        spec.spect.resample(VISIBLE_LAMBDA_START,
                            VISIBLE_LAMBDA_END,
                            SPD_NB_SAMPLE);
      }
    }
  }
  SpbIo(const imspec &s, uint wstart, uint wend,
        uint sample_nb)
      : imgspec(s) {
    for (auto &imcol : imgspec) {
      for (auto &spec : imcol) {
        spec.spect.resample(wstart, wend, sample_nb);
      }
    }
  }
  float first_wave() {
    float s = FLT_MAX;
    for (const auto &imcol : imgspec) {
      for (const auto &spec : imcol) {
        auto sw = spec.spect.min_wave();
        if (sw < s) {
          s = sw;
        }
      }
    }
    return s;
  }
  float last_wave() {
    float s = FLT_MIN;
    for (const auto &imcol : imgspec) {
      for (const auto &spec : imcol) {
        auto sw = spec.spect.max_wave();
        if (sw > s) {
          s = sw;
        }
      }
    }
    return s;
  }
  uint32_t nb_channels() {
    return (uint32_t)imgspec[0][0].spect.size();
  }
  float wavelength_resolution() {
    auto swaves = imgspec[0][0].spect.wavelengths();
    return (float)(swaves[1] - swaves[0]);
  }
  std::vector<float>
  data(const std::function<float(Power)> &fn = [](auto i) {
    return (float)(i * 1.0f);
  }) {
    std::vector<float> vd;
    int colsize = (int)imgspec[0].size();
    int rowsize = (int)imgspec.size();
    for (int c = 0; c < colsize; c++) {
      for (int r = 0; r < rowsize; r++) {
        for (const auto &pwr :
             imgspec[r][c].spect.powers().values) {
          vd.push_back(fn(pwr));
        }
      }
    }
    return vd;
  }
};

class SpdJsonIo {
public:
  imspec imgspec;
  Real antialiasing_coeff;

public:
  SpdJsonIo(const imspec &img, Real coeff)
      : imgspec(img), antialiasing_coeff(coeff) {}
  json pixel_obj(int x, int y, sampled_spectrum &s) {
    s.scale(antialiasing_coeff);
    json j_map(s.spect.wavelength_power);
    json pixkey = json::object(
        {{"x", x}, {"y", y}, {"values", j_map}});
    return pixkey;
  }
  json data() {
    int imrownb = (int)imgspec.size();
    int imcolnb = (int)imgspec[0].size();
    int i = 0;
    json obj = json::object();
    for (int x = 0; x < imrownb; x++) {
      for (int y = 0; y < imcolnb; y++) {
        json pixval = pixel_obj(x, y, imgspec[x][y]);
        obj.push_back({std::to_string(i), pixval});
        i++;
      }
    }
    return obj;
  }
};
}
