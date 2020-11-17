#pragma once
#include <common.hpp>

namespace ptracey {
Real random_real(unsigned int seed) {
  static std::uniform_real_distribution<Real> distribution(
      0.0f, 1.0f);
  static std::mt19937 generator(seed);
  return distribution(generator);
}
Real random_real() {
  auto now1 = std::chrono::high_resolution_clock::now();
  auto now2 = std::chrono::high_resolution_clock::now();
  auto d1 = now2 - now1;
  return random_real(static_cast<unsigned int>(d1.count()));
}
Real random_real(Real mn, Real mx) {
  return mn + (mx - mn) * random_real();
}
int random_int() { return static_cast<int>(random_real()); }
int random_int(int mn, int mx) {
  return static_cast<int>(random_real(mn, mx));
}
inline Real degrees_to_radians(Real degrees) {
  return degrees * M_PI / 180.0;
}

template <typename T> T clamp(T x, T min, T max) {
  if (x < min)
    return min;
  if (x > max)
    return max;
  return x;
}
template <typename T> T dclamp(T x, T mn, T mx) {
  return clamp<T>(x, mn, mx);
}

template <typename T>
T interp(T t, T input_start, T input_end, T output_start,
         T output_end) {
  return (t - input_start) / (input_end - input_start) *
             (output_end - output_start) +
         output_start;
}
template <typename T> T interp(T t, T s1, T s2) {
  // interpolate from glsl reference
  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/mix.xhtml
  return interp(t, 0.0, 1.0, s1, s2);
}

template <typename T>
std::vector<T> linspace(T start, T end, unsigned int size) {
  // from: https://stackoverflow.com/a/27030598/7330813
  std::vector<T> lspaced;
  if (size == 0)
    return lspaced;
  if (size == 1) {
    lspaced.push_back(start);
    return lspaced;
  }
  auto delta = (end - start) / (size - 1);
  for (int i = 0; i < size - 1; i++) {
    lspaced.push_back(start + delta * i);
  }
  lspaced.push_back(end);
  return lspaced;
}

// ucs related functions
// from McCluney (Ross), Introduction to radiometry and
// photometry, Boston, 1994. QC795.42 .M33 1994. ISBN :
// 978-0-89006-678-2.
template <typename T> Real get_uprim(T x, T y) {
  return (4.0 * x) / (-2.0 * x + 12.0 * y + 3.0);
}
template <typename T> Real get_vprim(T x, T y) {
  return (9.0 * y) / (-2.0 * x + 12.0 * y + 3.0);
}
template <typename T> Real get_wprim(T uprim, T vprim) {
  return 1.0 - uprim - vprim;
}
template <typename T>
void get_uvwprims(T x, T y, T &uprim, T &vprim, T &wprim) {
  uprim = get_uprim(x, y);
  vprim = get_vprim(x, y);
  wprim = get_wprim(uprim, vprim);
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
}
