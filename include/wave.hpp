#pragma once

#include <common.hpp>
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
    std::vector<T> vs;
    vs.resize(values.size());
    for (unsigned int i = 0; i < values.size(); i++) {
      vs[i] = values[i] / s;
    }
    auto ss = sampled_wave(vs);
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
  sampled_wave interpolate(Real low = 0.0,
                           Real high = FLT_MAX) {
    std::vector<T> csvec;
    csvec.resize(values.size());
    T start = min();
    T end = max();
    for (int i = 0; i < values.size(); i++) {
      csvec[i] =
          interp<T>(values[i], start, end, low, high);
    }
    sampled_wave cs = sampled_wave(csvec);
    D_CHECK(!cs.has_nans());
    return cs;
  }
};
