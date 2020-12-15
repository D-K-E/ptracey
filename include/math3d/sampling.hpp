#pragma once
// sampling strategies mostly for educational
// use

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
  return sum * (upper - lower) /
         static_cast<Real>(nb_samples);
}
Real estimator_uniform(std::vector<Real> f_x_samples) {
  const Real[min, max] = std::minmax_element(
      f_x_samples.begin(), f_x_samples.end());
  Real sum = 0.0;
  for (const auto &codomain_value : f_x_samples) {
    sum += codomain_value;
  }
  return sum * (max - min) /
         static_cast<Real>(f_x_samples.size());
}

Real monte_carlo_estimator(
    std::vector<D1data> f_x_input,
    const std::function<Real(D1data)> &f,
    const std::function<Real(D1data)> &p) {
  Real sum = 0.0;
  for (const auto &domain_value : f_x_input) {
    sum += f(x) / p(x);
  }
  return sum / static_cast<Real>(f_x_input.size());
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
         static_cast<Real>(nb_samples);
}
Real monte_carlo_estimator(
    std::vector<D2data> f_x_input,
    const std::function<Real(D2data)> &f,
    const std::function<Real(D2data)> &p) {
  Real sum = 0.0;
  for (const auto &domain_value : f_x_input) {
    sum += f(x) / p(x);
  }
  return sum / static_cast<Real>(f_x_input.size());
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
         (zupper - zlower) / static_cast<Real>(nb_samples);
}
Real monte_carlo_estimator(
    std::vector<D3data> f_x_input,
    const std::function<Real(D3data)> &f,
    const std::function<Real(D3data)> &p) {
  Real sum = 0.0;
  for (const auto &domain_value : f_x_input) {
    sum += f(x) / p(x);
  }
  return sum / static_cast<Real>(f_x_input.size());
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
  return sum / static_cast<Real>(f_x_input.size());
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

where \f$X_{ij}\f$ is a random variable representing jth
sample out of
\f$n_i\f$ samples generated by the ith sampling technique
and \f$ w_i(x)\f$
is the weighting functions

The optimal weighting function is explained  in the konda
2019 paper.
This is what we are going to implement.
It introduces two novel concepts called technique matrix A
and contribution
vector b in order to solve for alpha which minimizes the
variance of weights.

Once the alpha value is found the optimal estimator is
determined as



The problem of using optimal mis weights is defined in p.5:

Constraints:

- \f$ \sum_{i=1}^N w_i(x) = 1 \f$
- \f$ p_i(x) = 0 \iff w_i(x) = 0 \f$

- set \f$ {1, ..., i, ..., N} \f$ is discreet and fixed.

we have a NxN technique matrix A, whose terms are composed
of:
\f$ A = \begin{bmatrix}
        % first row
        p_0(x) \cdot \frac{p_0(x)}{\sum_{j=1}^N n_j p_j} &
        p_0(x) \cdot \frac{p_1(x)}{\sum_{j=1}^N n_j p_j} &
        p_0(x) \cdot \frac{p_2(x)}{\sum_{j=1}^N n_j p_j} &
... &
        p_0(x) \cdot \frac{p_N(x)}{\sum_{j=1}^N n_j p_j} \\
        % second row
        p_1(x) \cdot \frac{p_0(x)}{\sum_{j=1}^N n_j p_j} &
        p_1(x) \cdot \frac{p_1(x)}{\sum_{j=1}^N n_j p_j} &
        p_1(x) \cdot \frac{p_2(x)}{\sum_{j=1}^N n_j p_j} &
... &
        p_1(x) \cdot \frac{p_N(x)}{\sum_{j=1}^N n_j p_j} \\
        % passage
        \vdots & \vdots & \vdots & ... & \vdots \\
        % Nth row
        p_N(x) \cdot \frac{p_0(x)}{\sum_{j=1}^N n_j p_j} &
        p_N(x) \cdot \frac{p_1(x)}{\sum_{j=1}^N n_j p_j} &
        p_N(x) \cdot \frac{p_2(x)}{\sum_{j=1}^N n_j p_j} &
... &
        p_N(x) \cdot \frac{p_N(x)}{\sum_{j=1}^N n_j p_j}
        \end{bmatrix}
\f$

We also have a component vector b whose terms are:
\f$ b = {f \cdot \frac{p_0(x)}{\sum_{j=1}^N n_j p_j},
        f \cdot \frac{p_1(x)}{\sum_{j=1}^N n_j p_j},
        f \cdot \frac{p_2(x)}{\sum_{j=1}^N n_j p_j}, \dots,
        f \cdot \frac{p_i(x)}{\sum_{j=1}^N n_j p_j}, \dots,
        f \cdot \frac{p_N(x)}{\sum_{j=1}^N n_j p_j}}
\f$

The alpha is defined as the following: \f$ A \alpha = b \f$
This leads to the following equation which computes the
optimal weight for
the given function:
\f$
w_i(x) = \alpha_i \frac{p_i(x)}{f(x)} +
         \frac{n_i p_i(x)}{\sum_{j=1}^N n_j p_j(x)}
         (1 - \frac{\sum_{j=1}^N \alpha_j p_j(x)}{f(x)})
\f$

When we plug this into Multiple importance sampling
equation, we obtain:

\f$
F^{°} = \sum_{i=1}^N \alpha_i +
        \frac{1}{M} \sum_{i=1}^N \sum_{j=1}^{n_i} (
        \frac{f(X_{ij})}{p_c{X_{ij}}} -
        \frac{\sum_{k=1}^N \alpha_k
p_k(X_{ij})}{p_c(X_{ij})}
        )
\f$

N: number of sampling techniques.
M: \f$ \sum_{i=1}^N n_i \f$
\f$ n_i \f$: number of samples taken from \f$ p_i \f$
\f$ p_i(x) \f$: where \f$ i \in {1, ..., N} \f$ set of
probability densities.
\f$ p_c(x) \f$: \f$ \sum_{i=1}^N \frac{n_i}{M} p_i(x) \f$

According to paper in order to implement this we need to
estimate matrix A, vector b, and alpha, then we can
use them to give an estimate of f.

Estimation of matrix A is done using the following formula:
\f$ <A> = \sum_{i=1}^N \sum_{j=1}^{n_i} W_{ij} W_{ij}^T \f$

Estimation of vector b is done using the following formula:
\f$ <b> = \sum_{i=1}^N \sum_{j=1}^{n_i} f(X_{ij}) S_{ij}
W_{ij}\f$

where \f$ S_{ij} = (\sum_{k=1}^N n_k p_k(X_{ij}))^{-1} \f$
and \f$ W_{ij} = S_{ij} (p_1(X_{ij}), ..., p_N(X_{ij}))^T
               = \frac{(p_1(X_{ij}), ...,
p_N(X_{ij}))^T}{\sum_{k=1}^N n_k p_k(X_{ij})}
    \f$

The progressive implementation of these procedures is the
following
which is proved to be unbiased:
std::vector<std::function<Real(std::vector<Real>)>> p_N = {
    [](auto sample_set){
        // uniform sampling
        int mn = 0; int mx = (int)sample_set.size();
        int index = random_int(mn, mx);
        return sample_set[index];
    },
    [](auto sample_set){
        // poisson sampling etc
        int index = poisson_sampling(sample_set);
        return sample_set[index];
    }, ...
};
std::vector<Real> sample_set = {0.1, 5.0, 1.78, ..., N};
mat A = Matrix.size(N, N).filled(0);
vec b = Vector.size(N).filled(0);
vec a = Vector.size(N).filled(0);
Real result = 0.0;
int U = 3;
int max_iter = 20
for (int m = 0; m < max_iter; m++){
    //
    std::vector<std::vector<Real>> X_ijs; // jth sample out
n_i samples generated by p_i
    for (int i = 1; i < N; i++){
        //
        std::vector<Real> X_i_js;
        for (int j = 1; j < i; j++){
            Real X_ij = p_N[i](sample_set);
            X_i_js.push_back(X_ij);
        }
        X_ijs.push_back(X_i_js);
    }
    Real alpha;
    if (m >=1 && m % U == 0){
        alpha = LU_decompose_and_solve(A, b, N);
    }
    Real estimate = mis_estimate_f(alpha, p_N, sample_set);
    result = result + estimate;
    A = A + sum_w_ijs(p_N, sample_set, N);
    b = b + sum_f_sijs(p_N, sample_set, N);
}
 */
using FnVec =
    std::vector<std::function<Real(std::vector<Real>)>>;

Real mis_estimate_f(
    const VectorXf &alpha,
    const std::vector<Real> &sample_set,
    const std::function<Real(Real)> &f, const FnVec &p_N,
    const std::map<int, int> &nb_samples_taken_from_p_N,
    const std::vector<std::vector<Real>> &X_samples_by_pN,
    int N) {
  // \sum_{i=1}^N \alpha_i
  Real alpha_sum = alpha.sum();

  // \frac{1}{M}
  int M = 0;
  for (int i = 1; i < N; i++) {
    int nb_samples_taken_from_p_i =
        nb_samples_taken_from_p_N.at(i);
    M += nb_samples_taken_from_p_i;
  }
  Real frac_1_M = 1 / static_cast<Real>(M);

  auto p_c = [M, N, nb_samples_taken_from_p_N,
              p_N](Real x) {
    Real sumval = 0.0;
    for (int i = 0; i < N; i++) {
      int n_i = nb_samples_taken_from_p_N.at(i);
      Real n_i_over_M =
          static_cast<Real>(n_i) / static_cast<Real>(M);
      auto p_i = p_N[i];
      Real p_i_x = p_i(x);
      sumval += n_i_over_M * p_i_x;
    }
  };

  auto sum_alpha_pk = [p_N, alpha, N](Real x) -> Real {
    Real sumval = 0;
    for (int k = 0; k < N; k++) {
      Real alpha_k = alpha[k];
      auto p_k = p_N[k];
      sumval += alpha_k * p_k(x);
    }
    return sumval;
  };

  //\sum_{i=1}^N \sum_{j=1}^{n_i}
  Real i_sum = 0.0;
  for (int i = 0; i < N; i++) {
    int n_i = nb_samples_taken_from_p_N.at(i);
    Real j_sum = 0.0;
    for (int j = 0; j < N; j++) {
      // \frac{f(X_{ij})}{p_c{X_{ij}}}
      Real X_ij = X_samples_by_pN[i][j];
      Real f_x = f(X_ij);
      Real p_cx = p_c(X_ij);
      Real f_x_over_p_c = f_x / p_cx;

      /**\frac{\sum_{k=1}^N \alpha_k
       * p_k(X_{ij})}{p_c(X_{ij})}*/
      Real alpha_k_sum = sum_alpha_pk(X_ij);
      Real alpha_k_sum_over_pc = alpha_k_sum / p_c;
      j_sum += f_x_over_p_c - alpha_k_sum_over_pc;
    }
    i_sum += j_sum;
  }
  Real m_i_sum = frac_1_M * i_sum;
  return m_i_sum + alpha_sum;
}

Real mis_optimal() {
  //
  std::vector<Real> sample_set = {0.1, 5.0, 1.78, 0.46};
  auto sample_fn = [](Real x) -> Real {
    return (x * x) / 2.0;
  } for (int i = 0; i < 20; i++) {
    sample_set.push_back(
        sample_fn(1 / static_cast<Real>(i)));
  }
  int sampleN = (int)sample_set.size();
  MatrixXf A = MatrixXf::Zero(sampleN, sampleN);
  VectorXf b = MatrixXf::Zero(sampleN);
  VectorXf alpha = MatrixXf::Zero(sampleN);
  Real result = 0.0;
  int U = 3;
  int max_iter = 20;
  // probability density functions all uniform for now
  FnVec p_N(sampleN);
  for (int h = 0; h < sampleN; h++) {
    p_N[h] = [](std::vector<Real> sset) -> Real {
      int mn = 0;
      int mx = (int)sset.size();
      int index = random_int(mn, mx);
      return sset[index];
    };
  }
  for (int m = 0; m < max_iter; m++) {
    //
    /** n_i samples generated by p_i vector */
    std::vector<std::vector<Real>> X_samples_by_pN;

    std::map<int, int> nb_samples_taken_from_p_N;
    for (int i = 0; i < sampleN; i++) {
      //
      int nb_samples_taken_from_p_i = 0;
      std::vector<Real> samples_taken_from_p_i;
      for (int j = 0; j < i; j++) {
        auto p_i = p_N[i];
        Real X_ij = p_i(sample_set);
        samples_taken_from_p_i.push_back(X_ij);
        nb_samples_taken_from_p_i++;
      }
      nb_samples_taken_from_p_N.insert(
          make_pair(i, nb_samples_taken_from_p_i));
      X_samples_by_pN.push_back(samples_taken_from_p_i);
    }
    if (m >= 1 && m % U == 0) {
      alpha = A.colPivHouseholderQr().solve(b);
    }
    Real estimate =
        mis_estimate_f(alpha, p_N, sample_set, sample_fn,
                       nb_samples_taken_from_p_N,
                       X_samples_by_pN, sampleN);
    result = result + estimate;
    A = A + sum_w_ijs(p_N, sample_set, sampleN);
    b = b + sum_f_sijs(p_N, sample_set, sampleN);
  }
}
