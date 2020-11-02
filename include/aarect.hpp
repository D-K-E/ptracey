#pragma once
#include <aabb.hpp>
#include <hittable.hpp>
#include <info.hpp>
#include <ray.hpp>
#include <vec3.hpp>

class AaRect : public hittable {
protected:
  vec3 axis_normal;
  double a0, a1, // aligned1
      b0, b1;    // aligned2
  AxisInfo ax;

public:
  double k;
  shared_ptr<material> mat_ptr;

public:
  AaRect() {}
  AaRect(double a_0, double a_1, double b_0, double b_1,
         double _k, shared_ptr<material> mat, vec3 anormal)
      : a0(a_0), a1(a_1), b0(b_0), b1(b_1), k(_k),
        mat_ptr(mat), axis_normal(anormal) {
    if (anormal.z() == 1.0) {
      ax.aligned1 = 0;
      ax.aligned2 = 1;
      ax.notAligned = 2;
    } else if (anormal.x() == 1.0) {
      ax.aligned1 = 1;
      ax.aligned2 = 2;
      ax.notAligned = 0;
    } else if (anormal.y() == 1.0) {
      ax.aligned1 = 0;
      ax.aligned2 = 2;
      ax.notAligned = 1;
    }
  }
  bool hit(const ray &r, double t0, double t1,
           hit_record &rec) const override {
    /*
       point of intersection satisfies
       both P = O + D*m Point = Origin + Direction *
       magnitude
       and
       x0 < x_i < x1 y0 < y_i < y1 y0,y1 and x0,x1 being the
       limits of
       rectangle
     */
    auto t = (k - r.origin()[ax.notAligned]) /
             r.direction()[ax.notAligned];
    if (t < t0 || t > t1)
      return false;
    double a = r.origin()[ax.aligned1] +
               t * r.direction()[ax.aligned1];
    double b = r.origin()[ax.aligned2] +
               t * r.direction()[ax.aligned2];
    bool c1 = a0 < a and a < a1;
    bool c2 = b0 < b and b < b1;
    if ((c1 and c2) == false) {
      return false;
    }
    rec.u = (a - a0) / (a1 - a0);
    rec.v = (b - b0) / (b1 - b0);
    rec.t = t;
    vec3 outward_normal = axis_normal;
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mat_ptr;
    rec.p = r.at(t);
    return true;
  }
  bool bounding_box(double t0, double t1,
                    aabb &output_box) const override {
    // The bounding box must have non-zero width in each
    // dimension, so pad the Z
    // dimension a small amount.
    point3 p1, p2;
    // choose points with axis
    if (ax.notAligned == 2) {
      p1 = point3(a0, b0, k - 0.0001);
      p2 = point3(a1, b1, k + 0.0001);
    } else if (ax.notAligned == 1) {
      p1 = point3(a0, k - 0.0001, b0);
      p2 = point3(a1, k + 0.0001, b1);
    } else if (ax.notAligned == 0) {
      p1 = point3(k - 0.0001, a0, b0);
      p2 = point3(k + 0.0001, a1, b1);
    }
    output_box = aabb(p1, p2);
    return true;
  }
};

class xy_rect : public AaRect {
public:
  double x0, x1, y0, y1;

public:
  xy_rect() {}
  xy_rect(double _x0, double _x1, double _y0, double _y1,
          double _k, shared_ptr<material> mat)
      : AaRect(_x0, _x1, _y0, _y1, _k, mat, vec3(0.0, 0.0, 1.0)),
        x0(_x0), x1(_x1), y0(_y0), y1(_y1) {}
};
class xz_rect : public AaRect {
public:
  double x0, x1, z0, z1;

public:
  xz_rect() {}
  xz_rect(double _x0, double _x1, double _z0, double _z1,
          double _k, shared_ptr<material> mat)
      : AaRect(_x0, _x1, _z0, _z1, _k, mat, vec3(0.0, 1.0, 0.0)),
        x0(_x0), x1(_x1), z0(_z0), z1(_z1) {}
  double pdf_value(const point3 &origin,
                   const vec3 &dir) const override {
    hit_record rec;
    if (!this->hit(ray(origin, dir), 0.001, FLT_MAX, rec))
      return 0;

    auto area = (x1 - x0) * (z1 - z0);
    // auto distance_squared = rec.dist * rec.dist *
    // length_squared(dir);
    // auto cosine = fabs(dot(dir, rec.normal) /
    // length(dir));

    // return distance_squared / (cosine * area);
    return get_pdf_surface(dir, rec.normal, rec.t, area);
  }

  vec3 random(const point3 &origin) const override {
    auto random_point = point3(random_double(x0, x1), k,
                               random_double(z0, z1));
    return random_point - origin;
  }
};
class yz_rect : public AaRect {
public:
  double y0, y1, z0, z1;

public:
  yz_rect() {}
  yz_rect(double _y0, double _y1, double _z0, double _z1,
          double _k, shared_ptr<material> mat)
      : AaRect(_y0, _y1, _z0, _z1, _k, mat, vec3(1, 0, 0)),
        y0(_y0), y1(_y1), z0(_z0), z1(_z1) {}
};
