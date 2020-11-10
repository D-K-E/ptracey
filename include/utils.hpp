#pragma once
#include <cmath>
#include <external.hpp>
#include <sstream>
using std::shared_ptr;
using std::make_shared;
using std::sqrt;
using std::make_pair;
using std::isnan;

double random_double() {
  static std::uniform_real_distribution<double>
      distribution(0.0f, 1.0f);
  static std::mt19937 generator;
  return distribution(generator);
}

double random_double(double mn, double mx) {
  static std::uniform_real_distribution<double>
      distribution(mn, mx);
  static std::mt19937 generator;
  return distribution(generator);
}
int random_int() {
  return static_cast<int>(random_double());
}
int random_int(int mn, int mx) {
  return static_cast<int>(random_double(mn, mx));
}
inline double degrees_to_radians(double degrees) {
  return degrees * M_PI / 180.0;
}

template <typename Real>
Real clamp(Real x, Real min, Real max) {
  if (x < min)
    return min;
  if (x > max)
    return max;
  return x;
}
template <typename T> T dclamp(T x, T mn, T mx) {
  return clamp<T>(x, mn, mx);
}
inline double interp(double t, double s1, double s2) {
  // interpolate
  return (1 - t) * s1 + t * s2;
}

#define D_CHECK(call)                                      \
  {                                                        \
    bool res = call;                                       \
    if (!res) {                                            \
      std::stringstream txt;                               \
      txt << #call << " :: " << res << " :: " << __FILE__  \
          << " :: " << __LINE__ << std::endl;              \
      throw std::runtime_error(txt.str());                 \
    }                                                      \
  }

using fn = std::function<bool(int)>;
int findInterval(int size, const fn &f) {
  // taken from
  // https://github.com/mmp/pbrt-v3/blob/307d1620bf75771482f5dfd1dede1da0d33b5ee2/src/core/pbrt.h
  int first = 0, len = size;
  while (len > 0) {
    int half = len >> 1, middle = first + half;
    //
    if (f(middle)) {
      //
      first = middle + 1;
      len -= half + 1;
    } else {
      len = half;
    }
  }
  return clamp<int>(first - 1, 0, size - 2);
}
