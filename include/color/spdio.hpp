#pragma once

#include <color/sampled_spectrum.hpp>
#include <color/spd.hpp>
#include <color/spectrum.hpp>
#include <color/specutils.hpp>
#include <common.hpp>

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
  static imspec from_file(const char *path) {
    uint32_t width, height, nb_channels;
    float first_wavelength, wavelength_resolution;
    float last_wavelength;
    spb::read_header(
        path, width, height, nb_channels, first_wavelength,
        wavelength_resolution, last_wavelength);
    float *data = new float[width * height * nb_channels];
    spb::read_file(path, width, height, nb_channels, data);
    float wave_length = first_wavelength;
    std::vector<WaveLength> wavelengths;
    while (wave_length < last_wavelength) {
      wavelengths.push_back((WaveLength)wave_length);
      wave_length += wavelength_resolution;
    }
    //
    imspec spec(width,
                std::vector<sampled_spectrum>(height));
    int imsize = width * height;
    for (int y = 0; y < height; y++) {
      for (int x = 0; x < width; x++) {
        int xyval = y + x;
        std::vector<Power> pwrs(nb_channels);
        for (int j = 0; j < nb_channels; j++) {
          pwrs[j] = (Power)data[xyval + j];
        }
        sampled_wave<Power> spower(pwrs);
        spd sspd(spower, wavelengths);
        sampled_spectrum sspec(sspd,
                               SpectrumType::Reflectance);
        spec[x][y] = sspec;
      }
    }
    delete[] data;
    return spec;
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

struct CsvData {
  std::pair<uint, uint> xy;
  std::vector<std::pair<Real, Real>> wavelength_power;
  std::size_t data_size;
};

class SpdCsvIo {
public:
  imspec imgspec;
  Real antialiasing_coeff;
  std::vector<CsvData> csvds;
  std::size_t max_row_size = 0;

public:
  SpdCsvIo(const imspec &imgs, Real coeff)
      : imgspec(imgs), antialiasing_coeff(coeff) {
    csvds = get_csv_data();
  }

  std::vector<CsvData> get_csv_data() {
    std::vector<CsvData> datas;
    for (int y = 0; y < imgspec.size(); y++) {
      for (int x = 0; x < imgspec[0].size(); x++) {
        auto sample = imgspec[y][x];
        auto waves = sample.spect.wavelengths();
        auto powers = sample.spect.powers();
        std::vector<std::pair<Real, Real>> pws;
        for (std::size_t i = 0; i < waves.size(); i++) {
          pws.push_back(std::pair<Real, Real>(
              (Real)waves[(uint)i],
              powers[(uint)i] * antialiasing_coeff));
        }
        CsvData csvd;
        csvd.wavelength_power = pws;
        csvd.xy = std::pair<uint, uint>((uint)x, (uint)y);
        csvd.data_size = pws.size();
        if (csvd.data_size > max_row_size) {
          max_row_size = csvd.data_size;
        }
        datas.push_back(csvd);
      }
    }
    return datas;
  }
  std::vector<std::pair<Real, Real>>
  get_power_row(std::size_t nb_row) {
    std::vector<std::pair<Real, Real>> row;
    for (const auto &csvd : csvds) {
      const auto wpowers = csvd.wavelength_power;
      if (wpowers.size() <= nb_row) {
        row.push_back(std::pair<Real, Real>(0.0, 0.0));
      } else {
        row.push_back(std::pair<Real, Real>(
            wpowers[nb_row].first, wpowers[nb_row].second));
      }
    }
    return row;
  }
  std::vector<std::pair<uint, uint>> get_first_row() {
    std::vector<std::pair<uint, uint>> first_row;
    for (const auto &csvd : csvds) {
      first_row.push_back(csvd.xy);
    }
    return first_row;
  }
  std::vector<std::vector<std::pair<Real, Real>>> data() {
    std::vector<std::vector<std::pair<Real, Real>>> rowds;
    for (std::size_t i = 0; i < max_row_size; i++) {
      rowds.push_back(get_power_row(i));
    }
    return rowds;
  }
};
}
