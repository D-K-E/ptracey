#pragma once
#include <external.hpp>
#include <utils.hpp>
// wave implementation
using namespace ptracey;
namespace ptracey {
class sinwave {
public:
  unsigned int sample_nb;
  std::vector<double> values;
  double phase = 0.0;
  double freq = 1.0;
  double amplitude = 1.0;
  sinwave(unsigned int nb_sample = 60)
      : sample_nb(nb_sample) {
    values.clear();
    for (unsigned int i = 1; i < nb_sample; i++) {
      values.push_back(sinmap(static_cast<double>(i)));
    }
  }
  sinwave(unsigned int nb_sample, double _f, double _amp,
          double _phase)
      : sample_nb(nb_sample) {
    set_coeffs(_f, _amp, _phase);
    values.clear();
    for (unsigned int i = 1; i < nb_sample; i++) {
      values.push_back(sinmap(static_cast<double>(i)));
    }
  }
  sinwave(std::vector<double> vals, double _f, double _amp,
          double _phase)
      : sample_nb(static_cast<unsigned int>(vals.size())) {
    set_coeffs(_f, _amp, _phase);
    values.clear();
    for (auto v : vals) {
      values.push_back(sinmap(v));
    }
  }
  sinwave(const sinwave &s) {
    D_CHECK(!s.has_nans());
    values = s.values;
    sample_nb = s.sample_nb;
    freq = s.freq;
    phase = s.phase;
    amplitude = s.amplitude;
  }
  void set_coeffs(double _f, double _a, double _b) {
    if (_f <= 0.0) {
      throw std::runtime_error("frequency value can not be "
                               "less than or equal to 0: " +
                               std::to_string(_f));
    }
    if (_a <= 0.0) {
      throw std::runtime_error("amplitude value can not be "
                               "less than or equal to 0: " +
                               std::to_string(_a));
    }

    freq = _f;
    amplitude = _a;
    phase = _b;
  }
  double sinmap(double x) {
    return amplitude * sin(freq * (x + phase));
  }
  double max() const {
    return *std::max_element(values.begin(), values.end());
  }
  double min() const {
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
  sinwave &operator=(const sinwave &s) {
    D_CHECK(!s.has_nans());
    values = s.values;
    sample_nb = s.sample_nb;
    freq = s.freq;
    phase = s.phase;
    amplitude = s.amplitude;
    return *this;
  }
  sinwave &operator+=(const sinwave &s) {
    D_CHECK(!s.has_nans());
    D_CHECK(s.sample_nb == sample_nb);
    for (unsigned int i = 0; i < sample_nb; i++) {
      values[i] += s.values[i];
    }
    return *this;
  }
  sinwave &operator+=(const double &s) {
    for (unsigned int i = 0; i < sample_nb; i++) {
      values[i] += s;
    }
    return *this;
  }
  sinwave operator+(const sinwave &s) const {
    D_CHECK(!s.has_nans());
    D_CHECK(s.sample_nb == sample_nb);
    sinwave ss = *this;
    for (unsigned int i = 0; i < sample_nb; i++) {
      ss.values[i] += s.values[i];
    }
    return ss;
  }
  sinwave operator+(const double &s) const {
    sinwave ss = *this;
    for (unsigned int i = 0; i < sample_nb; i++) {
      ss.values[i] += s;
    }
    return ss;
  }
  friend sinwave operator+(const double &s,
                           const sinwave &ss) {
    return ss + s;
  }
  sinwave &operator-=(const sinwave &s) {
    D_CHECK(!s.has_nans());
    D_CHECK(s.sample_nb == sample_nb);
    for (unsigned int i = 0; i < sample_nb; i++) {
      values[i] -= s.values[i];
    }
    return *this;
  }
  sinwave &operator-=(const double &s) {
    for (unsigned int i = 0; i < sample_nb; i++) {
      values[i] -= s;
    }
    return *this;
  }
  sinwave operator-(const sinwave &s) const {
    D_CHECK(!s.has_nans());
    D_CHECK(s.sample_nb == sample_nb);
    sinwave ss = *this;
    for (unsigned int i = 0; i < sample_nb; i++) {
      ss.values[i] -= s.values[i];
    }
    return ss;
  }
  sinwave operator-(const double &s) const {
    sinwave ss = *this;
    for (unsigned int i = 0; i < sample_nb; i++) {
      ss.values[i] -= s;
    }
    return ss;
  }
  friend sinwave operator-(const double &s,
                           const sinwave &ss) {
    return ss - s;
  }
  sinwave &operator*=(const sinwave &s) {
    D_CHECK(!s.has_nans());
    D_CHECK(s.sample_nb == sample_nb);
    for (unsigned int i = 0; i < sample_nb; i++) {
      values[i] *= s.values[i];
    }
    return *this;
  }
  sinwave &operator*=(const double &s) {
    for (unsigned int i = 0; i < sample_nb; i++) {
      values[i] *= s;
    }
    return *this;
  }
  sinwave operator*(const sinwave &s) const {
    D_CHECK(!s.has_nans());
    D_CHECK(s.sample_nb == sample_nb);
    sinwave ss = *this;
    for (unsigned int i = 0; i < sample_nb; i++) {
      ss.values[i] *= s.values[i];
    }
    return ss;
  }
  sinwave operator*(const double &s) const {
    sinwave ss = *this;
    for (unsigned int i = 0; i < sample_nb; i++) {
      ss.values[i] *= s;
    }
    return ss;
  }
  friend sinwave operator*(const double &s,
                           const sinwave &ss) {
    return ss * s;
  }
  sinwave &operator/=(const sinwave &s) {
    D_CHECK(!s.has_nans());
    D_CHECK(!s.has_zeros());
    D_CHECK(s.sample_nb == sample_nb);
    for (unsigned int i = 0; i < sample_nb; i++) {
      values[i] /= s.values[i];
    }
    return *this;
  }
  sinwave &operator/=(const double &s) {
    D_CHECK(s != 0.0);
    for (unsigned int i = 0; i < sample_nb; i++) {
      values[i] /= s;
    }
    return *this;
  }
  sinwave operator/(const sinwave &s) const {
    D_CHECK(!s.has_nans());
    D_CHECK(!s.has_zeros());
    D_CHECK(s.sample_nb == sample_nb);
    sinwave ss = *this;
    for (unsigned int i = 0; i < sample_nb; i++) {
      ss.values[i] /= s.values[i];
    }
    return ss;
  }
  sinwave operator/(const double &s) const {
    D_CHECK(s != 0.0);
    sinwave ss = *this;
    for (unsigned int i = 0; i < sample_nb; i++) {
      ss.values[i] /= s;
    }
    return ss;
  }
  friend sinwave operator/(const double &s,
                           const sinwave &ss) {
    D_CHECK(!ss.has_nans());
    D_CHECK(!ss.has_zeros());
    D_CHECK(s != 0.0);
    sinwave sss = ss;
    for (unsigned int i = 0; i < sss.sample_nb; i++) {
      sss.values[i] = s / sss.values[i];
    }
    return sss;
  }
  double &operator[](int i) {
    D_CHECK(i >= 0 && i < sample_nb);
    return values[i];
  }
  double operator[](int i) const {
    D_CHECK(i >= 0 && i < sample_nb);
    return values[i];
  }
  bool operator==(const sinwave &cs) const {
    D_CHECK(!cs.has_nans());
    for (unsigned int i = 0; i < sample_nb; i++) {
      if (cs.values[i] != values[i])
        return false;
    }
    return true;
  }
  bool operator!=(const sinwave &cs) const {
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
  sinwave clamp(double low = 0.0, double high = FLT_MAX) {
    sinwave cs = *this;
    for (int i = 0; i < sample_nb; i++) {
      cs.values[i] = dclamp<double>(values[i], low, high);
    }
    D_CHECK(!cs.has_nans());
    return cs;
  }
  double wavelength() const { return fabs(phase / freq); }
  static sinwave fromWaveLength(unsigned int nb_sample,
                                double wlength, double f,
                                double amp) {
    double ph = f * wlength;
    sinwave s(nb_sample, f, amp, ph);
    return s;
  }
};
}
