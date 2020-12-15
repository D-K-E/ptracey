#pragma once

#include <common.hpp>
#include <utils.hpp>
using namespace ptracey;
namespace ptracey {

/** \brief a wave sample representing a continuous wave in
   discreet form

 */
template <class T> class sampled_wave {
public:
  /**
    \brief holds observed power values of the wave
   */
  std::vector<T> values;

  /**\brief a constant value constructor

    We give the number of samples and a constant value in
   order to
    initialize the sampled wave with a single value like 0,
   or 420

   \param size determines the number of samples of the wave
   \param val the constant power value.
   */
  sampled_wave(T val, unsigned int size) {
    values.clear();
    for (unsigned int i = 0; i < size; i++) {
      values.push_back(val);
    }
    D_CHECK(!has_nans());
  }
  /**
    \brief directly construct from given values.

    We construct the wave directly from samples. We only
    check for nan values.

    \param vals a vector of values
   */
  sampled_wave(const std::vector<T> &vals) : values(vals) {
    D_CHECK(!has_nans());
  }
  /**
    \brief gives the maximum value among sampled points
   */
  T max() const {
    return *std::max_element(values.begin(), values.end());
  }
  /**
      \brief gives the minimum value among sampled points
     */

  T min() const {
    return *std::min_element(values.begin(), values.end());
  }
  /**
    \brief check if sampled power values contain nan value.
   */
  bool has_nans() const {
    for (auto val : values) {
      if (isnan(val))
        return true;
    }
    return false;
  }
  /**
      \brief check if sampled power values contain 0
     value.
  */
  bool has_zeros() const {
    for (auto val : values) {
      if (val == 0.0)
        return true;
    }
    return false;
  }
  /**
        \brief check if sampled power values contain only 0
       value.
    */
  bool is_black() const {
    for (auto val : values) {
      if (val != 0.0)
        return false;
    }
    return true;
  }
  /**
    \brief assign a sample wave to this wave if it does not
    contain nan values
   */
  sampled_wave &operator=(const sampled_wave &s) {
    D_CHECK(!s.has_nans());
    values = s.values;
    return *this;
  }
  /**
    \brief apply a function to a single power value.

    We check whether the operation produces a negative value
as well.

    \param index index of the power value among values
vector
    \param fn function to be applied to the power value at
index.
    \return a new wave where the desired element is changed.

TODO \todo This operation is probably very costly for large
samples.
   */
  sampled_wave
  apply(unsigned int index, T value,
        const std::function<T(T, T)> &fn) const {
    std::vector<T> vs;
    vs.resize(values.size());
    std::copy(values.begin(), values.end(), vs.begin());
    auto val = fn(values[index], value);
    COMP_CHECK_MSG(val < 0.0, val, 0.0,
                   "operation produces negative value");
    vs[index] = val;
    auto wave = sampled_wave(values);
    return wave;
  }
  /**
      \brief apply a function to all power values.

      \param fn function to be applied to power values
      \return a new wave

TODO: \todo for basic arithmetic operations it might be
enough to check for
minimum and maximum power values for the given function.
   */
  sampled_wave
  apply(T value, const std::function<T(T, T)> &fn) const {
    std::vector<T> vs;
    vs.resize(values.size());
    for (std::size_t i = 0; i < values.size(); i++) {
      auto val = fn(values[i], value);
      COMP_CHECK_MSG(val < 0.0, val, 0.0,
                     "operation produces a negative value");
      vs[i] = val;
    }
    auto wave = sampled_wave(vs);
    return wave;
  }
  /**
    \brief apply a function to same sized wave samples

    \param fn function to be applied to power values
    \return a new wave
  */
  sampled_wave
  apply(const sampled_wave &s,
        const std::function<T(T, T)> &fn) const {
    D_CHECK(!s.has_nans());
    D_CHECK(s.values.size() == values.size());
    std::vector<T> vs;
    vs.resize(values.size());

    for (std::size_t i = 0; i < values.size(); i++) {
      auto val = fn(values[i], s.values[i]);
      COMP_CHECK_MSG(val < 0.0, val, 0.0,
                     "operation produces a negative value");
      vs[i] = val;
    }
    auto wave = sampled_wave(vs);
    return wave;
  }

  /**
    \brief add and assign values of given sampled wave

TODO \todo this pointer changes value after this operation.
This
is probably not
the intention of the user if she is using += operator.
   */
  sampled_wave &operator+=(const sampled_wave &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i + j; });
    return wave;
  }
  /**
      \brief add and the given power value to this wave.

  TODO \todo this pointer changes value after this
  operation.
  This is probably not
  the intention of the user if she is using += operator.
     */
  sampled_wave &operator+=(const T &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i + j; });
    return wave;
  }
  /**
      \brief add two same sized sampled waves
  */
  sampled_wave operator+(const sampled_wave &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i + j; });
    return wave;
  }
  /**
      \brief add given power value to all of the elements of
     the sampled wave.
  */
  sampled_wave operator+(const T &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i + j; });
    return wave;
  }
  /**
    \see sampled_wave operator+(const T &s) const
   */
  friend sampled_wave operator+(const T &s,
                                const sampled_wave &ss) {
    return ss + s;
  }

  /**
    \brief subtract and assign values of given sampled wave

TODO \todo this pointer changes value after this operation.
This is probably not the intention of the user if she is
using -= operator.
   */
  sampled_wave &operator-=(const sampled_wave &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i - j; });
    return wave;
  }
  /**
      \brief subtract and the given power value to this
  wave.

  TODO \todo this pointer changes value after this
  operation.  This is
  probably not the intention of the user if she is using -=
  operator.
     */
  sampled_wave &operator-=(const T &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i - j; });
    return wave;
  }
  /**
      \brief subtract two same sized sampled waves
  */
  sampled_wave operator-(const sampled_wave &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i - j; });
    return wave;
  }
  /**
      \brief subtract power value from this wave

      We check whether the new value is producing negative
     power values.
  */
  sampled_wave operator-(const T &s) const {
    auto minval = min();
    COMP_CHECK_MSG(
        minval <= s, minval, s,
        "given value would produce negative powers");
    auto wave =
        apply(s, [](auto this_wave_value, auto value) {
          return this_wave_value - value;
        });
    return wave;
  }
  /** \brief subtract every element of given sampled_wave
   * from the value
   */
  friend sampled_wave operator-(const T &s,
                                const sampled_wave &ss) {
    auto maxval = ss.max();
    COMP_CHECK_MSG(
        maxval <= s, maxval, s,
        "given value would produce negative powers");
    auto wave =
        ss.apply(s, [](auto this_wave_value, auto value) {
          return value - this_wave_value;
        });
    return wave;
  }
  /**
      \brief multiply and assign values of given sampled
  wave

  TODO \todo this pointer changes value after this
  operation.
  This
  is probably not
  the intention of the user if she is using *= operator.
     */

  sampled_wave &operator*=(const sampled_wave &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i * j; });
    return wave;
  }
  /**
      \brief multiply and the given power value to this
  wave.

  TODO \todo this pointer changes value after this
  operation.
  This is probably not
  the intention of the user if she is using *= operator.
     */
  sampled_wave &operator*=(const T &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i * j; });
    return wave;
  }

  /**
      \brief multiply two same sized sampled waves
  */
  sampled_wave operator*(const sampled_wave &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i * j; });
    return wave;
  }
  /**
    \brief multiply given power value to all of the elements
    of the sampled wave.
  */
  sampled_wave operator*(const T &s) const {
    auto wave =
        apply(s, [](auto i, auto j) { return i * j; });
    return wave;
  }
  /**
    \see sampled_wave operator*(const T &s) const
  */
  friend sampled_wave operator*(const T &s,
                                const sampled_wave &ss) {
    return ss * s;
  }
  /**
    \brief divide and assign values of given sampled wave

TODO \todo this pointer changes value after this operation.
This is probably not the intention of the user if she is
using /= operator.
   */
  sampled_wave &operator/=(const sampled_wave &s) {
    D_CHECK(!s.has_zeros());
    auto wave =
        apply(s, [](auto i, auto j) { return i / j; });
    return wave;
  }
  /**
      \brief divide and the given power value to this
  wave.

  TODO \todo this pointer changes value after this
  operation. This is probably not the intention of the user
  if she is using /= operator.
  */
  sampled_wave &operator/=(const T &s) const {
    D_CHECK(s != 0.0);
    auto wave =
        apply(s, [](auto i, auto j) { return i / j; });
    return wave;
  }
  /**
    \brief divide two same sized sampled waves
  */

  sampled_wave operator/(const sampled_wave &s) const {
    D_CHECK(!s.has_zeros());
    auto wave =
        apply(s, [](auto i, auto j) { return i / j; });
    return wave;
  }
  /**
      \brief divide power value from this wave

      \param s value to divide
      We check if s is 0 then perform the division.
  */
  sampled_wave operator/(const T &s) const {
    D_CHECK(s != 0.0);
    auto wave =
        apply(s, [](auto i, auto j) { return i / j; });
    return wave;
  }
  /**
    \brief divide power value from this wave

    \param s value to divide
    \param ss wave to divide.

    We check if s is 0, then divide 1 by s. Then we perform
    a mutliplication
    of the resulting value with the ss.
  */

  friend sampled_wave operator/(const T &s,
                                const sampled_wave &ss) {
    D_CHECK(s != 0.0);
    auto sval = 1.0 / s;
    auto wave = ss * sval;
    return wave;
  }
  /**
    \brief access to power value at given index.
    Mostly used for setting a new value for the given index;
    \return a reference
   */
  T &operator[](uint i) {
    D_CHECK(i < (uint)values.size());
    return values[i];
  }
  /**
    \brief access to power value at given index.
    Mostly used for reading the value of the given index;
    \return a value
   */
  T operator[](uint i) const {
    D_CHECK((uint)i < values.size());
    return values[i];
  }
  /**
    \brief check if given sampled wave has same power with
    this one.

    \param cs comparison wave
  */
  bool operator==(const sampled_wave &cs) const {
    D_CHECK(!cs.has_nans());
    for (std::size_t i = 0; i < values.size(); i++) {
      if (cs.values[i] != values[i])
        return false;
    }
    return true;
  }
  /** \brief check if given sampled_wave is not equal to
   * this or not*/
  bool operator!=(const sampled_wave &cs) const {
    return !(*this == cs);
  }
  /** \brief reduce power values of this wave to a single
   * value using given function
     \param fn function that is applied during reduction
     \param counter_value initial value assigned to
   aggregating variable
   * */
  T reduce(const std::function<T(T, T)> &fn,
           T counter_value) const {
    T s = counter_value;
    for (auto v : values)
      s = fn(s, v);
    return s;
  }
  /** \brief reduce sampled wave by summation*/
  T sum() const {
    return reduce(
        [](T counter_value, T element) {
          return counter_value + element;
        },
        0.0);
  }
  /** \brief reduce sampled wave by multiplication*/
  T product() const {
    return reduce(
        [](T counter_value, T element) {
          return counter_value * element;
        },
        1.0);
  }
  /** \brief find the average value of this wave*/
  Real average() const {
    return sum() / (int)values.size();
  }
  /** \brief find the number of samples inside this wave*/
  uint size() const { return (uint)values.size(); }

  /** \brief clamp power values of this wave to given range
    We also check if the resulting wave has nan values.

    \param low low end of the range
    \param high high end of the range
    \return sampled_wave with values between the range [low,
    high]
   */
  sampled_wave clamp(T low = 0.0, T high = FLT_MAX) {
    sampled_wave cs = *this;
    for (int i = 0; i < values.size(); i++) {
      cs.values[i] = dclamp<T>(values[i], low, high);
    }
    D_CHECK(!cs.has_nans());
    return cs;
  }
  /**
    \brief interpolate values of this wave to given range
    \see clamp(), \fn interp<T>()

    \param low low end of range
    \param high high end of range
   */
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

/** \brief serialize sampled_wave to output stream*/
template <typename T>
inline std::ostream &operator<<(std::ostream &out,
                                const sampled_wave<T> &ss) {
  std::stringstream s;
  for (auto r : ss.values) {
    s << "," << std::to_string(r);
  }
  auto s_str = s.str();
  out << s_str << std::endl;
  return out;
}
}
