#pragma once

#include <algorithm>
#include <color/specdata.hpp>
#include <color/specutils.hpp>
#include <color/wave.hpp>
#include <common.hpp>
#include <math3d/vec3.hpp>
#include <utils.hpp>

using namespace ptracey;
namespace ptracey {

class spd {
public:
  /** start of the wave length range*/
  WaveLength wave_start;
  /** end of the wave length range*/
  WaveLength wave_end;
  std::map<WaveLength, Power> wavelength_power;
  std::vector<WaveLength> wave_lengths;
  sampled_wave<Power> power_values;

public:
  // static methods
  static spd random() { return spd(); }
  static spd random(Real mn, Real mx) {
    auto ss = spd();
    auto pw = ss.powers();
    std::vector<Real> pws;
    for (uint i = 0; i < (uint)pw.size(); i++) {
      pws.push_back(random_real(mn, mx));
    }
    auto power_spd = sampled_wave<Power>(pws);
    auto wlength = ss.wavelengths();
    ss = spd(power_spd, wlength);
    return ss;
  }

  static spd zeros_like(const spd &s) {
    auto waves = s.wavelengths();
    std::vector<Power> pws(waves.size(), 0.0);
    sampled_wave<Power> powers(pws);
    auto sp = spd(powers, waves);
    return sp;
  }

public:
  // ----------------- Start Constructors ---------------
  spd(uint size = 471 / SPD_STRIDE) {
    std::vector<Power> powers;
    for (uint i = 0; i < size; i++) {
      powers.push_back(0.0);
    }
    auto power_spd = sampled_wave<Power>(powers);
    auto ws = linspace<WaveLength>(VISIBLE_LAMBDA_START,
                                   VISIBLE_LAMBDA_END,
                                   powers.size());
    power_values = power_spd;
    wave_lengths = ws;
    wave_start = min_wave();
    wave_end = max_wave();
  }
  spd(const sampled_wave<Power> &sampled_powers,
      WaveLength wstart)
      : wave_start(wstart), power_values(sampled_powers) {
    for (uint i = 0; i < sampled_powers.size(); i++) {
      WaveLength wavelength =
          i + static_cast<WaveLength>(wstart);
      wave_lengths.push_back(wavelength);
    }
    wave_end = wave_start + static_cast<WaveLength>(
                                sampled_powers.size() - 1);
  }
  spd(const sampled_wave<Power> &sampled_powers,
      const std::vector<WaveLength> &wlengths) {
    COMP_CHECK(
        sampled_powers.size() == (uint)wlengths.size(),
        sampled_powers.size(), (uint)wlengths.size());
    for (auto wl : wlengths) {
      if (wl < 0) {
        throw std::runtime_error(
            "wavelength can not be less than 0");
      }
    }
    power_values = sampled_powers;
    wave_lengths = wlengths;
    wave_start = *std::min_element(wave_lengths.begin(),
                                   wave_lengths.end());
    wave_end = *std::max_element(wave_lengths.begin(),
                                 wave_lengths.end());
  }
  /**
    Slightly more correct way of making a spds.
    If the user knows exact functions that generate
    corresponding
    power for a given wavelength. That function should be
    passed in
    the body of power generator.
    @power_generator: takes a wavelength as input and
    generates the
    corresponding power.
    @wavelength_generator: maps a discreet range of numbers
    [0,1,2,...,n] to
    wavelengths.
    @wave_range: gives the size of the discreet range of
    numbers. If the
    desired range of numbers is [0,1,2,...,n] then n+1
    should be given.
   */
  spd(uint wave_range, const std::function<WaveLength(uint)>
                           &wavelength_generator,
      const std::function<Power(WaveLength)>
          &power_generator) {
    wave_start = wavelength_generator(0);
    wave_end = wavelength_generator(wave_range - 1);
    wave_lengths.clear();
    std::vector<Power> ps;
    for (uint i = 1; i < wave_range; i++) {
      WaveLength wave = wavelength_generator(i);
      wave_lengths.push_back(wave);
      Power power_value = power_generator(wave);
      ps.push_back(power_value);
    }
    power_values = sampled_wave<Power>(ps);
  }
  template <typename V>
  void fill_with_stride(std::vector<V> &dest,
                        const std::vector<V> &srcv,
                        unsigned int stride) {
    for (unsigned int i = 0; i < (uint)srcv.size();
         i += stride) {
      dest.push_back(srcv[i]);
    }
  }
  spd(const path &csv_path,
      const std::string &wave_col_name = "wavelength",
      const std::string &power_col_name = "power",
      const char &sep = ',',
      const unsigned int stride = SPD_STRIDE,
      const std::function<WaveLength(WaveLength)>
          &wave_transform = [](auto j) { return j; },
      const std::function<Power(Power)> &power_transform =
          [](auto j) { return j; }) {
    wavelength_power.clear();
    path csv_path_abs = RUNTIME_PATH / csv_path;
    rapidcsv::Document doc(csv_path_abs.string(),
                           rapidcsv::LabelParams(0, -1),
                           rapidcsv::SeparatorParams(sep));
    std::vector<Power> temp;
    try {
      temp = doc.GetColumn<Power>(power_col_name);
    } catch (const std::out_of_range &err) {
      throw std::runtime_error(std::string(err.what()) +
                               " :: " + power_col_name +
                               " in file :: " +
                               csv_path_abs.string());
    }
    std::vector<Power> powers;
    fill_with_stride<Power>(powers, temp, stride);
    std::vector<WaveLength> tempv;
    try {
      tempv = doc.GetColumn<WaveLength>(wave_col_name);
    } catch (const std::out_of_range &error) {
      throw std::runtime_error(std::string(error.what()) +
                               " :: " + wave_col_name +
                               " in file :: " +
                               csv_path_abs.string());
    }
    COMP_CHECK(tempv.size() == temp.size(), tempv.size(),
               temp.size());
    std::vector<WaveLength> wlengths;
    fill_with_stride<WaveLength>(wlengths, tempv, stride);

    COMP_CHECK(powers.size() == wlengths.size(),
               powers.size(), wlengths.size());
    for (auto wl : wlengths) {
      if (wl < 0) {
        throw std::runtime_error(
            "wavelength can not be less than 0");
      }
    }
    wave_start =
        *std::min_element(wlengths.begin(), wlengths.end());
    wave_end =
        *std::max_element(wlengths.begin(), wlengths.end());
    wave_lengths = wlengths;
    power_values = powers;
  }
  // ------------------- End Constructors -----------------
public:
  // ------------------- Start Methods --------------------

  void resample(const spd &s);
  void resample(const WaveLength &waveLStart,
                const WaveLength &waveLEnd,
                const uint &outSize);
  spd resample_c(const spd &s) const;
  spd resample_c(const uint &outSize) const;
  spd resample_c(const WaveLength &waveLStart,
                 const WaveLength &waveLEnd,
                 const uint &outSize) const;

  sampled_wave<Power> powers() const {
    return power_values;
  }
  std::vector<WaveLength> wavelengths() const {
    std::vector<WaveLength> wlengths;
    wlengths.resize(wave_lengths.size());
    std::copy(wave_lengths.begin(), wave_lengths.end(),
              wlengths.data());
    if (wave_start > wlengths[0]) {
      std::sort(wlengths.begin(), wlengths.end(),
                [](auto i, auto j) { return i < j; });
    }
    return wlengths;
  }
  int power_index(WaveLength w) const {
    auto iter = std::find(wave_lengths.begin(),
                          wave_lengths.end(), w);
    int index = -1;
    if (iter != wave_lengths.end()) {
      index = std::distance(wave_lengths.begin(), iter);
    }
    return index;
  }
  spd interpolate(Real low = 0.0,
                  Real high = FLT_MAX) const {
    sampled_wave<Power> normal_power = powers();
    auto normal_power2 =
        normal_power.interpolate(low, high);
    auto wlengths = wavelengths();
    auto ss = spd(normal_power2, wlengths);
    return ss;
  }
  spd clamp(Power low = 0.0, Power high = FLT_MAX) const {
    sampled_wave<Power> normal_power = powers();
    std::vector<Power> nvs;
    for (auto np : normal_power.values) {
      nvs.push_back(dclamp<Power>(np, low, high));
    }
    auto normal_power2 = sampled_wave<Power>(nvs);
    auto wlengths = wavelengths();
    auto ss = spd(normal_power2, wlengths);
    return ss;
  }
  spd normalized() const { return interpolate(0.0, 1.0); }
  void normalize() {
    auto spd = normalized();
    *this = spd;
  }
  WaveLength min_wave() const {
    return *std::min_element(wave_lengths.begin(),
                             wave_lengths.end());
  }
  WaveLength max_wave() const {
    return *std::max_element(wave_lengths.begin(),
                             wave_lengths.end());
  }
  Power min_power() const { return power_values.min(); }
  Power max_power() const { return power_values.max(); }
  uint size() const { return (uint)wave_lengths.size(); }
  Power integrate(const spd &nspd) const {
    COMP_CHECK(size() == nspd.size(), size(), nspd.size());
    WaveLength x1 = min_wave();
    WaveLength x2 = max_wave();

    Power stepsize = (x2 - x1) / static_cast<Power>(size());
    Power val = 0.0;
    auto waves = wavelengths();
    for (std::size_t i = 0; i < size(); i++) {
      auto pw = power_values[i];
      auto v = nspd[waves[i]];
      val += pw * v;
    }
    return val * stepsize;
  }
  void apply(Power pvalue,
             const std::function<Power(Power, Power)> &fn) {
    for (uint i = 0; power_values.size(); i++) {
      power_values[i] = fn(power_values[i], pvalue);
    }
  }
  spd apply_c(
      Power pvalue,
      const std::function<Power(Power, Power)> &fn) const {
    std::vector<WaveLength> waves;
    std::vector<Power> pwers;
    for (uint i = 0; i < size(); i++) {
      pwers.push_back(fn(power_values[i], pvalue));
      waves.push_back(wave_lengths[i]);
    }
    auto sw = sampled_wave<Power>(pwers);
    auto sp = spd(sw, waves);
    return sp;
  }
  bool apply(WaveLength wave_length, Power pvalue,
             const std::function<Power(Power, Power)> &fn) {
    if (in(wave_length)) {
      int index = power_index(wave_length);
      Power power_value = power_values[index];
      power_values[index] = fn(power_value, pvalue);
      return true;
    }
    return false;
  }
  bool apply(const spd &s,
             const std::function<spd(spd, spd)> &fn,
             spd &ss) const {
    auto waves = wavelengths();
    auto swaves = s.wavelengths();
    auto res = *this;
    if (waves == swaves) {
      ss = fn(res, s);
      return true;
    }
    return false;
  }
  /**
    Slightly more robust implementation of apply.

    The idea is to give two functions: 1 for matching spds,
    two for not
    matching spds.
    If they match we apply the first function if they don't
    match.
    We apply the second function for each wavelength and
    power value.
    If the wavelength is not present in the spd we insert it
    with the
    given power value.
   */
  spd rapply(const spd &s,
             const std::function<spd(spd, spd)> &eqfn,
             const std::function<bool(
                 spd, WaveLength, Power)> &uneqfn) const {
    spd ss;
    if (apply(s, eqfn, ss)) {
      return ss;
    }
    auto swaves = s.wavelengths();
    for (const auto &w : swaves) {
      auto spower = s[w];
      if (!uneqfn(ss, w, spower))
        ss.update(w, spower);
    }
    return ss;
  }
  Power interpolate_wave_power(WaveLength wl) const {
    //
    auto waves = wavelengths();
    auto wsize = (int)waves.size();
    auto lower =
        std::lower_bound(waves.begin(), waves.end(), wl);
    if (lower != waves.end()) {
      auto index = std::distance(waves.begin(), lower);
      if (index == 0) {
        throw std::runtime_error(
            "wavelength is below the available range");
      }
      auto index2 = index - 1;
      Power p1 = power_values[index];
      Power p2 = power_values[index2];
      return (p1 + p2) / 2.0;
    }

    throw std::runtime_error(
        "wavelength is above the available range");
  }
  Power operator[](WaveLength wave_length) const {
    int index = power_index(wave_length);
    if (index < 0) {
      return interpolate_wave_power(wave_length);
    }
    return power_values[index];
  }
  bool in(WaveLength wave_length) const {
    auto it = std::find(wave_lengths.begin(),
                        wave_lengths.end(), wave_length);
    if (it != wave_lengths.end()) {
      return true;
    }
    return false;
  }
  std::map<WaveLength, Power> spd_map() const {
    std::map<WaveLength, Power> wpower;
    for (std::size_t i = 0; i < size(); i++) {
      wpower.insert(
          make_pair(wave_lengths[i], power_values[i]));
    }
    return wpower;
  }
  void
  set_from_map(const std::map<WaveLength, Power> &wpws) {
    //
    wave_lengths.clear();
    std::vector<Power> pws;
    for (const auto &wp : wpws) {
      wave_lengths.push_back(wp.first);
      pws.push_back(wp.second);
    }
    wave_start = min_wave();
    wave_end = max_wave();
    power_values = sampled_wave<Power>(pws);
  }
  void insert(WaveLength wave_length, Power pvalue) {
    if (!in(wave_length)) {
      auto wpowers = spd_map();
      wpowers.insert(make_pair(wave_length, pvalue));
      set_from_map(wpowers);
    }
  }
  void update(WaveLength wave_length, Power pvalue) {
    std::map<WaveLength, Power> wpowers = spd_map();
    wpowers.insert_or_assign(wave_length, pvalue);
    set_from_map(wpowers);
  }
  void add(WaveLength wave_length, Power pvalue) {
    bool res =
        apply(wave_length, pvalue,
              [](Power i, Power j) { return i + j; });
    if (!res) {
      update(wave_length, pvalue);
    }
  }
  void scale(Power pvalue) {
    apply(pvalue, [](Power i, Power j) { return i * j; });
  }
  spd operator*(Power pvalue) const {
    return apply_c(pvalue, [](auto spdval, auto second) {
      return spdval * second;
    });
  }
  spd &operator*=(Power pvalue) {
    apply(pvalue, [](Power spdv, Power second) {
      return spdv * second;
    });
    return *this;
  }
  friend spd operator*(Power pvalue, const spd &s) {
    return s * pvalue;
  }
  spd operator-(Power pvalue) const {
    return apply_c(pvalue, [](auto spdval, auto j) {
      return spdval - j;
    });
  }
  friend spd operator-(Power pvalue, const spd &s) {
    return s.apply_c(pvalue, [](auto spdval, auto j) {
      return j - spdval;
    });
  }

  spd operator+(Power pvalue) const {
    return apply_c(pvalue, [](auto spdval, auto j) {
      return spdval + j;
    });
  }
  friend spd operator+(Power pvalue, const spd &s) {
    return s + pvalue;
  }
  spd &operator+=(Power pvalue) {
    apply(pvalue,
          [](Power spdval, Power j) { return spdval + j; });
    return *this;
  }
  spd &operator+=(const spd &s) {
    auto resp = apply(s,
                      [](spd i, spd j) {
                        auto pwrs = i.powers() + j.powers();
                        return spd(pwrs, i.wavelengths());
                      },
                      *this);
    if (resp) {
      return *this;
    }
    throw std::runtime_error("spd's don't match");
  }
  //
};

auto rho_rspd = spd(CSV_PARENT / "rho-r-2012.csv",
                    WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);

static spd rho_r = rho_rspd.normalized();

auto rho_gspd = spd(CSV_PARENT / "rho-g-2012.csv",
                    WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);

static spd rho_g = rho_gspd;

auto rho_bspd = spd(CSV_PARENT / "rho-b-2012.csv",
                    WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);

static spd rho_b = rho_bspd;

auto cie_xbarspd =
    spd(CSV_PARENT / "cie-x-bar-1964.csv", WCOL_NAME,
        PCOL_NAME, SEP, SPD_STRIDE);

static spd cie_xbar_spd = cie_xbarspd.normalized();

auto cie_ybarspd =
    spd(CSV_PARENT / "cie-y-bar-1964.csv", WCOL_NAME,
        PCOL_NAME, SEP, SPD_STRIDE);

static spd cie_ybar_spd = cie_ybarspd.normalized();

auto cie_zbarspd =
    spd(CSV_PARENT / "cie-z-bar-1964.csv", WCOL_NAME,
        PCOL_NAME, SEP, SPD_STRIDE);

static spd cie_zbar_spd = cie_zbarspd.normalized();

auto stand_d65 = spd(CSV_PARENT / "cie-d65-standard.csv",
                     WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);

static spd standard_d65 = stand_d65.normalized();
//

Power get_cie_val(const spd &qlambda, const spd &cie_bar) {
  Power sum = 0.0;
  auto qwaves = qlambda.wavelengths();
  auto qmaxwave = qlambda.max_wave();
  auto qminwave = qlambda.min_wave();
  auto qwsize = (uint)qwaves.size();
  auto cie = cie_bar.resample_c(qminwave, qmaxwave, qwsize);
  for (const auto &wave : qwaves) {
    sum += qlambda[wave] * cie[wave];
  }
  auto scale = (qmaxwave - qminwave) / Real(qwsize);
  return scale * sum;
}

Power get_cie_val(const spd &qlambda, const spd &rlambda,
                  const spd &cie_bar) {
  Power sum = 0.0;
  auto rmaxwave = rlambda.max_wave();
  auto rminwave = rlambda.min_wave();
  auto rwaves = rlambda.wavelengths();
  auto rwsize = (uint)rwaves.size();
  spd q_lambda =
      qlambda.resample_c(rminwave, rmaxwave, rwsize);
  spd cie = cie_bar.resample_c(rminwave, rmaxwave, rwsize);
  auto waves = qlambda.wavelengths();
  for (const auto &wave : rwaves) {
    sum += q_lambda[wave] * rlambda[wave] * cie[wave];
  }
  auto scale =
      (rmaxwave - rminwave) / Real(CIE_Y_integral * rwsize);

  return scale * sum;
}

Power get_cie_x(const spd &qlambda) {
  return get_cie_val(qlambda, cie_xbar_spd);
}

Power get_cie_x(const spd &qlambda, const spd &rlambda) {
  return get_cie_val(qlambda, rlambda, cie_xbar_spd);
}

Power get_cie_y(const spd &qlambda) {
  return get_cie_val(qlambda, cie_ybar_spd);
}

Power get_cie_y(const spd &qlambda, const spd &rlambda) {
  return get_cie_val(qlambda, rlambda, cie_ybar_spd);
}

Power get_cie_z(const spd &qlambda) {
  return get_cie_val(qlambda, cie_zbar_spd);
}

Power get_cie_z(const spd &qlambda, const spd &rlambda) {
  return get_cie_val(qlambda, rlambda, cie_zbar_spd);
}

void get_cie_values(const spd &qlambda, Power &x, Power &y,
                    Power &z) {
  x = get_cie_x(qlambda);
  y = get_cie_y(qlambda);
  z = get_cie_z(qlambda);
}

void get_cie_values(const spd &qlambda, const spd &rlambda,
                    Power &x, Power &y, Power &z) {
  x = get_cie_x(qlambda, rlambda);
  y = get_cie_y(qlambda, rlambda);
  z = get_cie_z(qlambda, rlambda);
}

void get_cie_values(const spd &qlambda, vec3 &xyz) {
  Power x, y, z;
  get_cie_values(qlambda, x, y, z);
  xyz = vec3(x, y, z);
  Power sum = xyz.sum();
  if (sum != 0) {
    xyz = xyz / sum;
  }
}

void get_cie_values(const spd &qlambda, const spd &rlambda,
                    vec3 &xyz) {
  Power x, y, z;
  get_cie_values(qlambda, rlambda, x, y, z);
  xyz = vec3(x, y, z);
  Power sum = xyz.sum();
  if (sum != 0) {
    xyz = xyz / sum;
  }
}

inline std::ostream &operator<<(std::ostream &out,
                                const spd &ss) {
  auto waves = ss.wavelengths();
  auto powers = ss.powers();
  return out << "spectrum power distribution: " << std::endl
             << "waves " << waves << std::endl
             << "powers: " << powers << std::endl;
}
}
