#pragma once
#include <color/spectrum.hpp>
#include <common.hpp>
#include <hittable.hpp>
#include <material.hpp>
#include <ray.hpp>
using namespace ptracey;
namespace ptracey {

/**
  main rendering code equation:

\f[
L_o(x, w_o, \lambda, t) =
    L_e(x, w_o, \lambda, t) +
    \int_{\omega} f_r(x,w_i,w_o,\lambda,t)
        L_i(x,w_i,\lambda,t)
        (w_i \cdot n) d_w
\f]

- x: point in surface
- w_o: outgoing direction
- w_i: incoming direction
- lambda: wavelength
- t: time
- n: surface normal

 */
color ray_color(const ray &r, const spectrum background,
                const hittable &world, int depth) {
  hit_record rec;
  auto wlength = r.wavelength();
  // add the wavelength if it is not already present

  // If we've exceeded the ray bounce limit, no more light
  // is gathered.
  if (depth <= 0) {
    // L_e + L_r = 0 = L_o
    return color(0.0);
  }

  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, FLT_MAX, rec)) {
    // L_e + L_r = background = L_o
    color back_lambda = background.evaluate(wlength);
    return back_lambda;
  }

  scatter_record srec;
  color emitted =
      rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);

  if (!rec.mat_ptr->scatter(r, rec, srec)) {
    // L_e + 0 = L_o
    return emitted;
  }

  if (srec.is_specular) {
    // f_r * L_i + 0 = L_r + L_e = 0
    // TODO what if the material changes the wavelength of
    // the ray ??
    color atten_color = srec.attenuation;
    color r_col = ray_color(srec.specular_ray, background,
                            world, depth - 1);
    auto spec_color = atten_color * r_col;
    return spec_color;
  }

  // w_o
  ray scattered = ray(rec.p, srec.pdf_ptr->generate(),
                      r.time(), wlength);
  Real pdf_val = srec.pdf_ptr->value(scattered.direction());
  Real scatter_pdf =
      rec.mat_ptr->scattering_pdf(r, rec, scattered);
  color r_color =
      ray_color(scattered, background, world, depth - 1);
  color f_r = srec.attenuation;
  //

  return emitted + f_r * scatter_pdf * r_color / pdf_val;
}
}
