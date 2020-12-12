#pragma once
// sampling strategies mostly for educational
// use

#include <algorithm>
#include <common.hpp>
#include <math3d/vec3.hpp>
#include <utils.hpp>

// data types

struct D1data {
  Real x;
  D1data() : x(0) {}
  D1data(Real a) : x(a) {}
};
struct D2data {
  Real x;
  Real y;
  D2data() : x(0), y(0) {}
  D2data(Real a) : x(a), y(a) {}
  D2data(Real a[2]) : x(a[0]), y(a[1]) {}
  D2data(Real a, Real b) : x(a), y(b) {}
};
struct D3data {
  Real x;
  Real y;
  Real z;
  D3data(Real a) : x(a), y(a), z(a) {}
  D3data(Real a[3]) : x(a[0]), y(a[1]), z(a[2]) {}
  D3data(Real a, Real b, Real c) : x(a), y(b), z(c) {}
  D3data(vec3 v) : x(v.x()), y(v.y()), z(v.z()) {}
};
struct DNdata {
  std::vector<Real> data;
  DNdata(const std::vector<Real> &d) : data(d) {}
};

// 1 d cases

Real f(D1data data) { return data.x * data.x; }
Real f2(D2data data) { return data.x * data.y; }
Real f3(D3data data) { return (data.x * data.y) / data.z; }

/**
  I have a function f. Monte carlo estimation of the
  integral \f$ \int_a^b
  f(x)dx\f$ is
  \f$ \frac{b - a}{N} \sum_{i=1}^N f(X_i) \f$ where X is
  the sampled in some way from the codomain of the function.

  uniform sampling means that I am simply generating my
input in from a
  uniform distribution.

  Importance sampling means that I am transforming the above
equation to
following: \f$ \frac{1}{N} \sum_{i=1}^N
\frac{f(X_i)}{p(X_i)} \f$ which is not
very far if we consider that the above equation can be
rewritten in this form
if we accept \f$p(X_i)\f$ as \f$\frac{1}{p(X_i)}\f$
 */
Real estimator_uniform(
    Real lower, Real upper, uint nb_samples,
    const std::function<Real(D1data)> &fn = f) {
  Real sum = 0.0;
  for (uint i = 1; i < nb_samples; i++) {
    D1data domain_value;
    domain_value.x = random_real(lower, upper);
    Real codomain_value = f(domain_value);
    sum += codomain_value;
  }
  return sum * (upper - lower) / (Real)nb_samples;
}
Real estimator_uniform(std::vector<Real> f_x_samples) {
  const Real[min, max] = std::minmax_element(
      f_x_samples.begin(), f_x_samples.end());
  Real sum = 0.0;
  for (const auto &codomain_value : f_x_samples) {
    sum += codomain_value;
  }
  return sum * (max - min) / (Real)f_x_samples.size();
}

Real monte_carlo_estimator(
    std::vector<D1data> f_x_input,
    const std::function<Real(D1data)> &f,
    const std::function<Real(D1data)> &p) {
  Real sum = 0.0;
  for (const auto &domain_value : f_x_input) {
    sum += f(x) / p(x);
  }
  return sum / (Real)f_x_input.size();
}

// 2 d cases
Real estimator_uniform(
    Real xlower, Real xupper, Real ylower, Real yupper,
    uint nb_samples,
    const std::function<Real(D2data)> &fn = f2) {
  Real sum = 0.0;
  for (uint i = 1; i < nb_samples; i++) {
    D2data domain_value;
    domain_value.x = random_real(xlower, xupper);
    domain_value.y = random_real(ylower, yupper);
    Real codomain_value = f2(domain_value);
    sum += codomain_value;
  }
  return sum * (xupper - xlower) * (yupper - ylower) /
         (Real)nb_samples;
}
Real monte_carlo_estimator(
    std::vector<D2data> f_x_input,
    const std::function<Real(D2data)> &f,
    const std::function<Real(D2data)> &p) {
  Real sum = 0.0;
  for (const auto &domain_value : f_x_input) {
    sum += f(x) / p(x);
  }
  return sum / (Real)f_x_input.size();
}

// 3 d cases
Real estimator_uniform(
    Real xlower, Real xupper, Real ylower, Real yupper,
    Real zlower, Real zupper, uint nb_samples,
    const std::function<Real(D3data)> &fn = f3) {
  Real sum = 0.0;
  for (uint i = 1; i < nb_samples; i++) {
    D3data domain_value;
    domain_value.x = random_real(xlower, xupper);
    domain_value.y = random_real(ylower, yupper);
    domain_value.z = random_real(zlower, zupper);
    Real codomain_value = f3(domain_value);
    sum += codomain_value;
  }
  return sum * (xupper - xlower) * (yupper - ylower) *
         (zupper - zlower) / (Real)nb_samples;
}
Real monte_carlo_estimator(
    std::vector<D3data> f_x_input,
    const std::function<Real(D3data)> &f,
    const std::function<Real(D3data)> &p) {
  Real sum = 0.0;
  for (const auto &domain_value : f_x_input) {
    sum += f(x) / p(x);
  }
  return sum / (Real)f_x_input.size();
}

// Nd cases
Real monte_carlo_estimator(
    std::vector<DNdata> f_x_input,
    const std::function<Real(DNdata)> &f,
    const std::function<Real(DNdata)> &p) {
  Real sum = 0.0;
  for (const auto &domain_value : f_x_input) {
    sum += f(x) / p(x);
  }
  return sum / (Real)f_x_input.size();
}

/**
  Multiple importance sampling

  Multiple importance sampling means that we are sampling
  functions like \f$ \int f(x)g(x) \f$
  Pbrt proposes along with Veach the balance heuristic
https://graphics.stanford.edu/courses/cs348b-03/papers/veach-chapter9.pdf

https://graphics.cg.uni-saarland.de/papers/konda-2019-siggraph-optimal-mis.pdf

The estimator for the multiple importance sampling is
\f$ \sum_{i=1}^N \sum_{j=1}^{n_i}
\frac{w_{i}(X_{ij})f(X_{ij})}{n_ip_i(X_{ij})} \f$

where \f$X_{ij}\f$ is a random variable representing jth sample out of
\f$n_i\f$ samples generated by the ith sampling technique and \f$ w_i(x)\f$
is the weighting functions

The optimal weighting function is explained  in the konda 2019 paper.
This is what we are going to implement.
It introduces two novel concepts called technique matrix A and contribution
vector b in order to solve for alpha which minimizes the variance of weights.
 */
