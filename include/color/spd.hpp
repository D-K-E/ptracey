#pragma once

#include <color/wave.hpp>
#include <common.hpp>
#include <utils.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

static const unsigned int SPD_STRIDE = 20;
static const int VISIBLE_LAMBDA_START = 360;
static const int VISIBLE_LAMBDA_END = 830;

path CSV_PARENT = "./media/data/spectrum";
const std::string WCOL_NAME = "wavelength";
const std::string PCOL_NAME = "power";
const std::string SEP = ",";

class spd {
public:
  WaveLength wave_start;
  WaveLength wave_end;
  std::map<WaveLength, Power> wavelength_power;

public: // static members
  static spd rho_r;
  static spd rho_g;
  static spd rho_b;
  static spd cie_xbar;
  static spd cie_ybar;
  static spd cie_zbar;
  static spd standard_d65;
  static Power K;

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
    auto power_spd = sampled_wave<Real>(pws);
    auto wlength = ss.wavelengths();
    ss = spd(power_spd, wlength);
    return ss;
  }

public:
  // ----------------- Start Constructors ---------------
  spd(uint size = 471 / SPD_STRIDE) {
    wavelength_power.clear();
    std::vector<Power> powers;
    for (uint i = 0; i < size; i++) {
      powers.push_back(random_real());
    }
    auto power_spd = sampled_wave<Real>(powers);
    auto ws = linspace<WaveLength>(VISIBLE_LAMBDA_START,
                                   VISIBLE_LAMBDA_END,
                                   powers.size());
    for (uint i = 0; i < (uint)powers.size(); i++) {
      auto pw = make_pair(ws[i], powers[i]);
      wavelength_power.insert(pw);
    }
  }
  spd(const sampled_wave<Power> &sampled_powers,
      WaveLength wstart, uint stride = SPD_STRIDE)
      : wave_start(wstart) {
    wavelength_power.clear();
    for (uint i = 0; i < sampled_powers.size(); i++) {
      Real power = sampled_powers[i];
      WaveLength wavelength =
          i + static_cast<WaveLength>(wstart);
      auto pw = make_pair(wavelength, power);
      wavelength_power.insert(pw);
    }
    wave_end = wave_start + static_cast<WaveLength>(
                                sampled_powers.size() - 1);
  }
  spd(const sampled_wave<Power> &sampled_powers,
      std::vector<WaveLength> wlengths,
      uint stride = SPD_STRIDE) {
    wavelength_power.clear();
    D_CHECK(sampled_powers.size() == wlengths.size());
    for (auto wl : wlengths) {
      if (wl < 0) {
        throw std::runtime_error(
            "wavelength can not be less than 0");
      }
    }
    std::map<WaveLength, Real> temp;
    for (uint i = 0; i < (uint)wlengths.size(); i++) {
      temp.insert(
          make_pair(wlengths[i], sampled_powers[i]));
    }
    std::sort(wlengths.begin(), wlengths.end(),
              [](auto i, auto j) { return i < j; });
    wave_start = wlengths[0];
    wave_end = wlengths[wlengths.size() - 1];
    for (uint i = 0; i < wlengths.size(); i++) {
      auto pw =
          make_pair(wlengths[i], temp.at(wlengths[i]));
      wavelength_power.insert(pw);
    }
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
    wavelength_power.clear();
    for (uint i = 1; i < wave_range; i++) {
      WaveLength wave = wavelength_generator(i);
      Real power_value = power_generator(wave);
      wavelength_power.insert(make_pair(wave, power_value));
    }
  }
  template <typename U>
  void fill_with_stride(std::vector<U> &dest,
                        const std::vector<U> &srcv,
                        unsigned int stride) {
    for (unsigned int i = 0; i < srcv.size(); i += stride) {
      dest.push_back(srcv[i]);
    }
  }
  spd(const path &csv_path,
      const std::string &wave_col_name = "wavelength",
      const std::string &power_col_name = "power",
      const std::string &sep = ",",
      const unsigned int stride = SPD_STRIDE) {
    wavelength_power.clear();
    path csv_path_abs = RUNTIME_PATH / csv_path;
    rapidcsv::Document doc(csv_path_abs.string(),
                           rapidcsv::LabelParams(0, -1),
                           rapidcsv::SeparatorParams(','));
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
    D_CHECK(tempv.size() == temp.size());
    std::vector<WaveLength> wlengths;
    fill_with_stride<WaveLength>(wlengths, tempv, stride);

    wavelength_power.clear();
    D_CHECK(powers.size() == wlengths.size());
    for (auto wl : wlengths) {
      if (wl < 0) {
        throw std::runtime_error(
            "wavelength can not be less than 0");
      }
    }
    std::map<WaveLength, Real> tempwp;
    for (uint i = 0; i < wlengths.size(); i++) {
      tempwp.insert(make_pair(wlengths[i], powers[i]));
    }
    std::sort(wlengths.begin(), wlengths.end(),
              [](auto i, auto j) { return i < j; });
    wave_start = wlengths[0];
    wave_end = wlengths[wlengths.size() - 1];
    for (uint i = 0; i < wlengths.size(); i++) {
      auto pw =
          make_pair(wlengths[i], tempwp.at(wlengths[i]));
      wavelength_power.insert(pw);
    }
  }
  // ------------------- End Constructors -----------------
public:
  // ------------------- Start Methods --------------------
  sampled_wave<Power> powers() const {
    std::vector<Power> ps;
    for (auto pw : wavelength_power) {
      ps.push_back(pw.second);
    }
    return sampled_wave<Power>(ps);
  }
  std::vector<WaveLength> wavelengths() const {
    std::vector<WaveLength> ps;
    for (auto pw : wavelength_power) {
      ps.push_back(pw.first);
    }
    std::sort(ps.begin(), ps.end(),
              [](auto i, auto j) { return i < j; });
    return ps;
  }
  spd normalized() const {
    sampled_wave<Power> normal_power = powers();
    auto normal_power2 = normal_power.interpolate(0.0, 1.0);
    auto wlengths = wavelengths();
    auto ss = spd(normal_power2, wlengths);
    return ss;
  }
  void normalize() {
    auto spd = normalized();
    *this = spd;
  }
  int min_wave() const {
    auto it = std::min_element(
        wavelength_power.begin(), wavelength_power.end(),
        [](const auto &k1, const auto &k2) {
          return k1.first < k2.first;
        });
    auto wavel =
        it == wavelength_power.end() ? -1.0 : it->first;
    return wavel;
  }
  int max_wave() const {
    auto it = std::max_element(
        wavelength_power.begin(), wavelength_power.end(),
        [](const auto &k1, const auto &k2) {
          return k1.first < k2.first;
        });
    auto wavel =
        it == wavelength_power.end() ? -1.0 : it->first;
    return wavel;
  }
  Power min_power() const {
    auto it = std::min_element(
        wavelength_power.begin(), wavelength_power.end(),
        [](const auto &k1, const auto &k2) {
          return k1.second < k2.second;
        });
    auto p =
        it == wavelength_power.end() ? -1.0 : it->second;
    return p;
  }
  Power max_power() const {
    auto it = std::max_element(
        wavelength_power.begin(), wavelength_power.end(),
        [](const auto &k1, const auto &k2) {
          return k1.second < k2.second;
        });
    auto p =
        it == wavelength_power.end() ? -1.0 : it->second;
    return p;
  }
  std::size_t size() const {
    return wavelength_power.size();
  }
  Real integrate(const spd &nspd) const {
    D_CHECK(size() == nspd.size());
    Real x1 = min_wave();
    Real x2 = max_wave();

    Real stepsize = (x2 - x1) / static_cast<Real>(size());
    Real val = 0.0;
    auto waves = wavelengths();
    for (unsigned int i = 0; i < (unsigned int)size();
         i++) {
      auto pw = wavelength_power.at(waves[i]);
      auto v = nspd[waves[i]];
      val += pw * v;
    }
    return val * stepsize;
  }
  void apply(Real pvalue,
             const std::function<Power(Power, Power)> &fn) {
    for (auto &pr : wavelength_power) {
      pr.second = fn(pr.second, pvalue);
    }
  }
  bool apply(WaveLength wave_length, Power pvalue,
             const std::function<Power(Power, Power)> &fn) {
    if (in(wave_length)) {
      Power power_value = wavelength_power.at(wave_length);
      wavelength_power[wave_length] =
          fn(power_value, pvalue);
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
    auto waves = wavelengths();
    auto swaves = s.wavelengths();
    auto res = *this;
    if (waves == swaves)
      return eqfn(res, s);
    for (const auto &w : swaves) {
      auto spower = s[w];
      if (!uneqfn(res, w, spower))
        res.update(w, spower);
    }
    return res;
  }
  Power operator[](WaveLength wave_length) const {
    return wavelength_power.at(wave_length);
  }
  bool in(WaveLength wave_length) {
    auto it = wavelength_power.find(wave_length);
    if (it != wavelength_power.end()) {
      return true;
    }
    return false;
  }
  void insert(WaveLength wave_length, Power pvalue) {
    if (!in(wave_length)) {
      wavelength_power.insert(
          make_pair(wave_length, pvalue));
    }
  }
  void update(WaveLength wave_length, Power pvalue) {
    wavelength_power.insert_or_assign(wave_length, pvalue);
  }
  void add(WaveLength wave_length, Power pvalue) {
    bool res =
        apply(wave_length, pvalue,
              [](Power i, Power j) { return i + j; });
    if (!res) {
      update(wave_length, pvalue);
    }
  }
  void scale(Real pvalue) {
    apply(pvalue, [](Real i, Real j) { return i * j; });
  }
};
Power get_cie_k() {
  Power sum = 0.0;
  auto waves = spd::standard_d65.wavelengths();
  for (const auto &wave : waves) {
    sum += spd::standard_d65[wave] * spd::cie_ybar[wave];
  }
  return 100.0 / sum;
}
Power get_cie_k(const spd &ss, const spd &cie_y,
                WaveLength wave_length_start = 360,
                WaveLength wave_length_end = 830,
                uint stepsize = 0) {
  Power sum = 0.0;
  if (stepsize > 0) {
    for (uint i = wave_length_start; i < wave_length_end;
         i += stepsize) {
      Power w1 = ss[static_cast<WaveLength>(i)];
      Power w2 = cie_y[static_cast<WaveLength>(i)];
      sum += (w1 * w2);
    }
    return 100 / sum;
  }
  if (ss.wavelength_power.size() !=
      cie_y.wavelength_power.size()) {
    throw std::runtime_error("if step size is 0, spd and "
                             "cie_y should have same size");
  }
  auto pw1 = ss.powers();
  auto pw2 = cie_y.powers();
  for (uint i = 0; i < ss.wavelength_power.size(); i++) {
    sum += pw1[i] * pw2[i];
  }
  return 100.0 / sum;
}

Power get_cie_val(const spd &qlambda, const spd &cie_bar) {
  Power sum = 0.0;
  auto waves = qlambda.wavelengths();
  for (const auto &wave : waves) {
    sum += qlambda[wave] * cie_bar[wave];
  }
  return spd::K * sum;
}
Power get_cie_val(const spd &qlambda, const spd &rlambda,
                  const spd &cie_bar) {
  Power sum = 0.0;
  auto waves = qlambda.wavelengths();
  for (const auto &wave : waves) {
    sum += qlambda[wave] * rlambda[wave] * cie_bar[wave];
  }
  return spd::K * sum;
}
Power get_cie_x(const spd &qlambda) {
  return get_cie_val(qlambda, spd::cie_xbar);
}
Power get_cie_x(const spd &qlambda, const spd &rlambda) {
  return get_cie_val(qlambda, rlambda, spd::cie_xbar);
}
Power get_cie_y(const spd &qlambda) {
  return get_cie_val(qlambda, spd::cie_ybar);
}
Power get_cie_y(const spd &qlambda, const spd &rlambda) {
  return get_cie_val(qlambda, rlambda, spd::cie_ybar);
}
Power get_cie_z(const spd &qlambda) {
  return get_cie_val(qlambda, spd::cie_zbar);
}
Power get_cie_z(const spd &qlambda, const spd &rlambda) {
  return get_cie_val(qlambda, rlambda, spd::cie_zbar);
}
void get_cie_values(const spd &qlambda, Power &X, Power &Y,
                    Power &Z) {
  X = get_cie_x(qlambda);
  Y = get_cie_y(qlambda);
  Z = get_cie_z(qlambda);
}
void get_cie_values(const spd &qlambda, const spd &rlambda,
                    Power &X, Power &Y, Power &Z) {
  X = get_cie_x(qlambda, rlambda);
  Y = get_cie_y(qlambda, rlambda);
  Z = get_cie_z(qlambda, rlambda);
}
void get_cie_values(const spd &qlambda, vec3 &xyz) {
  Power X, Y, Z;
  get_cie_values(qlambda, X, Y, Z);
  xyz = vec3(X, Y, Z);
  Real sum = xyz.sum();
  if (sum != 0) {
    xyz = xyz / sum;
  }
}
void get_cie_values(const spd &qlambda, const spd &rlambda,
                    vec3 &xyz) {
  Power X, Y, Z;
  get_cie_values(qlambda, rlambda, X, Y, Z);
  xyz = vec3(X, Y, Z);
  Real sum = xyz.sum();
  if (sum != 0) {
    xyz = xyz / sum;
  }
}

auto rho_rspd = spd(CSV_PARENT / "rho-r-2012.csv",
                    WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
spd spd::rho_r = rho_rspd.normalized();

auto rho_gspd = spd(CSV_PARENT / "rho-g-2012.csv",
                    WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
spd spd::rho_g = rho_gspd.normalized();

auto rho_bspd = spd(CSV_PARENT / "rho-b-2012.csv",
                    WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
spd spd::rho_b = rho_bspd.normalized();

auto cie_xbarspd =
    spd(CSV_PARENT / "cie-x-bar-1964.csv", WCOL_NAME,
        PCOL_NAME, SEP, SPD_STRIDE);

spd spd::cie_xbar = cie_xbarspd.normalized();

auto cie_ybarspd =
    spd(CSV_PARENT / "cie-y-bar-1964.csv", WCOL_NAME,
        PCOL_NAME, SEP, SPD_STRIDE);

spd spd::cie_ybar = cie_ybarspd.normalized();

auto cie_zbarspd =
    spd(CSV_PARENT / "cie-z-bar-1964.csv", WCOL_NAME,
        PCOL_NAME, SEP, SPD_STRIDE);
spd spd::cie_zbar = cie_zbarspd.normalized();

auto stand_d65 = spd(CSV_PARENT / "cie-d65-standard.csv",
                     WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
spd spd::standard_d65 = stand_d65.normalized();
//
Power spd::K = get_cie_k();
}
