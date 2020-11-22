#pragma once

#include <color/color.hpp>
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

class spd : public colorable {
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

public:
  // static methods
  static spd random() { return spd(); }
  static spd random(Real mn, Real mx) {
    auto ss = spd();
    auto pw = ss.power();
    std::vector<Real> pws;
    for (int i = 0; i < pw.size(); i++) {
      pws.push_back(random_real(mn, mx));
    }
    auto power_spd = sampled_wave<Real>(pws);
    auto wlength = ss.wavelengths();
    ss = spd(power_spd, wlength);
    return ss;
  }

public:
  // ----------------- Start Constructors ---------------
  spd(unsigned int size = 471 / SPD_STRIDE) {
    wavelength_power.clear();
    std::vector<Power> powers;
    for (unsigned int i = 0; i < size; i++) {
      powers.push_back(random_real());
    }
    auto power_spd = sampled_wave<Real>(powers);
    auto ws = linspace<WaveLength>(VISIBLE_LAMBDA_START,
                                   VISIBLE_LAMBDA_END,
                                   powers.size());
    for (int i = 0; i < powers.size(); i++) {
      auto pw = make_pair(ws[i], powers[i]);
      wavelength_power.insert(pw);
    }
  }
  spd(const sampled_wave<Power> &sampled_powers,
      WaveLength wstart, unsigned int stride = SPD_STRIDE)
      : wave_start(wstart) {
    wavelength_power.clear();
    for (unsigned int i = 0; i < sampled_powers.size();
         i++) {
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
      unsigned int stride = SPD_STRIDE) {
    wavelength_power.clear();
    D_CHECK(sampled_powers.size() == wlengths.size());
    for (auto wl : wlengths) {
      if (wl < 0) {
        throw std::runtime_error(
            "wavelength can not be less than 0");
      }
    }
    std::map<WaveLength, Real> temp;
    for (int i = 0; i < wlengths.size(); i++) {
      temp.insert(
          make_pair(wlengths[i], sampled_powers[i]));
    }
    std::sort(wlengths.begin(), wlengths.end(),
              [](auto i, auto j) { return i < j; });
    wave_start = wlengths[0];
    wave_end = wlengths[wlengths.size() - 1];
    for (int i = 0; i < wlengths.size(); i++) {
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
  spd(unsigned int wave_range,
      const std::function<WaveLength(unsigned int)>
          &wavelength_generator,
      const std::function<Power(WaveLength)>
          &power_generator) {
    wave_start = wavelength_generator(0);
    wave_end = wavelength_generator(wave_range - 1);
    wavelength_power.clear();
    for (unsigned int i = 1; i < wave_range; i++) {
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
    for (int i = 0; i < wlengths.size(); i++) {
      tempwp.insert(make_pair(wlengths[i], powers[i]));
    }
    std::sort(wlengths.begin(), wlengths.end(),
              [](auto i, auto j) { return i < j; });
    wave_start = wlengths[0];
    wave_end = wlengths[wlengths.size() - 1];
    for (int i = 0; i < wlengths.size(); i++) {
      auto pw =
          make_pair(wlengths[i], tempwp.at(wlengths[i]));
      wavelength_power.insert(pw);
    }
  }
  // ------------------- End Constructors -----------------
public:
  // ------------------- Start Methods --------------------
  sampled_wave<Power> power() const {
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
  spd<Power> normalized() const {
    sampled_wave<Power> normal_power = power();
    auto normal_power2 = normal_power.interpolate(0.0, 1.0);
    auto wlengths = wavelengths();
    auto ss = spd<Power>(normal_power2, wlengths);
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
  Real integrate(const spd<Power> &nspd) const {
    D_CHECK(size() == nspd.size());
    Real x1 = min_wave();
    Real x2 = max_wave();

    Real stepsize = (x2 - x1) / static_cast<Real>(size());
    Real val = 0.0;
    auto waves = wavelengths();
    for (unsigned int i = 0; i < (unsigned int)size();
         i++) {
      auto pw = eval_wavelength(waves[i]);
      auto v = nspd.eval_wavelength(waves[i]);
      val += pw * v;
    }
    return val * stepsize;
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
  vec3 to_xyz() override {
    //
  }
  template <> Power evaluate(const WaveLength &w) const {
    //
  }
};
Real get_cie_k(const spd<Real> &ss, const spd<Real> &cie_y,
               int wave_length_start, int wave_length_end,
               int stepsize = 0) {
  Real sum = 0.0;
  if (stepsize > 0) {
    for (int i = wave_length_start; i < wave_length_end;
         i += stepsize) {
      Real w1 = ss[static_cast<WaveLength>(i)];
      Real w2 = cie_y[static_cast<WaveLength>(i)];
      sum += (w1 * w2);
    }
    return 100 / sum;
  }
  if (ss.wavelength_power.size() !=
      cie_y.wavelength_power.size()) {
    throw std::runtime_error("if step size is 0, spd and "
                             "cie_y should have same size");
  }
  auto pw1 = ss.power();
  auto pw2 = cie_y.power();
  for (int i = 0; i < ss.wavelength_power.size(); i++) {
    sum += pw1[i] * pw2[i];
  }
  return 100.0 / sum;
}
Real get_cie_val(const spd<Real> &qlambda,
                 const spd<Real> &cie_bar) {
  Real sum = qlambda.integrate(cie_bar);
  return sum;
}
Real get_cie_val(const spd<Real> &qlambda,
                 const spd<Real> &rlambda,
                 const spd<Real> &cie_bar) {
  D_CHECK(qlambda.size() == rlambda.size());
  D_CHECK(rlambda.size() == cie_bar.size());
  WaveLength w1 = qlambda.min_wave();
  WaveLength w2 = qlambda.max_wave();

  Real stepsize =
      (w2 - w1) / static_cast<Real>(qlambda.size());
  Real sum = 0.0;
  auto waves = qlambda.wavelengths();
  for (auto w : waves) {
    auto pw1 = qlambda[w];
    auto pw2 = rlambda[w];
    auto pw3 = cie_bar[w];
    sum += (pw1 * pw2 * pw3);
  }
  return sum * stepsize;
}
Real get_cie_val(const spd<Real> &rs, const spd<Real> &ss,
                 const spd<Real> &cie_spd,
                 WaveLength wl_start, WaveLength wl_end,
                 unsigned int stepsize = 0) {
  Real sum = 0.0;
  if (stepsize > 0) {
    for (unsigned int i = wl_start; i < wl_end;
         i += stepsize) {
      Real w1 = ss[i];
      Real w2 = cie_spd[i];
      Real w3 = rs[i];
      sum += (w1 * w2 * w3);
    }
    return sum;
  }
  if (ss.wavelength_power.size() !=
          cie_spd.wavelength_power.size() ||
      cie_spd.wavelength_power.size() !=
          rs.wavelength_power.size()) {
    throw std::runtime_error("if step size is 0, spd and "
                             "cie_y should have same size");
  }
  auto pw1 = ss.power();
  auto pw2 = cie_spd.power();
  auto pw3 = rs.power();
  for (unsigned int i = 0; i < ss.wavelength_power.size();
       i++) {
    sum += pw1[i] * pw2[i] * pw3[i];
  }
  return sum;
}
Real get_cie_value(const spd<Real> &reflectance,
                   const spd<Real> &illuminant,
                   const spd<Real> &cie_val,
                   const spd<Real> &cie_y,
                   WaveLength wl_start, WaveLength wl_end,
                   unsigned int stepsize = 0) {
  Real k = get_cie_k(illuminant, cie_y, wl_start, wl_end,
                     stepsize);
  Real val = get_cie_val(reflectance, illuminant, cie_val,
                         wl_start, wl_end, stepsize);
  return k * val;
}

auto rho_rspd = spd(CSV_PARENT / "rho-r-2012.csv",
                    WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
static spd spd::rho_r = rho_rspd.normalized();

auto rho_gspd = spd(CSV_PARENT / "rho-g-2012.csv",
                    WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
static spd spd::rho_g = rho_gspd.normalized();

auto rho_bspd = spd(CSV_PARENT / "rho-b-2012.csv",
                    WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
static spd spd::rho_b = rho_bspd.normalized();

auto cie_xbarspd =
    spd(CSV_PARENT / "cie-x-bar-1964.csv", WCOL_NAME,
        PCOL_NAME, SEP, SPD_STRIDE);

static spd spd::cie_xbar = cie_xbarspd.normalized();

auto cie_ybarspd =
    spd(CSV_PARENT / "cie-y-bar-1964.csv", WCOL_NAME,
        PCOL_NAME, SEP, SPD_STRIDE);

static spd spd::cie_ybar = cie_ybarspd.normalized();

auto cie_zbarspd =
    spd(CSV_PARENT / "cie-z-bar-1964.csv", WCOL_NAME,
        PCOL_NAME, SEP, SPD_STRIDE);
static spd spd::cie_zbar = cie_zbarspd.normalized();

auto stand_d65 = spd(CSV_PARENT / "cie-d65-standard.csv",
                     WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
static spd spd::standard_d65 = stand_d65.normalized();
//
}
