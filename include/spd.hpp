#pragma once

#include <common.hpp>
#include <utils.hpp>
#include <vec3.hpp>
#include <wave.hpp>

using namespace ptracey;
namespace ptracey {

static const unsigned int SPD_STRIDE = 20;
static const int VISIBLE_LAMBDA_START = 360;
static const int VISIBLE_LAMBDA_END = 830;

path CSV_PARENT = "./media/data/spectrum";
const std::string WCOL_NAME = "wavelength";
const std::string PCOL_NAME = "power";
const std::string SEP = ",";

template <class T> class spd {
  // sampled spectrum power distribution
public:
  unsigned int wave_start;
  unsigned int wave_end;
  std::map<unsigned int, T> wavelength_power;
  spd(unsigned int size = 471 / SPD_STRIDE) {
    wavelength_power.clear();
    std::vector<Real> powers;
    for (unsigned int i = 0; i < size; i++) {
      powers.push_back(random_real());
    }
    auto power_spd = sampled_wave<Real>(powers);
    auto ws = linspace<unsigned int>(VISIBLE_LAMBDA_START,
                                     VISIBLE_LAMBDA_END,
                                     powers.size());
    for (int i = 0; i < powers.size(); i++) {
      auto pw = make_pair(ws[i], powers[i]);
      wavelength_power.insert(pw);
    }
  }
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
  spd(const sampled_wave<T> &sampled_powers,
      unsigned int wstart, unsigned int stride = SPD_STRIDE)
      : wave_start(wstart) {
    wavelength_power.clear();
    for (unsigned int i = 0; i < sampled_powers.size();
         i++) {
      T power = sampled_powers[i];
      unsigned int wavelength =
          i + static_cast<unsigned int>(wstart);
      auto pw = make_pair(wavelength, power);
      wavelength_power.insert(pw);
    }
    wave_end = wave_start + static_cast<unsigned int>(
                                sampled_powers.size() - 1);
  }
  spd(const sampled_wave<T> &sampled_powers,
      std::vector<unsigned int> wlengths,
      unsigned int stride = SPD_STRIDE) {
    wavelength_power.clear();
    D_CHECK(sampled_powers.size() == wlengths.size());
    for (auto wl : wlengths) {
      if (wl < 0) {
        throw std::runtime_error(
            "wavelength can not be less than 0");
      }
    }
    std::map<unsigned int, T> temp;
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
      const std::function<unsigned int(unsigned int)>
          &wavelength_generator,
      const std::function<T(unsigned int)>
          &power_generator) {
    wave_start = wavelength_generator(0);
    wave_end = wavelength_generator(wave_range - 1);
    wavelength_power.clear();
    for (unsigned int i = 1; i < wave_range; i++) {
      unsigned int wave = wavelength_generator(i);
      T power_value = power_generator(wave);
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
    std::vector<T> temp;
    try {
      temp = doc.GetColumn<T>(power_col_name);
    } catch (const std::out_of_range &err) {
      throw std::runtime_error(std::string(err.what()) +
                               " :: " + power_col_name +
                               " in file :: " +
                               csv_path_abs.string());
    }
    std::vector<T> powers;
    fill_with_stride<T>(powers, temp, stride);
    std::vector<unsigned int> tempv;
    try {
      tempv = doc.GetColumn<unsigned int>(wave_col_name);
    } catch (const std::out_of_range &error) {
      throw std::runtime_error(std::string(error.what()) +
                               " :: " + wave_col_name +
                               " in file :: " +
                               csv_path_abs.string());
    }
    D_CHECK(tempv.size() == temp.size());
    std::vector<unsigned int> wlengths;
    fill_with_stride<unsigned int>(wlengths, tempv, stride);

    wavelength_power.clear();
    D_CHECK(powers.size() == wlengths.size());
    for (auto wl : wlengths) {
      if (wl < 0) {
        throw std::runtime_error(
            "wavelength can not be less than 0");
      }
    }
    std::map<unsigned int, T> tempwp;
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
  sampled_wave<T> power() const {
    std::vector<T> ps;
    for (auto pw : wavelength_power) {
      ps.push_back(pw.second);
    }
    return sampled_wave<T>(ps);
  }
  std::vector<unsigned int> wavelengths() const {
    std::vector<unsigned int> ps;
    for (auto pw : wavelength_power) {
      ps.push_back(pw.first);
    }
    std::sort(ps.begin(), ps.end(),
              [](auto i, auto j) { return i < j; });
    return ps;
  }
  spd<T> normalized() const {
    sampled_wave<T> normal_power = power();
    auto normal_power2 = normal_power.interpolate(0.0, 1.0);
    auto wlengths = wavelengths();
    auto ss = spd<T>(normal_power2, wlengths);
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
  T min_power() const {
    auto it = std::min_element(
        wavelength_power.begin(), wavelength_power.end(),
        [](const auto &k1, const auto &k2) {
          return k1.second < k2.second;
        });
    auto p =
        it == wavelength_power.end() ? -1.0 : it->second;
    return p;
  }
  T max_power() const {
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
  Real integrate(const spd<T> &nspd) const {
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
  T power_average(unsigned int w1, unsigned int w2) const {
    auto pwave1 = wavelength_power.at(w1);
    auto pwave2 = wavelength_power.at(w2);
    return (pwave1 + pwave2) / 2.0;
  }
  T power_average(const std::vector<unsigned int> &ws,
                  int lbound_pos) const {
    lbound_pos = lbound_pos == 0 ? 1 : lbound_pos;
    unsigned int w1 = ws[lbound_pos - 1];
    unsigned int w2 = ws[lbound_pos];
    return power_average(w1, w2);
  }
  bool eval_wavelength(unsigned int wave_length,
                       T &power_value) const {
    auto it = wavelength_power.find(wave_length);

    if (it != wavelength_power.end()) {
      power_value = wavelength_power.at(wave_length);
      return true;
    }
    return false;
  }
  T eval_wavelength(unsigned int wave_length) const {
    T ret;
    if (eval_wavelength(wave_length, ret))
      return ret;
    auto wlengths = wavelengths();
    auto lbound = std::lower_bound(
        wlengths.begin(), wlengths.end(), wave_length);
    if (lbound != wlengths.end()) {
      int lbound_pos = lbound - wlengths.begin();
      ret = power_average(wlengths, lbound_pos);
    } else {
      Real wave1 = min_wave();
      Real wave2 = max_wave();

      Real wave = interp<Real>(wave_length, wave1, wave2);
      auto lbound = std::lower_bound(wlengths.begin(),
                                     wlengths.end(), wave);
      int lbound_pos = lbound - wlengths.begin();
      ret = power_average(wlengths, lbound_pos);
    }
    return ret;
  }
  spd &operator+=(const spd &s) const {
    return apply(s, [](sampled_wave<T> p1, spd sp) {
      return p1 + sp.power();
    });
  }
  spd &operator+=(const Real &s) const {
    return apply(s, [](sampled_wave<T> p1, Real sp) {
      return p1 + sp;
    });
  }
  spd apply(const spd &s,
            const std::function<sampled_wave<T>(
                sampled_wave<T>, spd)> &fn) const {
    auto pw1 = power();
    auto wlengths = wavelengths();
    auto pw3 = fn(pw1, s);
    auto ss = spd<T>(pw3, wlengths, 1);
    return ss;
  }
  spd apply(const Real &s,
            const std::function<sampled_wave<T>(
                sampled_wave<T>, Real)> &fn) const {
    auto pw1 = power();
    auto wlengths = wavelengths();
    auto pw3 = fn(pw1, s);
    auto ss = spd<T>(pw3, wlengths, 1);
    return ss;
  }
  bool apply(unsigned int wave_length, T pvalue,
             const std::function<T(T, T)> &fn) {
    if (in(wave_length)) {
      T power_value = wavelength_power.at(wave_length);
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
             const std::function<bool(spd, unsigned int, T)>
                 &uneqfn) const {
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

  spd operator+(const spd &s) const {
    return apply(s, [](sampled_wave<T> p1, spd sp) {
      return p1 + sp.power();
    });
  }
  spd operator+(const Real &s) const {
    return apply(s, [](sampled_wave<T> p1, Real sp) {
      return p1 + sp;
    });
  }
  friend spd operator+(const Real &s, const spd &ss) {
    return ss + s;
  }
  spd &operator-=(const spd &s) const {
    return apply(s, [](sampled_wave<T> p1, spd sp) {
      return p1 - sp.power();
    });
  }
  spd &operator-=(const Real &s) const {
    return apply(s, [](sampled_wave<T> p1, Real sp) {
      return p1 - sp;
    });
  }
  spd operator-(const spd &s) const {
    return apply(s, [](sampled_wave<T> p1, spd sp) {
      return p1 - sp.power();
    });
  }
  spd operator-(const Real &s) const {
    return apply(s, [](sampled_wave<T> p1, Real sp) {
      return p1 - sp;
    });
  }
  friend spd operator-(const Real &s, const spd &ss) {
    return ss - s;
  }
  spd &operator*=(const spd &s) const {
    return apply(s, [](sampled_wave<T> p1, spd sp) {
      return p1 * sp.power();
    });
  }
  spd &operator*=(const Real &s) const {
    return apply(s, [](sampled_wave<T> p1, Real sp) {
      return p1 * sp;
    });
  }
  spd operator*(const spd &s) const {
    return apply(s, [](sampled_wave<T> p1, spd sp) {
      return p1 * sp.power();
    });
  }
  spd operator*(const Real &s) const {
    return apply(s, [](sampled_wave<T> p1, Real sp) {
      return p1 * sp;
    });
  }
  friend spd operator*(const Real &s, const spd &ss) {
    return ss * s;
  }
  spd &operator/=(const spd &s) const {
    return apply(s, [](sampled_wave<T> p1, spd sp) {
      return p1 / sp.power();
    });
  }
  spd &operator/=(const Real &s) const {
    return apply(s, [](sampled_wave<T> p1, Real sp) {
      return p1 + sp;
    });
  }
  spd operator/(const spd &s) const {
    return apply(s, [](sampled_wave<T> p1, spd sp) {
      return p1 / sp.power();
    });
  }
  spd operator/(const Real &s) const {
    return apply(s, [](sampled_wave<T> p1, Real sp) {
      return p1 + sp;
    });
  }
  friend spd operator/(const Real &s, const spd &ss) {
    return ss / s;
  }
  T operator[](unsigned int wave_length) const {
    return eval_wavelength(wave_length);
  }
  bool in(unsigned int wave_length) {
    auto it = wavelength_power.find(wave_length);
    if (it != wavelength_power.end()) {
      return true;
    }
    return false;
  }
  void insert(unsigned int wave_length, T pvalue) {
    if (!in(wave_length)) {
      wavelength_power.insert(
          make_pair(wave_length, pvalue));
    }
  }
  void update(unsigned int wave_length, T pvalue) {
    wavelength_power.insert_or_assign(wave_length, pvalue);
  }
  bool add(unsigned int wave_length, T pvalue) {
    return apply(wave_length, pvalue,
                 [](T i, T j) { return i + j; });
  }
  spd add(const spd &s) const {
    // slightly more robust implementation of addition
    return rapply(
        s, [](auto res, auto ss) { return res + ss; },
        [](auto res, auto wave, auto power_value) {
          return res.add(wave, power_value);
        });
  }
  spd add(const Real &s) const { return *this + s; }
  bool subt(unsigned int wave_length, T pvalue) {
    return apply(wave_length, pvalue,
                 [](T i, T j) { return i - j; });
  }
  spd subt(const spd &s) const {
    return rapply(
        s, [](auto res, auto ss) { return res - ss; },
        [](auto res, auto wave, auto power_value) {
          return res.subt(wave, power_value);
        });
  }
  spd subt(const Real &s) const { return *this - s; }
  spd multip(const spd &s) const {
    return rapply(
        s, [](auto res, auto ss) { return res * ss; },
        [](auto res, auto wave, auto power_value) {
          return res.multip(wave, power_value);
        });
  }
  spd multip(const Real &s) const { return *this * s; }
  bool multip(unsigned int wave_length, T pvalue) {
    return apply(wave_length, pvalue,
                 [](T i, T j) { return i * j; });
  }
  spd div(const spd &s) const {
    return rapply(
        s, [](auto res, auto ss) { return res / ss; },
        [](auto res, auto wave, auto power_value) {
          return res.div(wave, power_value);
        });
  }
  spd div(const Real &s) const { return *this / s; }
  bool div(unsigned int wave_length, T pvalue) {
    return apply(wave_length, pvalue,
                 [](T i, T j) { return i / j; });
  }
};
Real get_cie_k(const spd<Real> &ss, const spd<Real> &cie_y,
               int wave_length_start, int wave_length_end,
               int stepsize = 0) {
  Real sum = 0.0;
  if (stepsize > 0) {
    for (int i = wave_length_start; i < wave_length_end;
         i += stepsize) {
      Real w1 =
          ss.eval_wavelength(static_cast<unsigned int>(i));
      Real w2 = cie_y.eval_wavelength(
          static_cast<unsigned int>(i));
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
  unsigned int w1 = qlambda.min_wave();
  unsigned int w2 = qlambda.max_wave();

  Real stepsize =
      (w2 - w1) / static_cast<Real>(qlambda.size());
  Real sum = 0.0;
  auto waves = qlambda.wavelengths();
  for (auto w : waves) {
    auto pw1 = qlambda.eval_wavelength(w);
    auto pw2 = rlambda.eval_wavelength(w);
    auto pw3 = cie_bar.eval_wavelength(w);
    sum += (pw1 * pw2 * pw3);
  }
  return sum * stepsize;
}
Real get_cie_val(const spd<Real> &rs, const spd<Real> &ss,
                 const spd<Real> &cie_spd,
                 unsigned int wl_start, unsigned int wl_end,
                 unsigned int stepsize = 0) {
  Real sum = 0.0;
  if (stepsize > 0) {
    for (unsigned int i = wl_start; i < wl_end;
         i += stepsize) {
      Real w1 = ss.eval_wavelength(i);
      Real w2 = cie_spd.eval_wavelength(i);
      Real w3 = rs.eval_wavelength(i);
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
                   unsigned int wl_start,
                   unsigned int wl_end,
                   unsigned int stepsize = 0) {
  Real k = get_cie_k(illuminant, cie_y, wl_start, wl_end,
                     stepsize);
  Real val = get_cie_val(reflectance, illuminant, cie_val,
                         wl_start, wl_end, stepsize);
  return k * val;
}

auto rho_rspd =
    spd<Real>(CSV_PARENT / "rho-r-2012.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);
static spd<Real> rho_r = rho_rspd.normalized();

auto rho_gspd =
    spd<Real>(CSV_PARENT / "rho-g-2012.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);
static spd<Real> rho_g = rho_gspd.normalized();

auto rho_bspd =
    spd<Real>(CSV_PARENT / "rho-b-2012.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);
static spd<Real> rho_b = rho_bspd.normalized();

auto cie_xbarspd =
    spd<Real>(CSV_PARENT / "cie-x-bar-1964.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);

static spd<Real> cie_xbar = cie_xbarspd.normalized();

auto cie_ybarspd =
    spd<Real>(CSV_PARENT / "cie-y-bar-1964.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);

static spd<Real> cie_ybar = cie_ybarspd.normalized();

auto cie_zbarspd =
    spd<Real>(CSV_PARENT / "cie-z-bar-1964.csv", WCOL_NAME,
              PCOL_NAME, SEP, SPD_STRIDE);
static spd<Real> cie_zbar = cie_zbarspd.normalized();

auto stand_d65 =
    spd<Real>(CSV_PARENT / "cie-d65-standard.csv",
              WCOL_NAME, PCOL_NAME, SEP, SPD_STRIDE);
static spd<Real> standard_d65 = stand_d65.normalized();
//

inline vec3 xyz2rgb(const vec3 xyz) {
  auto r = 3.240479 * xyz.x() - 1.537150 * xyz.y() -
           0.498535 * xyz.z();
  auto g = -0.969256 * xyz.x() + 1.875991 * xyz.y() +
           0.041556 * xyz.z();
  auto b = 0.055648 * xyz.x() - 0.204043 * xyz.y() +
           1.057311 * xyz.z();
  return vec3(r, g, b);
}
vec3 to_xyz(const spd<Real> &s_lambda) {
  auto X = get_cie_val(s_lambda, cie_xbar);
  auto Y = get_cie_val(s_lambda, cie_ybar);
  auto Z = get_cie_val(s_lambda, cie_zbar);
  auto xyzsum = X + Y + Z;
  xyzsum = xyzsum == 0 ? 1 : xyzsum;
  auto xval = X / xyzsum;
  auto yval = Y / xyzsum;
  auto zval = 1.0 - (xval + yval);
  vec3 xyz = vec3(xval, yval, zval);
  return xyz;
}
vec3 to_xyz(const spd<Real> &s_lambda,
            const spd<Real> &r_lambda) {
  auto X = get_cie_val(s_lambda, r_lambda, cie_xbar);
  auto Y = get_cie_val(s_lambda, r_lambda, cie_ybar);
  auto Z = get_cie_val(s_lambda, r_lambda, cie_zbar);
  auto xyzsum = X + Y + Z;
  xyzsum = xyzsum == 0 ? 1 : xyzsum;
  auto xval = X / xyzsum;
  auto yval = Y / xyzsum;
  auto zval = 1.0 - (xval + yval);
  vec3 xyz = vec3(xval, yval, zval);
  return xyz;
}
vec3 to_color(const spd<Real> &s_lambda) {
  return xyz2rgb(to_xyz(s_lambda));
}
vec3 to_color(const spd<Real> &s_lambda,
              const spd<Real> &r_lambda) {
  return xyz2rgb(to_xyz(s_lambda, r_lambda));
}
}
