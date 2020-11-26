#pragma once
#include <common.hpp>
#include <hittable/hittable.hpp>
#include <camera/ray.hpp>
#include <utils.hpp>
#include <math3d/vec3.hpp>
using namespace ptracey;

namespace ptracey {

class hittable_list : public hittable {
public:
  hittable_list() {}
  hittable_list(shared_ptr<hittable> object) {
    add(object);
  }
  hittable_list(std::vector<shared_ptr<hittable>> objs) {
    objects.clear();
    for (const auto &obj : objs)
      add(obj);
  }

  void clear() { objects.clear(); }
  void add(shared_ptr<hittable> object) {
    objects.push_back(object);
  }

  virtual bool hit(const ray &r, Real t_min, Real t_max,
                   hit_record &rec) const override;

  virtual bool
  bounding_box(Real time0, Real time1,
               aabb &output_box) const override;
  virtual Real pdf_value(const vec3 &o,
                         const vec3 &v) const override;
  virtual vec3 random(const vec3 &o) const override;

public:
  std::vector<shared_ptr<hittable>> objects;
};
bool hittable_list::hit(const ray &r, Real t_min,
                        Real t_max, hit_record &rec) const {
  hit_record temp_rec;
  auto hit_anything = false;
  auto closest_so_far = t_max;

  for (const auto &object : objects) {
    if (object->hit(r, t_min, closest_so_far, temp_rec)) {
      hit_anything = true;
      closest_so_far = temp_rec.t;
      rec = temp_rec;
    }
  }

  return hit_anything;
}
bool hittable_list::bounding_box(Real time0, Real time1,
                                 aabb &output_box) const {
  if (objects.empty())
    return false;

  aabb temp_box;
  bool first_box = true;

  for (const auto &object : objects) {
    if (!object->bounding_box(time0, time1, temp_box))
      return false;
    output_box =
        first_box ? temp_box
                  : surrounding_box(output_box, temp_box);
    first_box = false;
  }

  return true;
}
Real hittable_list::pdf_value(const point3 &o,
                              const vec3 &v) const {
  auto weight = 1.0 / objects.size();
  auto sum = 0.0;

  for (const auto &object : objects)
    sum += weight * object->pdf_value(o, v);

  return sum;
}
vec3 hittable_list::random(const vec3 &o) const {
  auto int_size = static_cast<int>(objects.size());
  return objects[random_int(0, int_size - 1)]->random(o);
}
}
