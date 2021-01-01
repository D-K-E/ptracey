#pragma once
#include <common.hpp>

namespace ptracey {
/**
  \brief linear interpolation of one range to another

  \param input_start start of the input range
  \param input_end end of the input range
  \param output_start start of the input range
  \param output_end end of the input range
  \param t value whose is going to be interpolated

  \return value in output range
 */
template <typename T>
T interp(T t, T input_start, T input_end, T output_start,
         T output_end) {
  return (t - input_start) / (input_end - input_start) *
             (output_end - output_start) +
         output_start;
}
template <typename T>
std::vector<T> interp(const std::vector<T> &vs,
                      T input_start, T input_end,
                      T output_start, T output_end) {
  std::vector<T> dst;
  dst.resiz(vs.size());
  for (std::size_t i = 0; i < vs.size(); i++) {
    dst[i] = interp(vs[i], input_start, input_end,
                    output_start, output_end);
  }
  return dst;
}
template <typename T>
std::vector<T> interp(const std::vector<T> &vs,
                      T output_start, T output_end) {
  T input_start = *std::min_element(vs.begin(), vs.end());
  T input_end = *std::max_element(vs.begin(), vs.end());
  return interp(vs, input_start, input_end, output_start,
                output_end);
}

template <typename T> T interp(T t, T s1, T s2) {
  // interpolate from glsl reference
  // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/mix.xhtml
  return interp(t, 0.0, 1.0, s1, s2);
}
/**
  \brief linear interpolation between two values

  \param v1 start of interpolation range
  \param v2 end of interpolation range
  \param t value whose is going to be interpolated
 */
template <typename T, typename U, typename V>
T mix(T t, U v1, V v2) {
  return (1 - t) * v1 + t * v2;
}

/**
  \brief thread safe random number generator with a seed

  \param uint seed

  \return Real
 */
Real random_real(uint seed) {
  static std::uniform_real_distribution<Real> distribution(
      0.0f, 1.0f);
  static std::mt19937 generator(seed);
  return distribution(generator);
}
/**
  \brief thread safe random number generator whose seed is
  calculated from chrono library.

  \param uint seed

  \return Real

 */
Real random_real() {
  auto now1 = std::chrono::high_resolution_clock::now();
  auto now2 = std::chrono::high_resolution_clock::now();
  auto d1 = now2 - now1;
  return random_real(static_cast<unsigned int>(d1.count()));
}
/**
  \brief real random number between given range

  \param mn start of range
  \param mx end of range

  \return random value in given range
 */
Real random_real(Real mn, Real mx) {
  return interp<Real>(random_real(), mn, mx);
}
/**
  \brief thread safe random integer between 0 1
 */
int random_int() { return static_cast<int>(random_real()); }
/**
  \brief thread safe random integer between given range
 */
int random_int(int mn, int mx) {
  return static_cast<int>(random_real(mn, mx));
}
/**
  \brief transform degree to radian
 */
inline Real degrees_to_radians(Real degrees) {
  return degrees * M_PI / 180.0;
}
/**
  \brief transform radian to degrees
 */
inline Real radians_to_degrees(Real radian) {
  return radian * 180.0 / M_PI;
}

/**
  \brief clamp value to given range
  \param x value to be clamped
  \param min minimum value of the range
  \param max maximum value of the range

  \return either x, min or the max
 */
template <typename T, typename U, typename V>
T clamp(T x, U min, V max) {
  if (x < min)
    return min;
  if (x > max)
    return max;
  return x;
}

/**
  \brief clamp value to given range
  \param x value to be clamped
  \param min minimum value of the range
  \param max maximum value of the range

  \return either x, min or the max all in the same type
 */
template <typename T> T dclamp(T x, T mn, T mx) {
  return clamp<T, T, T>(x, mn, mx);
}

/**
  \brief even spaced vector between given range

  \param start start of the range
  \param end end of the range
  \param size of the resulting vector

  \return lspaced evenly spaced vector
 */
template <typename T>
std::vector<T> linspace(T start, T end, uint size) {
  // from: https://stackoverflow.com/a/27030598/7330813
  std::vector<T> lspaced;
  if (size == 0)
    return lspaced;
  if (size == 1) {
    lspaced.push_back(start);
    return lspaced;
  }
  auto delta = (end - start) / (size - 1);
  for (unsigned int i = 0; i < size - 1; i++) {
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
  do {                                                     \
    bool res = call;                                       \
    if (!res) {                                            \
      std::stringstream txt;                               \
      txt << #call << " :: "                               \
          << "false"                                       \
          << " :: " << __FILE__ << " :: " << __LINE__      \
          << std::endl;                                    \
      throw std::runtime_error(txt.str());                 \
    }                                                      \
  } while (0)

#define D_CHECK_MSG(call, msg)                             \
  do {                                                     \
    bool res = call;                                       \
    if (!res) {                                            \
      std::stringstream txt;                               \
      txt << #call << " :: "                               \
          << "false"                                       \
          << " :: " << __FILE__ << " :: " << __LINE__      \
          << std::endl                                     \
          << msg << std::endl;                             \
      throw std::runtime_error(txt.str());                 \
    }                                                      \
  } while (0)

#define COMP_CHECK(call, el1, el2)                         \
  do {                                                     \
    bool res = call;                                       \
    auto el1v = el1;                                       \
    auto el2v = el2;                                       \
    if (!res) {                                            \
      std::stringstream txt;                               \
      txt << #call << " values: " << std::endl             \
          << #el1 << ": " << el1v << std::endl             \
          << #el2 << ": " << el2v << std::endl             \
          << " :: "                                        \
          << "false"                                       \
          << " :: " << __FILE__ << " :: " << __LINE__      \
          << std::endl;                                    \
      throw std::runtime_error(txt.str());                 \
    }                                                      \
  } while (0)
#define COMP_CHECK_MSG(call, el1, el2, msg)                \
  do {                                                     \
    bool res = call;                                       \
    auto el1v = el1;                                       \
    auto el2v = el2;                                       \
    if (!res) {                                            \
      std::stringstream txt;                               \
      txt << #call << " values: " << std::endl             \
          << #el1 << ": " << el1v << std::endl             \
          << #el2 << ": " << el2v << std::endl             \
          << " :: "                                        \
          << "false"                                       \
          << " :: " << __FILE__ << " :: " << __LINE__      \
          << " :: " << msg << std::endl;                   \
      throw std::runtime_error(txt.str());                 \
    }                                                      \
  } while (0)

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

/**
  \brief print vector values to terminal

  \param out output stream normally terminal
  \param ss vector whose elements are going to be printed

  \return out output stream
 */
template <typename T>
inline std::ostream &operator<<(std::ostream &out,
                                const std::vector<T> &ss) {
  std::stringstream s;
  for (auto r : ss) {
    s << "," << std::to_string(r);
  }
  auto s_str = s.str();
  out << s_str << std::endl;
  return out;
}
/**
  \brief cast vector values to DestType type

  \param vs vector whose values are going to be cast
  \param fn cast, transform function

  \return ds vector in DestType
 */
template <typename SrcType, typename DestType>
inline std::vector<DestType>
cast_vec(const std::vector<SrcType> &vs,
         const std::function<DestType(SrcType)> &fn) {
  std::vector<DestType> ds;
  ds.resize(vs.size());
  for (std::size_t i = 0; i < vs.size(); i++) {
    ds[i] = fn(vs[i]);
  }
  return ds;
}

/**
  \brief cast vector values to DestType type

  \param vs vector whose values are going to be cast
  \param fn cast, transform function

  \return vector in DestType
 */
template <typename SrcType, typename DestType>
inline std::vector<DestType>
cast_vec(const std::vector<SrcType> &vs) {
  return cast_vec<SrcType, DestType>(
      vs, [](auto i) { return static_cast<DestType>(i); });
}
}
