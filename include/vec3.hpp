#pragma once
#include <common.hpp>
#include <utils.hpp>

namespace ptracey {

class vec3 {
public:
  vec3() : e{0, 0, 0} {}
  vec3(Real e0, Real e1, Real e2) : e{e0, e1, e2} {}
  vec3(Real e0) : e{e0, e0, e0} {}
  vec3(const Real e1[3]) : e{e1[0], e1[1], e1[2]} {}

  Real x() const { return e[0]; }
  Real y() const { return e[1]; }
  Real z() const { return e[2]; }
  Real r() const { return x(); }
  Real g() const { return y(); }
  Real b() const { return z(); }

  vec3 operator-() const {
    return vec3(-e[0], -e[1], -e[2]);
  }
  Real operator[](int i) const { return e[i]; }
  Real &operator[](int i) { return e[i]; }
  vec3 &operator+=(const vec3 &v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return *this;
  }
  vec3 &operator*=(const Real t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }
  vec3 &operator/=(const Real t) { return *this *= 1 / t; }
  vec3 add(const vec3 &v) {
    return vec3(x() + v.x(), y() + v.y(), z() + v.z());
  }
  Real sum() { return x() + y() + z(); }
  Real average() { return sum() / 3.0; }
  shared_ptr<vec3> add(const shared_ptr<vec3> &v) {
    return make_shared<vec3>(x() + v->x(), y() + v->y(),
                             z() + v->z());
  }
  vec3 add(const Real &v) {
    return vec3(x() + v, y() + v, z() + v);
  }
  shared_ptr<vec3> add(const shared_ptr<Real> vptr) {
    return make_shared<vec3>(add(*vptr));
  }
  vec3 subt(const Real &v) {
    return vec3(x() - v, y() - v, z() - v);
  }
  vec3 subt(const vec3 &v) {
    return vec3(x() - v.x(), y() - v.y(), z() - v.z());
  }
  shared_ptr<vec3> subt(const shared_ptr<vec3> &v) {
    return make_shared<vec3>(x() - v->x(), y() - v->y(),
                             z() - v->z());
  }
  shared_ptr<vec3> subt(const shared_ptr<Real> vptr) {
    return make_shared<vec3>(subt(*vptr));
  }
  vec3 multip(const vec3 &v) {
    return vec3(x() * v.x(), y() * v.y(), z() * v.z());
  }
  shared_ptr<vec3> multip(const shared_ptr<vec3> &v) {
    return make_shared<vec3>(x() * v->x(), y() * v->y(),
                             z() * v->z());
  }
  shared_ptr<vec3> multip(const shared_ptr<Real> &v) {
    return make_shared<vec3>(multip(*v));
  }
  vec3 multip(const Real &v) {
    return vec3(x() * v, y() * v, z() * v);
  }
  vec3 div(const Real &v) {
    if (v == 0.0)
      throw std::runtime_error("no zero division");
    return vec3(x() / v, y() / v, z() / v);
  }
  vec3 div(const vec3 &v) {
    if (v.x() == 0.0)
      throw std::runtime_error(
          "no zero division x is zero");
    if (v.y() == 0.0)
      throw std::runtime_error(
          "no zero division y is zero");
    if (v.z() == 0.0)
      throw std::runtime_error(
          "no zero division z is zero");
    return vec3(x() / v.x(), y() / v.y(), z() / v.z());
  }
  shared_ptr<vec3> div(const shared_ptr<vec3> &v) {
    auto vs = *v;
    vs = div(vs);
    return make_shared<vec3>(vs.x(), vs.y(), vs.z());
  }
  shared_ptr<vec3> div(const shared_ptr<Real> &v) {
    return make_shared<vec3>(div(*v));
  }
  Real length() const { return sqrt(length_squared()); }
  Real length_squared() const {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }
  inline Real min() const {
    return fmin(fmin(x(), y()), z());
  }
  inline Real max() const {
    return fmax(fmax(x(), y()), z());
  }
  bool near_zero() const {
    // Return true if the vector is close to zero in all
    // dimensions.
    const auto s = 1e-8;
    return (fabs(e[0]) < s) && (fabs(e[1]) < s) &&
           (fabs(e[2]) < s);
  }
  inline static vec3 random() {
    return vec3(random_real(), random_real(),
                random_real());
  }
  inline static vec3 random(Real min, Real max) {
    return vec3(random_real(min, max),
                random_real(min, max),
                random_real(min, max));
  }

public:
  Real e[3];
};

// Type aliases for vec3
using point3 = vec3; // 3D point

// vec3 Utility Functions

inline std::ostream &operator<<(std::ostream &out,
                                const vec3 &v) {
  return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3 &u, const vec3 &v) {
  return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1],
              u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v) {
  return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1],
              u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3 &u, const vec3 &v) {
  return vec3(u.e[0] * v.e[0], u.e[1] * v.e[1],
              u.e[2] * v.e[2]);
}

inline vec3 operator*(Real t, const vec3 &v) {
  return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline vec3 operator*(const vec3 &v, Real t) {
  return t * v;
}

inline vec3 operator/(vec3 v, Real t) {
  return (1 / t) * v;
}

inline Real dot(const vec3 &u, const vec3 &v) {
  return u.e[0] * v.e[0] + u.e[1] * v.e[1] +
         u.e[2] * v.e[2];
}

inline vec3 cross(const vec3 &u, const vec3 &v) {
  return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
              u.e[2] * v.e[0] - u.e[0] * v.e[2],
              u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(vec3 v) { return v / v.length(); }
inline vec3 min_vec(const vec3 &v1, const vec3 &v2) {
  float x = fmin(v1.x(), v2.x());
  float y = fmin(v1.y(), v2.y());
  float z = fmin(v1.z(), v2.z());
  return vec3(x, y, z);
}
inline vec3 max_vec(const vec3 &v1, const vec3 &v2) {
  float x = fmax(v1.x(), v2.x());
  float y = fmax(v1.y(), v2.y());
  float z = fmax(v1.z(), v2.z());
  return vec3(x, y, z);
}

inline vec3 random_in_unit_disk() {
  while (true) {
    auto p =
        vec3(random_real(-1, 1), random_real(-1, 1), 0);
    if (p.length_squared() >= 1)
      continue;
    return p;
  }
}

inline vec3 random_in_unit_sphere() {
  while (true) {
    auto p = vec3::random(-1, 1);
    if (p.length_squared() >= 1)
      continue;
    return p;
  }
}

inline vec3 random_unit_vector() {
  return unit_vector(random_in_unit_sphere());
}

inline vec3 random_in_hemisphere(const vec3 &normal) {
  vec3 in_unit_sphere = random_in_unit_sphere();
  if (dot(in_unit_sphere, normal) >
      0.0) // In the same hemisphere as the normal
    return in_unit_sphere;
  else
    return -in_unit_sphere;
}

inline vec3 reflect(const vec3 &v, const vec3 &n) {
  return v - 2 * dot(v, n) * n;
}

inline vec3 refract(const vec3 &uv, const vec3 &n,
                    Real etai_over_etat) {
  auto cos_theta = fmin(dot(-uv, n), 1.0);
  vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
  vec3 r_out_parallel =
      -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
  return r_out_perp + r_out_parallel;
}

inline vec3 random_cosine_direction() {
  auto r1 = random_real();
  auto r2 = random_real();
  auto z = sqrt(1 - r2);

  auto phi = 2 * M_PI * r1;
  auto x = cos(phi) * sqrt(r2);
  auto y = sin(phi) * sqrt(r2);

  return vec3(x, y, z);
}

inline vec3 random_to_sphere(Real radius,
                             Real distance_squared) {
  auto r1 = random_real();
  auto r2 = random_real();
  auto z =
      1 +
      r2 * (sqrt(1 - radius * radius / distance_squared) -
            1);

  auto phi = 2 * M_PI * r1;
  auto x = cos(phi) * sqrt(1 - z * z);
  auto y = sin(phi) * sqrt(1 - z * z);

  return vec3(x, y, z);
}
}
