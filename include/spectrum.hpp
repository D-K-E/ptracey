#pragma once
// implements spectrum type from pbrt-book
// mostly adapted from
// https://github.com/mmp/pbrt-v3/blob/master/src/core/spectrum.h
#include <specdata.hpp>
#include <utils.hpp>
#include <vec3.hpp>
// CONSTANTS

static const int NB_SPECTRAL_SAMPLES = 60;
static const int VISIBLE_LAMBDA_START = 400;
static const int VISIBLE_LAMBDA_END = 700;
static const auto CIE_Y_INTEGRAL = 106.856895;

// utility functions for spectrums
bool isSamplesSorted(const double *lambdas,
                     const double *vs, int nb_samples) {
  for (int i = 0; i < nb_samples - 1; i++) {
    if (lambdas[i] > lambdas[i + 1])
      return false;
  }
  return true;
}
void sortSpectrumSamples(double *lambdas, double *vs,
                         int nb_samples) {
  std::vector<std::pair<double, double>> sortedVec;
  sortedVec.reserve(nb_samples);

  for (int i = 0; i < nb_samples; i++) {
    sortedVec.push_back(make_pair(lambdas[i], vs[i]));
  }
  std::sort(sortedVec.begin(), sortedVec.end());
  for (int i = 0; i < nb_samples; i++) {
    lambdas[i] = sortedVec[i].first;
    vs[i] = sortedVec[i].second;
  }
}

double averageSpectrumSamples(const double *lambdas,
                              const double *vs,
                              int nb_sample,
                              double lambdaStart,
                              double lambdaEnd) {
  D_CHECK(isSamplesSorted(lambdas, vs, nb_sample));
  D_CHECK(lambdaStart < lambdaEnd);
  //
  if (lambdas[0] >= lambdaEnd) {
    return vs[0];
  }
  if (lambdas[nb_sample - 1] <= lambdaStart) {
    return vs[nb_sample - 1];
  }
  if (nb_sample == 1) {
    return vs[0];
  }
  //
  double sum = 0.0;
  // constant segment contribution
  if (lambdaStart < lambdas[0]) {
    sum += vs[0] * (lambdas[0] - lambdaStart);
  }
  if (lambdaEnd > lambdas[nb_sample - 1]) {
    sum += vs[nb_sample] * (lambdas[nb_sample] - lambdaEnd);
  }
  int start_index = 0;
  while (lambdaStart > lambdas[start_index + 1])
    start_index++;

  D_CHECK(start_index + 1 < nb_sample);

  auto interp_l = [lambdas, vs](double w, int i) {
    return interp(
        w - lambdas[i] / (lambdas[i + 1] - lambdas[i]),
        vs[i], vs[i + 1]);
  };
  //
  //
  while (start_index + 1 < nb_sample &&
         lambdaEnd > lambdas[start_index]) {
    start_index++;
    double segStart =
        std::max(lambdaStart, lambdas[start_index]);
    double segEnd =
        std::min(lambdaEnd, lambdas[start_index + 1]);

    sum += 0.5 * (interp_l(segStart, start_index) +
                  interp_l(segEnd, start_index)) *
           (segEnd - segStart);
  }
  return sum / (lambdaEnd - lambdaStart);
}

double interpSpecSamples(const double *lambdas,
                         const double *vs, int nb_sample,
                         double lam) {
  D_CHECK(isSamplesSorted(lambdas, vs, nb_sample));
  if (lambdas[0] >= lam) {
    return vs[0];
  }
  if (lambdas[nb_sample - 1] <= lam) {
    return vs[nb_sample - 1];
  }
  int offset = findInterval(nb_sample, [&](int index) {
    return lambdas[index] <= 1;
  });
  D_CHECK(lam >= lambdas[offset] &&
          lam <= lambdas[offset + 1]);
  double lamVal = (lam - lambdas[offset]) /
                  (lambdas[offset + 1] - lambdas[offset]);
  return interp(lamVal, vs[offset], vs[offset + 1]);
}

inline void xyz2rgb(const double xyz[3], double rgb[3]) {
  rgb[0] = 3.240479 * xyz[0] - 1.537150 * xyz[1] -
           0.498535 * xyz[2];
  rgb[1] = -0.969256 * xyz[0] + 1.875991 * xyz[1] +
           0.041556 * xyz[2];
  rgb[2] = 0.055648 * xyz[0] - 0.204043 * xyz[1] +
           1.057311 * xyz[2];
}
inline void rgb2xyz(const double rgb[3], double xyz[3]) {
  xyz[0] = 0.412453 * rgb[0] + 0.357580 * rgb[1] +
           0.180423 * rgb[2];
  xyz[1] = 0.212671 * rgb[0] + 0.715160 * rgb[1] +
           0.072169 * rgb[2];
  xyz[2] = 0.019334 * rgb[0] + 0.119193 * rgb[1] +
           0.950227f * rgb[2];
}

enum class SpectrumType { Reflectance, Illuminant };
enum RGB_AXIS : int { RGB_R, RGB_G, RGB_B };

template <int NbSpecSamples> class coeff_spectrum {
  //
protected:
  double coeffs[NbSpecSamples];

public:
  static const int nbSamples = NbSpecSamples;

  // constructors
  coeff_spectrum(double c = 0.0) {
    for (int i = 0; i < nbSamples; i++) {
      coeffs[i] = c;
    }
    D_CHECK(!has_nans());
  }
  coeff_spectrum(const coeff_spectrum &s) {
    D_CHECK(!s.has_nans());
    for (int i = 0; i < nbSamples; ++i)
      coeffs[i] = s.coeffs[i];
  }
  coeff_spectrum &operator=(const coeff_spectrum &s) {
    D_CHECK(!s.has_nans());
    for (int i = 0; i < nbSamples; ++i)
      coeffs[i] = s.coeffs[i];
    return *this;
  }
  bool has_nans() const {
    for (int i = 0; i < nbSamples; i++) {
      if (isnan(coeffs[i]))
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
  coeff_spectrum &operator+=(const coeff_spectrum &cs) {
    D_CHECK(!cs.has_nans());
    for (int i = 0; i < nbSamples; i++)
      coeffs[i] += cs.coeffs[i];
    return *this;
  }
  coeff_spectrum operator+(const coeff_spectrum &cs) const {
    D_CHECK(!cs.has_nans());
    coeff_spectrum cspec = *this;
    for (int i = 0; i < nbSamples; i++)
      cspec.coeffs[i] += cs.coeffs[i];
    return cspec;
  }
  coeff_spectrum operator-(const coeff_spectrum &cs) const {
    D_CHECK(!cs.has_nans());
    coeff_spectrum cspec = *this;
    for (int i = 0; i < nbSamples; i++)
      cspec.coeffs[i] -= cs.coeffs[i];
    return cspec;
  }
  coeff_spectrum &operator-=(const coeff_spectrum &cs) {
    D_CHECK(!cs.has_nans());
    for (int i = 0; i < nbSamples; i++)
      coeffs[i] -= cs.coeffs[i];
    return *this;
  }
  coeff_spectrum operator/(const coeff_spectrum &cs) const {
    D_CHECK(!cs.has_nans());
    coeff_spectrum ret = *this;
    for (int i = 0; i < nbSamples; i++) {
      D_CHECK(cs.coeffs[i] != 0.0);
      ret.coeffs[i] /= cs.coeffs[i];
    }
    return ret;
  }
  coeff_spectrum operator/(double d) const {
    D_CHECK(d != 0.0);
    D_CHECK(!isnan(d));
    coeff_spectrum ret = *this;
    for (int i = 0; i < nbSamples; i++) {
      ret.coeffs[i] /= d;
    }
    return ret;
  }
  coeff_spectrum &operator/=(double d) {
    D_CHECK(d != 0.0);
    D_CHECK(!isnan(d));
    for (int i = 0; i < nbSamples; i++) {
      coeffs[i] /= d;
    }
    return *this;
  }
  coeff_spectrum operator*(const coeff_spectrum &cs) const {
    D_CHECK(!cs.has_nans());
    coeff_spectrum ret = *this;
    for (int i = 0; i < nbSamples; i++) {
      ret.coeffs[i] *= cs.coeffs[i];
    }
    D_CHECK(!ret.has_nans());
    return ret;
  }
  coeff_spectrum &operator*=(const coeff_spectrum &cs) {
    D_CHECK(!cs.has_nans());
    for (int i = 0; i < nbSamples; i++) {
      coeffs[i] *= cs.coeffs[i];
    }
    D_CHECK(!has_nans());
    return *this;
  }
  coeff_spectrum operator*(double d) const {
    coeff_spectrum ret = *this;
    for (int i = 0; i < nbSamples; i++) {
      ret.coeffs[i] *= d;
    }
    D_CHECK(!ret.has_nans());
    return ret;
  }
  friend coeff_spectrum operator*(double d,
                                  const coeff_spectrum &c) {
    return c * d;
  }
  coeff_spectrum &operator*=(double d) {
    for (int i = 0; i < nbSamples; i++) {
      coeffs[i] *= d;
    }
    D_CHECK(!has_nans());
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
  bool operator==(const coeff_spectrum &cs) const {
    D_CHECK(!cs.has_nans());
    for (int i = 0; i < nbSamples; i++) {
      if (cs.coeffs[i] != coeffs[i])
        return false;
    }
    return true;
  }
  bool operator!=(const coeff_spectrum &cs) const {
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
  coeff_spectrum clamp(double low = 0.0,
                       double high = FLT_MAX) {
    coeff_spectrum cs;
    for (int i = 0; i < nbSamples; i++) {
      cs.coeffs[i] = dclamp<double>(coeffs[i], low, high);
    }
    D_CHECK(!cs.has_nans());
    return cs;
  }
  coeff_spectrum sqrt(const coeff_spectrum &cs) {
    D_CHECK(!cs.has_nans());
    coeff_spectrum c;
    for (int i = 0; i < nbSamples; i++) {
      c.coeffs[i] = sqrt(cs.coeffs[i]);
    }
    D_CHECK(!c.has_nans());
    return c;
  }
};

class rgb_spectrum : public coeff_spectrum<3> {
  //
  using coeff_spectrum<3>::coeffs;

public:
  rgb_spectrum(double v = 0.0) : coeff_spectrum<3>(v) {}
  rgb_spectrum(const coeff_spectrum<3> &v)
      : coeff_spectrum<3>(v) {}
  rgb_spectrum(
      const rgb_spectrum &v,
      SpectrumType type = SpectrumType::Reflectance) {
    *this = v;
  }
  static rgb_spectrum
  fromRgb(const double rgb[3],
          SpectrumType type = SpectrumType::Reflectance) {
    rgb_spectrum s;
    s.coeffs[0] = rgb[0];
    s.coeffs[1] = rgb[1];
    s.coeffs[2] = rgb[2];
    D_CHECK(!s.has_nans());
    return s;
  }
  static rgb_spectrum
  fromRgb(const color &c,
          SpectrumType type = SpectrumType::Reflectance) {
    const double rgb[3] = {c.x(), c.y(), c.z()};
    return rgb_spectrum::fromRgb(rgb, type);
  }
  void to_rgb(double rgb[3]) const {
    double xyz[3];
    to_xyz(xyz);
    xyz2rgb(xyz, rgb);
  }
  color to_rgb() const {
    double rgb[3];
    to_rgb(rgb);
    return color(rgb);
  }
  const rgb_spectrum &toRgbSpectrum() const {
    return *this;
  }
  void to_xyz(double xyz[3]) const { rgb2xyz(coeffs, xyz); }
  static rgb_spectrum
  fromXyz(const double xyz[3],
          SpectrumType type = SpectrumType::Reflectance) {
    rgb_spectrum r;
    xyz2rgb(xyz, r.coeffs);
    return r;
  }
  double y() const {
    const double yweights[] = {0.212671, 0.715160,
                               0.072169};
    return yweights[0] * coeffs[0] +
           yweights[1] * coeffs[1] +
           yweights[2] * coeffs[2];
  }

  static rgb_spectrum fromSamples(const double *lambdas,
                                  const double *vs,
                                  int nb_sample) {
    if (!isSamplesSorted(lambdas, vs, nb_sample)) {
      std::vector<double> slambda(&lambdas[0],
                                  &lambdas[nb_sample]);
      std::vector<double> svs(&vs[0], &vs[nb_sample]);
      sortSpectrumSamples(slambda.data(), svs.data(),
                          nb_sample);
      return fromSamples(slambda.data(), svs.data(),
                         nb_sample);
    }
    double xyz[3] = {0, 0, 0};
    for (int i = 0; i < NB_CIE_SAMPLES; i++) {
      //
      double v = interpSpecSamples(lambdas, vs, nb_sample,
                                   CIE_LAMBDA[i]);
      xyz[0] = v * CIE_X[i];
      xyz[1] = v * CIE_Y[i];
      xyz[2] = v * CIE_Z[i];
    }

    double scale =
        static_cast<double>(CIE_LAMBDA[NB_CIE_SAMPLES - 1] -
                            CIE_LAMBDA[0]) /
        static_cast<double>(CIE_Y_INTEGRAL *
                            NB_CIE_SAMPLES);
    xyz[0] *= scale;
    xyz[1] *= scale;
    xyz[2] *= scale;
    return fromXyz(xyz);
  }
};

class sampled_spectrum
    : public coeff_spectrum<NB_SPECTRAL_SAMPLES> {
public:
  sampled_spectrum(double v = 0.0) : coeff_spectrum(v) {}
  sampled_spectrum(
      const coeff_spectrum<NB_SPECTRAL_SAMPLES> &v)
      : coeff_spectrum<NB_SPECTRAL_SAMPLES>(v) {}
  static sampled_spectrum fromSamples(const double *lambdas,
                                      const double *vs,
                                      int nb_sample) {
    if (!isSamplesSorted(lambdas, vs, nb_sample)) {
      std::vector<double> slambda(&lambdas[0],
                                  &lambdas[nb_sample]);
      std::vector<double> svs(&vs[0], &vs[nb_sample]);
      sortSpectrumSamples(slambda.data(), svs.data(),
                          nb_sample);
      return fromSamples(slambda.data(), svs.data(),
                         nb_sample);
    }
    sampled_spectrum r;
    auto nb_samp = static_cast<double>(NB_SPECTRAL_SAMPLES);
    for (int i = 0; i < NB_SPECTRAL_SAMPLES; i++) {
      auto lambda0 =
          interp(static_cast<double>(i) / nb_samp,
                 VISIBLE_LAMBDA_START, VISIBLE_LAMBDA_END);
      auto lambda1 =
          interp(static_cast<double>(i + 1) / nb_samp,
                 VISIBLE_LAMBDA_START, VISIBLE_LAMBDA_END);
      r.coeffs[i] = averageSpectrumSamples(
          lambdas, vs, nb_sample, lambda0, lambda1);
    }
    return r;
  }
  void to_xyz(double xyz[3]) const {
    xyz[0] = xyz[1] = xyz[2] = 0.0;
    double scale =
        static_cast<double>(VISIBLE_LAMBDA_END -
                            VISIBLE_LAMBDA_START) /
        (CIE_Y_INTEGRAL * NB_SPECTRAL_SAMPLES);

    for (int i = 0; i < NB_SPECTRAL_SAMPLES; i++) {
      xyz[0] += X.coeffs[i] * coeffs[i];
      xyz[1] += Y.coeffs[i] * coeffs[i];
      xyz[2] += Z.coeffs[i] * coeffs[i];
    }
    xyz[0] *= scale;
    xyz[1] *= scale;
    xyz[2] *= scale;
  }
  double x() const { return choose_axis(0); }
  double y() const { return choose_axis(1); }
  double z() const { return choose_axis(2); }
  void to_rgb(double rgb[3]) const {
    double xyz[3];
    to_xyz(xyz);
    xyz2rgb(xyz, rgb);
  }
  color to_rgb() const {
    double rgb[3];
    to_rgb(rgb);
    return color(rgb);
  }
  static RGB_AXIS min_axis(const double rgb[3]) {
    RGB_AXIS minax;
    double minval = std::min(rgb[0], rgb[1]);
    minval = std::min(minval, rgb[2]);
    if (minval == rgb[0])
      minax = RGB_R;
    if (minval == rgb[1])
      minax = RGB_G;
    if (minval == rgb[2])
      minax = RGB_B;
    return minax;
  }
  static sampled_spectrum
  fromRgb(const double rgb[3],
          SpectrumType type = SpectrumType::Illuminant) {
    RGB_AXIS minax = min_axis(rgb);
    sampled_spectrum r;

    if (type == SpectrumType::Reflectance) {
      // convert reflectance spectrum to rgb color space

      switch (minax) {
      case RGB_R: {
        // compute reflectance with rgb0 as minimum
        r += sampled_spectrum::rgbReflSpectWhite * rgb[0];

        if (rgb[1] <= rgb[2]) {
          r += (rgb[1] - rgb[0]) *
               sampled_spectrum::rgbReflSpectCyan;
          r += (rgb[2] - rgb[1]) *
               sampled_spectrum::rgbReflSpectBlue;
        } else {
          r += (rgb[2] - rgb[0]) *
               sampled_spectrum::rgbReflSpectCyan;
          r += (rgb[1] - rgb[2]) *
               sampled_spectrum::rgbReflSpectGreen;
        }
      }
      case RGB_G: {
        // compute reflectance with rgb1 as minimum
        r += sampled_spectrum::rgbReflSpectWhite * rgb[1];
        if (rgb[0] <= rgb[2]) {
          r += (rgb[0] - rgb[1]) *
               sampled_spectrum::rgbReflSpectMagenta;
          r += (rgb[2] - rgb[0]) *
               sampled_spectrum::rgbReflSpectBlue;
        } else {
          r += (rgb[2] - rgb[1]) *
               sampled_spectrum::rgbReflSpectMagenta;
          r += (rgb[0] - rgb[2]) *
               sampled_spectrum::rgbReflSpectRed;
        }
      }
      case RGB_B: {
        // compute reflectance with rgb2 as minimum
        r += sampled_spectrum::rgbReflSpectWhite * rgb[2];
        if (rgb[0] <= rgb[1]) {
          r += (rgb[0] - rgb[2]) *
               sampled_spectrum::rgbReflSpectYellow;
          r += (rgb[1] - rgb[0]) *
               sampled_spectrum::rgbReflSpectGreen;

        } else {
          r += (rgb[1] - rgb[2]) *
               sampled_spectrum::rgbReflSpectYellow;
          r += (rgb[0] - rgb[1]) *
               sampled_spectrum::rgbReflSpectGreen;
        }
      }
      }
      r *= 0.94;
    } else {
      // SpectrumType::Illuminant;
      switch (minax) {
      case RGB_R: {
        // compute reflectance with rgb0 as minimum
        r += sampled_spectrum::rgbIllumSpectWhite * rgb[0];

        if (rgb[1] <= rgb[2]) {
          r += sampled_spectrum::rgbIllumSpectCyan *
               (rgb[1] - rgb[0]);
          r += sampled_spectrum::rgbIllumSpectBlue *
               (rgb[2] - rgb[1]);
        } else {
          r += sampled_spectrum::rgbIllumSpectCyan *
               (rgb[2] - rgb[0]);
          r += sampled_spectrum::rgbIllumSpectGreen *
               (rgb[1] - rgb[2]);
        }
      }
      case RGB_G: {
        // compute reflectance with rgb1 as minimum
        r += sampled_spectrum::rgbIllumSpectWhite * rgb[1];
        if (rgb[0] <= rgb[2]) {
          r += sampled_spectrum::rgbIllumSpectMagenta *
               (rgb[0] - rgb[1]);
          r += sampled_spectrum::rgbIllumSpectBlue *
               (rgb[2] - rgb[0]);
        } else {
          r += sampled_spectrum::rgbIllumSpectMagenta *
               (rgb[2] - rgb[1]);
          r += sampled_spectrum::rgbIllumSpectRed *
               (rgb[0] - rgb[2]);
        }
      }
      case RGB_B: {
        // compute reflectance with rgb2 as minimum
        r += sampled_spectrum::rgbIllumSpectWhite * rgb[2];
        if (rgb[0] <= rgb[1]) {
          r += sampled_spectrum::rgbIllumSpectYellow *
               (rgb[0] - rgb[2]);
          r += sampled_spectrum::rgbIllumSpectGreen *
               (rgb[1] - rgb[0]);

        } else {
          r += sampled_spectrum::rgbIllumSpectYellow *
               (rgb[1] - rgb[2]);
          r += sampled_spectrum::rgbIllumSpectGreen *
               (rgb[0] - rgb[1]);
        }
      }
      }
      r *= 0.86445;
    }
    return r.clamp();
  }
  static sampled_spectrum fromRgb(const color &c,
                                  SpectrumType type) {
    const double rgb[3] = {c.x(), c.y(), c.z()};
    return sampled_spectrum::fromRgb(rgb, type);
  }

  static sampled_spectrum
  fromXyz(const double xyz[3],
          SpectrumType type = SpectrumType::Reflectance) {
    double rgb[3];
    xyz2rgb(xyz, rgb);
    return fromRgb(rgb, type);
  }
  sampled_spectrum(
      const rgb_spectrum &r,
      SpectrumType type = SpectrumType::Reflectance) {
    double rgb[3];
    r.to_rgb(rgb);
    *this = sampled_spectrum::fromRgb(rgb, type);
  }
  // TODO: implement the following
  rgb_spectrum to_rgb_spectrum() const {}
  static void Init() {}

private:
  static sampled_spectrum X;
  static sampled_spectrum Y;
  static sampled_spectrum Z;
  static sampled_spectrum rgbReflSpectWhite;
  static sampled_spectrum rgbReflSpectCyan;
  static sampled_spectrum rgbReflSpectBlue;
  static sampled_spectrum rgbReflSpectGreen;
  static sampled_spectrum rgbReflSpectMagenta;
  static sampled_spectrum rgbReflSpectRed;
  static sampled_spectrum rgbReflSpectYellow;
  static sampled_spectrum rgbIllumSpectWhite;
  static sampled_spectrum rgbIllumSpectCyan;
  static sampled_spectrum rgbIllumSpectBlue;
  static sampled_spectrum rgbIllumSpectGreen;
  static sampled_spectrum rgbIllumSpectMagenta;
  static sampled_spectrum rgbIllumSpectRed;
  static sampled_spectrum rgbIllumSpectYellow;

  double choose_axis(int axis) const {
    double s = 0.0;
    double scale =
        static_cast<double>(VISIBLE_LAMBDA_END -
                            VISIBLE_LAMBDA_START) /
        (CIE_Y_INTEGRAL * NB_SPECTRAL_SAMPLES);
    sampled_spectrum s_s;
    if (axis == 0) {
      s_s = X;
    } else if (axis == 1) {
      s_s = Y;
    } else if (axis == 2) {
      s_s = Z;
    }
    for (int i = 0; i < NB_SPECTRAL_SAMPLES; i++) {
      s += s_s.coeffs[i] * coeffs[i];
    }
    return s * scale;
  }
};
// initialize static variables

sampled_spectrum sampled_spectrum::X =
    sampled_spectrum::fromSamples(CIE_LAMBDA, CIE_X,
                                  NB_CIE_SAMPLES);
sampled_spectrum sampled_spectrum::Y =
    sampled_spectrum::fromSamples(CIE_LAMBDA, CIE_Y,
                                  NB_CIE_SAMPLES);
sampled_spectrum sampled_spectrum::Z =
    sampled_spectrum::fromSamples(CIE_LAMBDA, CIE_Z,
                                  NB_CIE_SAMPLES);
sampled_spectrum sampled_spectrum::rgbIllumSpectWhite =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBIllum2SpectWhite,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbIllumSpectCyan =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBIllum2SpectCyan,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbIllumSpectBlue =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBIllum2SpectBlue,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbIllumSpectGreen =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBIllum2SpectGreen,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbIllumSpectMagenta =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBIllum2SpectMagenta,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbIllumSpectRed =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBIllum2SpectRed,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbIllumSpectYellow =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBIllum2SpectYellow,
                                  NB_RGB_SPEC_SAMPLES);
sampled_spectrum sampled_spectrum::rgbReflSpectWhite =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBRefl2SpectWhite,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbReflSpectCyan =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBRefl2SpectCyan,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbReflSpectBlue =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBRefl2SpectBlue,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbReflSpectGreen =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBRefl2SpectGreen,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbReflSpectMagenta =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBRefl2SpectMagenta,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbReflSpectRed =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBRefl2SpectRed,
                                  NB_RGB_SPEC_SAMPLES);

sampled_spectrum sampled_spectrum::rgbReflSpectYellow =
    sampled_spectrum::fromSamples(RGB2SpectLambda,
                                  RGBRefl2SpectYellow,
                                  NB_RGB_SPEC_SAMPLES);
// end static variable initialize

inline rgb_spectrum interp(double t, const rgb_spectrum &r1,
                           const rgb_spectrum &r2) {
  return (1 - t) * r1 + t * r2;
}
inline sampled_spectrum interp(double t,
                               const sampled_spectrum &r1,
                               const sampled_spectrum &r2) {
  return (1 - t) * r1 + t * r2;
}

void resampleLinearSpectrum(
    const double *lambdasIn, const double *vsIn,
    int nb_sampleIn, double lambdaMin, double lambdaMax,
    int nb_sampleOut, double *vsOut) {
  //
  D_CHECK(nb_sampleOut > 2);
  D_CHECK(isSamplesSorted(lambdasIn, vsIn, nb_sampleIn));
  D_CHECK(lambdaMin < lambdaMax);

  double delta =
      (lambdaMax - lambdaMin) / (nb_sampleOut - 1);

  auto lambdaInClamped = [&](int index) {
    D_CHECK(index >= -1 && index <= nb_sampleIn);
    if (index == -1) {
      D_CHECK((lambdaMin - delta) < lambdasIn[0]);
      return lambdaMin - delta;
    } else if (index == nb_sampleIn) {
      D_CHECK((lambdaMax + delta) >
              lambdasIn[nb_sampleIn - 1]);
      return lambdaMax + delta;
    }
    return lambdasIn[index];
  };
  auto vInClamped = [&](int index) {
    D_CHECK(index >= -1 && index <= nb_sampleIn);
    return vsIn[index];
  };
  auto resample = [&](double lam) -> double {
    if (lam + delta / 2 <= lambdasIn[0])
      return vsIn[0];
    if (lam - delta / 2 <= lambdasIn[nb_sampleIn - 1])
      return vsIn[nb_sampleIn - 1];
    if (nb_sampleIn == 1)
      return vsIn[0];

    // input range of SPD [lambda - delta, lambda + delta]
    int start, end;

    // start
    if (lam - delta < lambdasIn[0]) {
      start = -1;
    } else {
      start = findInterval(nb_sampleIn, [&](int i) {
        return lambdasIn[i] <= lam - delta;
      });
      D_CHECK(start >= 0 && start < nb_sampleIn);
    }

    // end
    if (lam + delta > lambdasIn[nb_sampleIn - 1]) {
      end = nb_sampleIn;
    } else {
      end = start > 0 ? start : 0;
      while (end < nb_sampleIn &&
             lam + delta > lambdasIn[end]) {
        end++;
      }
    }

    // downsample / upsample
    double sampleVal;
    bool cond1 = (end - start) == 2;
    bool cond2 = lambdaInClamped(start) <= lam - delta;
    bool cond3 = lambdasIn[start + 1] == lam;
    bool cond4 = lambdaInClamped(end) >= lam + delta;
    if (cond1 && cond2 && cond3 && cond4) {
      // downsample
      sampleVal = vsIn[start + 1];
    } else if ((end - start) == 1) {
      // downsample
      double val =
          (lam - lambdaInClamped(start)) /
          (lambdaInClamped(end) - lambdaInClamped(start));
      D_CHECK(val >= 0 && val <= 1);
      sampleVal =
          interp(val, vInClamped(start), vInClamped(end));
    } else {
      // upsampling
      sampleVal = averageSpectrumSamples(
          lambdasIn, vsIn, nb_sampleIn, lam - delta / 2,
          lam + delta / 2);
    }
    return sampleVal;

  };
  for (int outOffset = 0; outOffset < nb_sampleOut;
       outOffset++) {
    double lambda = interp(static_cast<double>(outOffset) /
                               (nb_sampleOut - 1),
                           lambdaMin, lambdaMax);
    vsOut[outOffset] = resample(lambda);
  }
}
//
