#pragma once

#include <common.hpp>
#include <stdexcept>
#include <utils.hpp>
#include <wave.hpp>

template <class T> class spd {
  // sampled spectrum power distribution
public:
  Real wave_start;
  Real wave_end;
  std::map<Real, T> wavelength_power;
  spd(unsigned int size = 60) {
    wavelength_power.clear();
    std::vector<Real> powers;
    for (unsigned int i = 0; i < size; i++) {
      powers.push_back(random_double());
    }
    auto power_spd = sampled_wave<Real>(powers);
    auto ws = linspace<Real>(400, 750, powers.size());
    for (int i = 0; i < powers.size(); i++) {
      auto pw = make_pair(ws[i], powers[i]);
      wavelength_power.insert(pw);
    }
  }
  spd(const sampled_wave<T> &sampled_powers,
      unsigned int wstart)
      : wave_start((Real)wstart) {
    wavelength_power.clear();
    for (int i = 0; i < sampled_powers.size(); i++) {
      T power = sampled_powers[i];
      Real wavelength = i + static_cast<Real>(wstart);
      auto pw = make_pair(wavelength, power);
      wavelength_power.insert(pw);
    }
    wave_end = wave_start +
               static_cast<Real>(sampled_powers.size() - 1);
  }
  spd(const sampled_wave<T> &sampled_powers, Real wstart,
      Real wend)
      : wave_start(wstart), wave_end(wend) {
    wavelength_power.clear();
    Real tmn = std::min(wstart, wend);
    wend = tmn == wstart ? wend : wstart;
    auto ws =
        linspace<Real>(tmn, wend, sampled_powers.size());
    for (int i = 0; i < sampled_powers.size(); i++) {
      auto pw = make_pair(ws[i], sampled_powers[i]);
      wavelength_power.insert(pw);
    }
  }
  spd(const sampled_wave<T> &sampled_powers,
      std::vector<Real> wlengths) {
    wavelength_power.clear();
    D_CHECK(sampled_powers.size() == wlengths.size());
    for (auto wl : wlengths) {
      if (wl < 0) {
        throw std::runtime_error(
            "wavelength can not be less than 0");
      }
    }
    std::map<Real, T> temp;
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
  spd(const path &csv_path,
      const std::string &wave_col_name = "wavelength",
      const std::string &power_col_name = "power",
      const std::string &sep = ",") {
    wavelength_power.clear();
    path csv_path_abs = RUNTIME_PATH / csv_path;
    rapidcsv::Document doc(csv_path_abs.string(),
                           rapidcsv::LabelParams(0, -1),
                           rapidcsv::SeparatorParams(','));
    std::vector<T> powers;
    try {
      powers = doc.GetColumn<T>(power_col_name);
    } catch (const std::out_of_range &err) {
      throw std::runtime_error(std::string(err.what()) +
                               " :: " + power_col_name +
                               " in file :: " +
                               csv_path_abs.string());
    }
    std::vector<Real> wlengths;
    try {
      wlengths = doc.GetColumn<T>(wave_col_name);
    } catch (const std::out_of_range &error) {
      throw std::runtime_error(std::string(error.what()) +
                               " :: " + wave_col_name +
                               " in file :: " +
                               csv_path_abs.string());
    }
    wavelength_power.clear();
    D_CHECK(powers.size() == wlengths.size());
    for (auto wl : wlengths) {
      if (wl < 0) {
        throw std::runtime_error(
            "wavelength can not be less than 0");
      }
    }
    std::sort(wlengths.begin(), wlengths.end(),
              [](auto i, auto j) { return i < j; });
    wave_start = wlengths[0];
    wave_end = wlengths[wlengths.size() - 1];
    for (int i = 0; i < powers.size(); i++) {
      auto pw = make_pair(powers[i], wlengths[i]);
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
  std::vector<Real> wavelengths() const {
    std::vector<Real> ps;
    for (auto pw : wavelength_power) {
      ps.push_back(pw.first);
    }
    std::sort(ps.begin(), ps.end(),
              [](auto i, auto j) { return i < j; });
    return ps;
  }
  spd<T> normalized() const {
    auto normal_wave = power().interpolate(0.0, 1.0);
    return spd<T>(normal_wave, wave_start, wave_end);
  }
  void normalize() {
    auto spd = normalized();
    *this = spd;
  }
  Real min_wave() const {
    auto it = std::min_element(
        wavelength_power.begin(), wavelength_power.end(),
        [](const auto &k1, const auto &k2) {
          return k1.first < k2.first;
        });
    auto wavel =
        it == wavelength_power.end() ? -1.0 : it->first;
    return wavel;
  }
  Real max_wave() const {
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
    for (int i = 0; i < (int)size(); i++) {
      auto pw = eval_wavelength(waves[i]);
      auto v = nspd.eval_wavelength(waves[i]);
      val += pw * v;
    }
    return val * stepsize;
  }
  T power_average(Real w1, Real w2) const {
    auto pwave1 = wavelength_power.at(w1);
    auto pwave2 = wavelength_power.at(w2);
    return (pwave1 + pwave2) / 2.0;
  }
  T power_average(const std::vector<Real> &ws,
                  int lbound_pos) const {
    lbound_pos = lbound_pos == 0 ? 1 : lbound_pos;
    Real w1 = ws[lbound_pos - 1];
    Real w2 = ws[lbound_pos];
    return power_average(w1, w2);
  }
  bool eval_wavelength(Real wave_length,
                       T &power_value) const {
    auto it = wavelength_power.find(wave_length);

    if (it != wavelength_power.end()) {
      power_value = wavelength_power.at(wave_length);
      return true;
    }
    return false;
  }
  T eval_wavelength(Real wave_length) const {
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
  spd &operator+=(const spd &s) {
    auto pw1 = power();
    auto pw2 = s.power();
    pw1 += pw2;
    auto wlengths = wavelengths();
    auto ss = spd(pw1, wlengths);
    return ss;
  }
  spd &operator+=(const Real &s) {
    auto pw1 = power();
    pw1 += s;
    auto wlengths = wavelengths();
    auto ss = spd(pw1, wlengths);
    return ss;
  }
  spd operator+(const spd &s) const {
    auto pw1 = power();
    auto pw2 = s.power();
    auto wlengths = wavelengths();
    auto pw3 = pw1 + pw2;
    auto ss = spd<T>(pw3, wlengths);
    return ss;
  }
  spd operator+(const Real &s) const {
    auto pw1 = power();
    auto pw2 = pw1 + s;
    auto wlengths = wavelengths();
    auto ss = spd(pw2, wlengths);
    return ss;
  }
  friend spd operator+(const Real &s, const spd &ss) {
    return ss + s;
  }
  spd &operator-=(const spd &s) {
    auto pw1 = power();
    auto pw2 = s.power();
    pw1 -= pw2;
    auto wlengths = wavelengths();
    auto ss = spd(pw1, wlengths);
    return ss;
  }
  spd &operator-=(const Real &s) {
    auto pw1 = power();
    pw1 -= s;
    auto wlengths = wavelengths();
    auto ss = spd(pw1, wlengths);
    return ss;
  }
  spd operator-(const spd &s) const {
    auto pw1 = power();
    auto pw2 = s.power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 - pw2, wlengths);
    return ss;
  }
  spd operator-(const Real &s) const {
    auto pw1 = power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 - s, wlengths);
    return ss;
  }
  friend spd operator-(const Real &s, const spd &ss) {
    return ss - s;
  }
  spd &operator*=(const spd &s) {
    auto pw1 = power();
    auto pw2 = s.power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 * pw2, wlengths);
    return ss;
  }
  spd &operator*=(const Real &s) {
    auto pw1 = power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 * s, wlengths);
    return ss;
  }
  spd operator*(const spd &s) const {
    auto pw1 = power();
    auto pw2 = s.power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 * pw2, wlengths);
    return ss;
  }
  spd operator*(const Real &s) const {
    auto pw1 = power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 * s, wlengths);
    return ss;
  }
  friend spd operator*(const Real &s, const spd &ss) {
    return ss * s;
  }
  spd &operator/=(const spd &s) {
    auto pw1 = power();
    auto pw2 = s.power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 / pw2, wlengths);
    return ss;
  }
  spd &operator/=(const Real &s) {
    auto pw1 = power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 / s, wlengths);
    return ss;
  }
  spd &operator/(const spd &s) {
    auto pw1 = power();
    auto pw2 = s.power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 / pw2, wlengths);
    return ss;
  }
  spd &operator/(const Real &s) {
    auto pw1 = power();
    auto wlengths = wavelengths();
    auto ss = spd(pw1 / s, wlengths);
    return ss;
  }
  friend spd operator/(const Real &s, const spd &ss) {
    return ss / s;
  }
  spd add(const spd &s) const { return *this + s; }
  spd add(const Real &s) const { return *this + s; }
  spd subt(const spd &s) const { return *this - s; }
  spd subt(const Real &s) const { return *this - s; }
  spd multip(const spd &s) const { return *this * s; }
  spd multip(const Real &s) const { return *this * s; }
  spd div(const spd &s) const { return *this / s; }
  spd div(const Real &s) const { return *this / s; }
};
Real get_cie_k(const spd<Real> &ss, const spd<Real> &cie_y,
               int wave_length_start, int wave_length_end,
               int stepsize = 0) {
  Real sum = 0.0;
  if (stepsize > 0) {
    for (int i = wave_length_start; i < wave_length_end;
         i += stepsize) {
      Real w1 = ss.eval_wavelength((Real)i);
      Real w2 = cie_y.eval_wavelength((Real)i);
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
  Real w1 = qlambda.min_wave();
  Real w2 = qlambda.max_wave();

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
                 const spd<Real> &cie_spd, int wl_start,
                 int wl_end, int stepsize = 0) {
  Real sum = 0.0;
  if (stepsize > 0) {
    for (int i = wl_start; i < wl_end; i += stepsize) {
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
  for (int i = 0; i < ss.wavelength_power.size(); i++) {
    sum += pw1[i] * pw2[i] * pw3[i];
  }
  return sum;
}
Real get_cie_value(const spd<Real> &reflectance,
                   const spd<Real> &illuminant,
                   const spd<Real> &cie_val,
                   const spd<Real> &cie_y, int wl_start,
                   int wl_end, int stepsize = 0) {
  Real k = get_cie_k(illuminant, cie_y, wl_start, wl_end,
                     stepsize);
  Real val = get_cie_val(reflectance, illuminant, cie_val,
                         wl_start, wl_end, stepsize);
  return k * val;
}
