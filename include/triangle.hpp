#pragma once
#include <hittable.hpp>
#include <info.hpp>
#include <ray.hpp>
#include <vec3.hpp>

class triangle : public hittable {
public:
  shared_ptr<material> mat_ptr;
  point3 p0, p1, p2;
  //
  triangle() {}
  triangle(point3 _p0, point3 _p1, point3 _p2,
           shared_ptr<material> mat)
      : p0(_p0), p1(_p1), p2(_p2), mat_ptr(mat) {}
  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    auto edge1 = p1 - p0;
    auto edge2 = p2 - p0;
    auto point_vec = cross(r.direction(), edge2);
    auto det = dot(edge1, point_vec);
    auto invDet = 1.0 / det;
    auto distVec = r.origin() - p0;
    auto u = dot(distVec, point_vec) * invDet;

    if (u < 0.0 || u > 1.0) {
      return false;
    }

    auto qvec = cross(distVec, edge1);
    auto v = dot(r.direction(), qvec) * invDet;

    if (v < 0.0 || v + u > 1.0)
      return false;
    auto t = dot(edge2, qvec) * invDet;
    if (t >= t_min && t <= t_max) {

      rec.v = v;
      rec.u = u;
      rec.t = t;
      rec.p = r.at(rec.t);
      rec.mat_ptr = mat_ptr;
      vec3 outnormal = cross(edge1, edge2);
      rec.set_face_normal(r, outnormal);
      return true;
    }
    return false;
  }
  vec3 random(const vec3 &o) const override {
    // from A. Glassner, Graphics Gems, 1995, p. 24
    auto t = random_real();
    auto ss = random_real();
    auto a = 1 - sqrt(t);
    auto b = (1 - ss) * sqrt(t);
    auto c = ss * sqrt(t);
    auto random_point = a * p0 + b * p1 + c * p2;
    return random_point - o;
  }
  bool bounding_box(double time0, double time1,
                    aabb &output_box) const override {
    point3 minp = min_vec(p0, p1);
    minp = min_vec(minp, p2);
    point3 maxp = min_vec(p0, p1);
    maxp = max_vec(maxp, p2);
    output_box = aabb(minp, maxp);
    return true;
  }
  double pdf_value(const vec3 &o,
                   const vec3 &v) const override {
    hit_record rec;
    if (!hit(ray(o, v), 0.001, FLT_MAX, rec))
      return 0.0;

    // herons formula
    auto side1 = (p0 - p1).length();
    auto side2 = (p0 - p2).length();
    auto side3 = (p1 - p2).length();
    auto half_p = (side1 + side2 + side3) / 2.0;
    auto area = sqrt(half_p * (half_p - side1) *
                     (half_p - side2) * (half_p - side3));
    return get_pdf_surface(v, rec.normal, rec.t, area);
  }
};
class AaTriangle : public triangle {

public:
  double k; // common axis
  shared_ptr<material> mat_ptr;
  double a0, a01, a1, // aligned1
      b0, b1;         // aligned2

  AaTriangle() {}
  AaTriangle(double a_0, double a_01, double a_1,
             double b_0, double b_1, double _k,
             shared_ptr<material> mat)
      : a0(a_0), a01(a_01), a1(a_1), b0(b_0), b1(b_1),
        k(_k), triangle(point3(a_0, b_0, _k),
                        point3(a_01, b_0, _k),
                        point3(a_1, b_1, _k), mat) {}
};
class xy_tri : public AaTriangle {
public:
  double x0, x01, x1, y0, y1;

  xy_tri() {}
  xy_tri(double _x0, double _x01, double _x1, double _y0,
         double _y1, double _k, shared_ptr<material> mat)
      : AaTriangle(_x0, _x01, _x1, _y0, _y1, _k, mat),
        x0(_x0), x01(_x01), x1(_x1), y0(_y0), y1(_y1) {}
};
class xz_tri : public AaTriangle {
public:
  double x0, x01, x1, z0, z1;

  xz_tri() {}
  xz_tri(double _x0, double _x01, double _x1, double _z0,
         double _z1, double _k, shared_ptr<material> mat)
      : AaTriangle(_x0, _x01, _x1, _z0, _z1, _k, mat),
        x0(_x0), x01(_x01), x1(_x1), z0(_z0), z1(_z1) {}
};
class yz_tri : public AaTriangle {
public:
  double y0, y01, y1, z0, z1;

  yz_tri() {}
  yz_tri(double _y0, double _y01, double _y1, double _z0,
         double _z1, double _k, shared_ptr<material> mat)
      : AaTriangle(_y0, _y01, _y1, _z0, _z1, _k, mat),
        y0(_y0), y01(_y01), y1(_y1), z0(_z0), z1(_z1) {}
};
