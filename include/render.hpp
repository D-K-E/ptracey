#pragma once
#include <common.hpp>
#include <hittable.hpp>
#include <material.hpp>
#include <ray.hpp>
#include <spectrum.hpp>
using namespace ptracey;
namespace ptracey {
/**
  main rendering code equation:

\f[
L_o(x, w_o, \lambda, t) =
    L_e(x, w_o, \lambda, t)
    \int_{\omega} f_r(x,w_i,w_o,\lambda,t)
        L_i(x,w_i,\lambda,t)
        (w_i \cdot n) d_w
\f]
 */
shared_ptr<spectrum>
ray_color(const ray &r,
          const shared_ptr<spectrum> &background,
          const hittable &world, int depth) {
  hit_record rec;

  // If we've exceeded the ray bounce limit, no more light
  // is gathered.
  if (depth <= 0)
    return make_shared<spectrum>(0.0);

  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, FLT_MAX, rec))
    return background;

  scatter_record srec;
  shared_ptr<spectrum> emitted =
      rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);

  if (!rec.mat_ptr->scatter(r, rec, srec))
    return emitted;

  if (srec.is_specular) {
    shared_ptr<spectrum> rs = ray_color(
        srec.specular_ray, background, world, depth - 1);

    return srec.attenuation->multip(rs);
  }

  ray scattered = ray(rec.p, srec.pdf_ptr->generate(),
                      r.time(), r.wavelength());
  auto pdf_val = srec.pdf_ptr->value(scattered.direction());
  auto scatter_pdf =
      rec.mat_ptr->scattering_pdf(r, rec, scattered);

  //
  shared_ptr<spectrum> recs = srec.attenuation->multip(
      make_shared<Real>(scatter_pdf));

  shared_ptr<spectrum> rcolor =
      ray_color(scattered, background, world, depth - 1);
  //
  recs = recs->multip(rcolor);

  auto pval = make_shared<Real>(pdf_val);

  //
  recs = recs->div(pval);

  return emitted->add(recs);
}
}
