#pragma once
// implements spectrum type from pbrt-book
#include <utils.hpp>

enum class SpectrumType { Reflectance, Illuminant };

template <int NbSpecSamples> class CoeffSpectrum {
  //
protected:
  double coeffs[NbSpecSamples];

public:
  static const int nbSamples = NbSpecSamples;

  // constructors
  CoeffSpectrum(double c = 0.0) {
    for (int i = 0; i < nbSamples; i++) {
      coeffs[i] = c;
    }
    D_CHECK(!has_nans());
  }
  CoeffSpectrum(const CoeffSpectrum &s) {
    D_CHECK(!s.has_nans());
    for (int i = 0; i < nbSamples; ++i)
      coeffs[i] = s.coeffs[i];
  }
  CoeffSpectrum &operator=(const CoeffSpectrum &s) {
    D_CHECK(!s.has_nans());
    for (int i = 0; i < nbSamples; ++i)
      coeffs[i] = s.coeffs[i];
    return *this;
  }
  bool has_nans() const {
    for (int i = 0; i < nbSamples; i++) {
      if (std::isnan(coeffs[i]))
        return true;
    }
    return false;
  }
  bool is_black() const {
    for (int i = 0; i < nbSamples; i++) {
      if (coeffs[i] != 0.0) {
        return false;
      }
    }
    return true;
  }
  // implementing operators
  CoeffSpectrum &operator+=(const CoeffSpectrum &cs) {
    D_CHECK(!cs.has_nans());
    for (int i = 0; i < nbSamples; i++)
      coeffs[i] += cs.coeffs[i];
    return *this;
  }
  CoeffSpectrum operator+(const CoeffSpectrum &cs) const {
    D_CHECK(!cs.has_nans());
    CoeffSpectrum cspec = *this;
    for (int i = 0; i < nbSamples; i++)
      cspec.coeffs[i] += cs.coeffs[i];
    return cspec;
  }
  CoeffSpectrum operator-(const CoeffSpectrum &cs) const {
    D_CHECK(!cs.has_nans());
    CoeffSpectrum cspec = *this;
    for (int i = 0; i < nbSamples; i++)
      cspec.coeffs[i] -= cs.coeffs[i];
    return cspec;
  }
  CoeffSpectrum &operator-=(const CoeffSpectrum &cs) {
    D_CHECK(!cs.has_nans());
    for (int i = 0; i < nbSamples; i++)
      coeffs[i] -= cs.coeffs[i];
    return *this;
  }
  CoeffSpectrum operator/(const CoeffSpectrum &cs) const {
    D_CHECK(!cs.has_nans());
    CoeffSpectrum ret = *this;
    for (int i = 0; i < nbSamples; i++) {
      D_CHECK(cs.coeffs[i] != 0.0);
      ret.coeffs[i] /= cs.coeffs[i];
    }
    return ret;
  }
  CoeffSpectrum operator/(double d) const {
    D_CHECK(d != 0.0);
    D_CHECK(!std::isnan(d));
    CoeffSpectrum ret = *this;
    for (int i = 0; i < nbSamples; i++) {
      ret.coeffs[i] /= d;
    }
    return ret;
  }
  CoeffSpectrum &operator/=(double d) {
    D_CHECK(d != 0.0);
    D_CHECK(!std::isnan(d));
    for (int i = 0; i < nbSamples; i++) {
      coeffs[i] /= d;
    }
    return *this;
  }
  CoeffSpectrum operator*(const CoeffSpectrum &cs) {
    D_CHECK(!cs.has_nans());
    CoeffSpectrum ret = *this;
    for (int i = 0; i < nbSamples; i++) {
      ret.coeffs[i] *= cs.coeffs[i];
    }
    D_CHECK(!ret.has_nans());
    return ret;
  }
  CoeffSpectrum &operator*=(const CoeffSpectrum &cs) {
    D_CHECK(!cs.has_nans());
    for (int i = 0; i < nbSamples; i++) {
      coeffs[i] *= cs.coeffs[i];
    }
    D_CHECK(!has_nans());
    return *this;
  }
  CoeffSpectrum operator*(double d) {
    CoeffSpectrum ret = *this;
    for (int i = 0; i < nbSamples; i++) {
      ret.coeffs[i] *= d;
    }
    D_CHECK(!ret.has_nans());
    return ret;
  }
  CoeffSpectrum &operator*=(double d) {
    for (int i = 0; i < nbSamples; i++) {
      coeffs[i] *= d;
    }
    D_CHECK(has_nans());
    return *this;
  }
  double &operator[](int i) {
    D_CHECK(i >= 0 && i < nbSamples);
    return coeffs[i];
  }
  double operator[](int i) const {
    D_CHECK(i >= 0 && i < nbSamples);
    return coeffs[i];
  }
  bool operator==(const CoeffSpectrum &cs) const {
    D_CHECK(!cs.has_nans());
    for (int i = 0; i < nbSamples; i++) {
      if (cs.coeffs[i] != coeffs[i])
        return false;
    }
    return true;
  }
  bool operator!=(const CoeffSpectrum &cs) const {
    return !(*this == cs);
  }
  double max() const {
    auto c = coeffs[0];
    for (int i = 1; i < nbSamples; i++) {
      c = fmax(c, coeffs[i]);
    }
    return c;
  }
  double min() const {
    auto c = coeffs[0];
    for (int i = 1; i < nbSamples; i++) {
      c = fmin(c, coeffs[i]);
    }
    return c;
  }
  CoeffSpectrum clamp(double low = 0.0,
                      double high = FLT_MAX) {
    CoeffSpectrum cs;
    for (int i = 0; i < nbSamples; i++) {
      cs.coeffs[i] = clamp(coeffs[i], low, high);
    }
    D_CHECK(!cs.has_nans());
    return cs;
  }
  CoeffSpectrum sqrt(const CoeffSpectrum &cs) {
    D_CHECK(!cs.has_nans());
    CoeffSpectrum c;
    for (int i = 0; i < nbSamples; i++) {
      c.coeffs[i] = sqrt(cs.coeffs[i]);
    }
    D_CHECK(!c.has_nans());
    return c;
  }
};
