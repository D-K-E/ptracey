#pragma once
#include <algorithm>
#include <common.hpp>
#include <stdexcept>
#include <utils.hpp>

template <class T> class sampled_wave {
public:
  std::vector<T> values;
  sampled_wave(T val, unsigned int size) {
    values.clear();
    for (unsigned int i = 0; i < size; i++) {
      values.push_back(val);
    }
    D_CHECK(!has_nans());
  }
  sampled_wave(std::vector<T> vals) {
    values.clear();
    for (const auto &v : vals)
      values.push_back(v);
    D_CHECK(!has_nans());
  }
  T max() const {
    return *std::max_element(values.begin(), values.end());
  }
  T min() const {
    return *std::min_element(values.begin(), values.end());
  }
  bool has_nans() const {
    for (auto val : values) {
      if (isnan(val))
        return true;
    }
    return false;
  }
  bool has_zeros() const {
    for (auto val : values) {
      if (val == 0.0)
        return true;
    }
    return false;
  }
  bool is_black() const {
    for (auto val : values) {
      if (val != 0.0)
        return false;
    }
    return true;
  }
  sampled_wave &operator=(const sampled_wave &s) {
    D_CHECK(!s.has_nans());
    values = s.values;
    return *this;
  }
  sampled_wave &operator+=(const sampled_wave &s) {
    D_CHECK(!s.has_nans());
    D_CHECK(s.values.size() == values.size());
    for (unsigned int i = 0; i < values.size(); i++) {
      values[i] += s.values[i];
    }
    return *this;
  }
  sampled_wave &operator+=(const T &s) {
    for (unsigned int i = 0; i < values.size(); i++) {
      values[i] += s;
    }
    return *this;
  }
  sampled_wave operator+(const sampled_wave &s) const {
    D_CHECK(!s.has_nans());
    D_CHECK(s.values.size() == values.size());
    sampled_wave ss = *this;
    for (unsigned int i = 0; i < values.size(); i++) {
      ss.values[i] += s.values[i];
    }
    return ss;
  }
  sampled_wave operator+(const T &s) const {
    sampled_wave ss = *this;
    for (unsigned int i = 0; i < values.size(); i++) {
      ss.values[i] += s;
    }
    return ss;
  }
  friend sampled_wave operator+(const T &s,
                                const sampled_wave &ss) {
    return ss + s;
  }
  sampled_wave &operator-=(const sampled_wave &s) {
    D_CHECK(!s.has_nans());
    D_CHECK(s.values.size() == values.size());
    for (unsigned int i = 0; i < values.size(); i++) {
      values[i] -= s.values[i];
    }
    return *this;
  }
  sampled_wave &operator-=(const T &s) {
    for (unsigned int i = 0; i < values.size(); i++) {
      values[i] -= s;
    }
    return *this;
  }
  sampled_wave operator-(const sampled_wave &s) const {
    D_CHECK(!s.has_nans());
    D_CHECK(s.values.size() == values.size());
    sampled_wave ss = *this;
    for (unsigned int i = 0; i < values.size(); i++) {
      ss.values[i] -= s.values[i];
    }
    return ss;
  }
  sampled_wave operator-(const T &s) const {
    sampled_wave ss = *this;
    for (unsigned int i = 0; i < values.size(); i++) {
      ss.values[i] -= s;
    }
    return ss;
  }
  friend sampled_wave operator-(const T &s,
                                const sampled_wave &ss) {
    return ss - s;
  }
  sampled_wave &operator*=(const sampled_wave &s) {
    D_CHECK(!s.has_nans());
    D_CHECK(s.values.size() == values.size());
    for (unsigned int i = 0; i < values.size(); i++) {
      values[i] *= s.values[i];
    }
    return *this;
  }
  sampled_wave &operator*=(const T &s) {
    for (unsigned int i = 0; i < values.size(); i++) {
      values[i] *= s;
    }
    return *this;
  }
  sampled_wave operator*(const sampled_wave &s) const {
    D_CHECK(!s.has_nans());
    D_CHECK(s.values.size() == values.size());
    sampled_wave ss = *this;
    for (unsigned int i = 0; i < values.size(); i++) {
      ss.values[i] *= s.values[i];
    }
    return ss;
  }
  sampled_wave operator*(const T &s) const {
    sampled_wave ss = *this;
    for (unsigned int i = 0; i < values.size(); i++) {
      ss.values[i] *= s;
    }
    return ss;
  }
  friend sampled_wave operator*(const T &s,
                                const sampled_wave &ss) {
    return ss * s;
  }
  sampled_wave &operator/=(const sampled_wave &s) {
    D_CHECK(!s.has_nans());
    D_CHECK(!s.has_zeros());
    D_CHECK(s.values.size() == values.size());
    for (unsigned int i = 0; i < values.size(); i++) {
      values[i] /= s.values[i];
    }
    return *this;
  }
  sampled_wave &operator/=(const T &s) {
    D_CHECK(s != 0.0);
    for (unsigned int i = 0; i < values.size(); i++) {
      values[i] /= s;
    }
    return *this;
  }
  sampled_wave operator/(const sampled_wave &s) const {
    D_CHECK(!s.has_nans());
    D_CHECK(!s.has_zeros());
    D_CHECK(s.values.size() == values.size());
    sampled_wave ss = *this;
    for (unsigned int i = 0; i < values.size(); i++) {
      ss.values[i] /= s.values[i];
    }
    return ss;
  }
  sampled_wave operator/(const T &s) const {
    D_CHECK(s != 0.0);
    sampled_wave ss = *this;
    for (unsigned int i = 0; i < values.size(); i++) {
      ss.values[i] /= s;
    }
    return ss;
  }
  friend sampled_wave operator/(const T &s,
                                const sampled_wave &ss) {
    D_CHECK(!ss.has_nans());
    D_CHECK(!ss.has_zeros());
    D_CHECK(s != 0.0);
    sampled_wave sss = ss;
    for (unsigned int i = 0; i < sss.values.size(); i++) {
      sss.values[i] = s / sss.values[i];
    }
    return sss;
  }
  T &operator[](int i) {
    D_CHECK(i >= 0 && i < values.size());
    return values[i];
  }
  T operator[](int i) const {
    D_CHECK(i >= 0 && i < values.size());
    return values[i];
  }
  bool operator==(const sampled_wave &cs) const {
    D_CHECK(!cs.has_nans());
    for (unsigned int i = 0; i < values.size(); i++) {
      if (cs.values[i] != values[i])
        return false;
    }
    return true;
  }
  bool operator!=(const sampled_wave &cs) const {
    return !(*this == cs);
  }
  double sum() const {
    double s = 0.0;
    for (auto v : values)
      s += v;
    return s;
  }
  double product() const {
    double s = 0.0;
    for (auto v : values)
      s *= v;
    return s;
  }
  double average() const {
    return sum() / (int)values.size();
  }
  std::size_t size() const { return values.size(); }
  sampled_wave clamp(T low = 0.0, T high = FLT_MAX) {
    sampled_wave cs = *this;
    for (int i = 0; i < values.size(); i++) {
      cs.values[i] = dclamp<T>(values[i], low, high);
    }
    D_CHECK(!cs.has_nans());
    return cs;
  }
  sampled_wave interpolate(T low = 0.0, T high = FLT_MAX) {
    sampled_wave cs = *this;
    for (int i = 0; i < values.size(); i++) {
      cs.values[i] = interp<T>(values[i], low, high);
    }
    D_CHECK(!cs.has_nans());
    return cs;
  }
};
template <class T> class sampled_spd {
  // sampled spectrum power distribution
public:
  Real wave_start;
  Real wave_end;
  std::vector<std::pair<T, Real>> power_wavelengths;
  sampled_spd(const sampled_wave<T> &sampled_powers,
              unsigned int wstart)
      : wave_start((Real)wstart) {
    power_wavelengths.clear();
    for (int i = 0; i < sampled_powers.size(); i++) {
      T power = sampled_powers[i];
      Real wavelength = i + static_cast<Real>(wstart);
      auto pw = make_pair(power, wavelength);
      power_wavelengths.push_back(pw);
    }
    wave_end = wave_start +
               static_cast<Real>(sampled_powers.size() - 1);
  }
  sampled_spd(const sampled_wave<T> &sampled_powers,
              Real wstart, Real wend)
      : wave_start(wstart), wave_end(wend) {
    power_wavelengths.clear();
    Real tmn = std::min(wstart, wend);
    wend = tmn == wstart ? wend : wstart;
    auto ws =
        linspace<Real>(tmn, wend, sampled_powers.size());
    for (int i = 0; i < sampled_powers.size(); i++) {
      auto pw = make_pair(sampled_powers[i], ws[i]);
      power_wavelengths.push_back(pw);
    }
  }
  sampled_spd(const sampled_wave<T> &sampled_powers,
              std::vector<Real> wlengths) {
    power_wavelengths.clear();
    D_CHECK(sampled_powers.size() == wlengths.size());
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
    for (int i = 0; i < sampled_powers.size(); i++) {
      auto pw = make_pair(sampled_powers[i], wlengths[i]);
      power_wavelengths.push_back(pw);
    }
  }
  sampled_wave<T> power() const {
    std::vector<T> ps;
    for (auto pw : power_wavelengths) {
      ps.push_back(pw.first);
    }
    return sampled_wave<T>(ps);
  }
  std::vector<Real> wavelength() const {
    std::vector<T> ps;
    for (auto pw : power_wavelengths) {
      ps.push_back(pw.second);
    }
    return ps;
  }
  sampled_spd<T> normalized() const {
    auto normal_wave = power().interpolate(0.0, 1.0);
    return sampled_spd<T>(normal_wave);
  }
  void normalize() {
    auto spd = normalized();
    *this = spd;
  }
  Real integrate(const sampled_wave<T> &vs) {
    D_CHECK(power_wavelengths.size() == vs.size());
    Real x1 = power_wavelengths[0].second;
    Real x2 =
        power_wavelengths[power_wavelengths.size() - 1]
            .second;
    Real stepsize =
        (x2 - x1) /
        static_cast<Real>(power_wavelengths.size());
    Real val = 0.0;
    for (int i = 0; i < power_wavelengths.size(); i++) {
      auto pw = power_wavelengths[i];
      auto v = vs[i];
      val += pw.first * v;
    }
    return val * stepsize;
  }
  T eval_wavelength(Real wave_length) const {
    auto wlengths = wavelength();
    auto lbound = std::lower_bound(
        wlengths.begin(), wlengths.end(), wave_length);
    T ret;
    if (lbound != wlengths.end()) {
      auto lbound_pos = lbound - wlengths.begin();
      lbound_pos = lbound_pos == 0 ? 1 : lbound_pos;
      auto pwave1 = power_wavelengths[lbound_pos - 1].first;
      auto pwave2 = power_wavelengths[lbound_pos].first;
      ret = (pwave1 + pwave2) / 2.0;
    } else {
      Real wave1 = power_wavelengths[0].second;
      Real wave2 =
          power_wavelengths[power_wavelengths.size() - 1]
              .second;
      Real wave = interp<Real>(wave_length, wave1, wave2);
      auto lbound = std::lower_bound(wlengths.begin(),
                                     wlengths.end(), wave);
      auto lbound_pos = lbound - wlengths.begin();
      lbound_pos = lbound_pos == 0 ? 1 : lbound_pos;
      auto pwave1 = power_wavelengths[lbound_pos - 1].first;
      auto pwave2 = power_wavelengths[lbound_pos].first;
      ret = (pwave1 + pwave2) / 2.0;
    }
    return ret;
  }
};

Real get_cie_k(const sampled_spd<Real> &ss,
               const sampled_spd<Real> &cie_y,
               int wave_length_start, int wave_length_end,
               int stepsize = 0) {
  Real sum = 0.0;
  if (stepsize > 0) {
    for (int i = wave_length_start; i < wave_length_end;
         i += stepsize) {
      Real w1 = ss.eval_wavelength(i);
      Real w2 = cie_y.eval_wavelength(i);
      sum += (w1 * w2);
    }
    return 100 / sum;
  }
  if (ss.power_wavelengths.size() !=
      cie_y.power_wavelengths.size()) {
    throw std::runtime_error("if step size is 0, spd and "
                             "cie_y should have same size");
  }
  auto pw1 = ss.power();
  auto pw2 = cie_y.power();
  for (int i = 0; i < ss.power_wavelengths.size(); i++) {
    sum += pw1[i] * pw2[i];
  }
  return 100.0 / sum;
}
Real get_cie_val(const sampled_spd<Real> &rs,
                 const sampled_spd<Real> &ss,
                 const sampled_spd<Real> &cie_spd,
                 int wl_start, int wl_end,
                 int stepsize = 0) {
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
  if (ss.power_wavelengths.size() !=
          cie_spd.power_wavelengths.size() ||
      cie_spd.power_wavelengths.size() !=
          rs.power_wavelengths.size()) {
    throw std::runtime_error("if step size is 0, spd and "
                             "cie_y should have same size");
  }
  auto pw1 = ss.power();
  auto pw2 = cie_spd.power();
  auto pw3 = rs.power();
  for (int i = 0; i < ss.power_wavelengths.size(); i++) {
    sum += pw1[i] * pw2[i] * pw3[i];
  }
  return sum;
}

Real get_cie_value(const sampled_spd<Real> &reflectance,
                   const sampled_spd<Real> &illuminant,
                   const sampled_spd<Real> &cie_val,
                   const sampled_spd<Real> &cie_y,
                   int wl_start, int wl_end,
                   int stepsize = 0) {
  Real k = get_cie_k(illuminant, cie_y, wl_start, wl_end,
                     stepsize);
  Real val = get_cie_val(reflectance, illuminant, cie_val,
                         wl_start, wl_end, stepsize);
  return k * val;
}
