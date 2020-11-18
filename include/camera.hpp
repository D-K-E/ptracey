#pragma once

#include <common.hpp>
#include <ray.hpp>
#include <utils.hpp>
#include <vec3.hpp>
//
using namespace ptracey;
namespace ptracey {
class camera {
public:
  camera()
      : camera(point3(0, 0, -1), point3(0, 0, 0),
               vec3(0, 1, 0), 40, 1, 0, 10) {}

  camera(point3 lookfrom, point3 lookat, vec3 vup,
         Real vfov, // vertical field-of-view in degrees
         Real aspect_ratio, Real aperture, Real focus_dist,
         Real _time0 = 0, Real _time1 = 0) {
    auto theta = degrees_to_radians(vfov);
    auto h = tan(theta / 2);
    auto viewport_height = 2.0 * h;
    auto viewport_width = aspect_ratio * viewport_height;

    w = unit_vector(lookfrom - lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    origin = lookfrom;
    horizontal = focus_dist * viewport_width * u;
    vertical = focus_dist * viewport_height * v;
    lower_left_corner = origin - horizontal / 2 -
                        vertical / 2 - focus_dist * w;

    lens_radius = aperture / 2;
    time0 = _time0;
    time1 = _time1;
  }

  ray get_ray(Real s, Real t) const {
    vec3 rd = lens_radius * random_in_unit_disk();
    vec3 offset = u * rd.x() + v * rd.y();
    return ray(
        origin + offset,
        lower_left_corner + s * horizontal + t * vertical -
            origin - offset,
        random_real(time0, time1),
        static_cast<unsigned int>(random_int(360, 830)));
  }

private:
  point3 origin;
  point3 lower_left_corner;
  vec3 horizontal;
  vec3 vertical;
  vec3 u, v, w;
  Real lens_radius;
  Real time0, time1; // shutter open/close times
};
}
