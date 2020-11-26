#pragma once
#include <math3d/onb.hpp>
#include <camera/ray.hpp>
#include <math3d/vec3.hpp>
using namespace ptracey;
namespace ptracey {
class pdf {
public:
  virtual ~pdf() {}

  virtual Real value(const vec3 &direction) const = 0;
  virtual vec3 generate() const = 0;
};

class cosine_pdf : public pdf {
public:
  cosine_pdf(const vec3 &w) { uvw.build_from_w(w); }
  Real value(const vec3 &direction) const override {
    auto cosine = dot(unit_vector(direction), uvw.w());
    return (cosine <= 0) ? 0 : cosine / M_PI;
  }
  vec3 generate() const override {
    return uvw.local(random_cosine_direction());
  }

public:
  onb uvw;
};

class hittable_pdf : public pdf {
public:
  hittable_pdf(shared_ptr<hittable> p, const point3 &origin)
      : o(origin), ptr(p) {}

  Real value(const vec3 &direction) const override {
    return ptr->pdf_value(o, direction);
  }

  vec3 generate() const override { return ptr->random(o); }

public:
  point3 o;
  shared_ptr<hittable> ptr;
};

class mixture_pdf : public pdf {
public:
  mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
    p[0] = p0;
    p[1] = p1;
  }

  Real value(const vec3 &direction) const override {
    return 0.5 * p[0]->value(direction) +
           0.5 * p[1]->value(direction);
  }

  vec3 generate() const override {
    if (random_real() < 0.5)
      return p[0]->generate();
    else
      return p[1]->generate();
  }

public:
  shared_ptr<pdf> p[2];
};
}
