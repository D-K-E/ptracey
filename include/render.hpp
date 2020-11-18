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
void ray_color(const ray &r,
               const shared_ptr<spectrum> background,
               const hittable &world, int depth,
               spectrum &output) {
  hit_record rec;
  // add the wavelength if it is not already present
  output.insert(r.wavelength(), 0.0);

  // If we've exceeded the ray bounce limit, no more light
  // is gathered.
  if (depth <= 0) {
    // L_e + L_r = 0 = L_o
    return;
  }

  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, FLT_MAX, rec)) {
    // L_e + L_r = background = L_o
    auto out_lambda = output.evaluate(r.wavelength());
    auto back_lambda = background->evaluate(r.wavelength());
    out_lambda += back_lambda;
    output.update(r.wavelength(), out_lambda);
    return;
  }

  scatter_record srec;
  shared_ptr<spectrum> emitted =
      rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
  auto emit_power = emitted->evaluate(r.wavelength());

  if (!rec.mat_ptr->scatter(r, rec, srec)) {
    // L_e + 0 = L_o
    auto out_lambda = output.evaluate(r.wavelength());
    out_lambda += emit_power;
    output.update(r.wavelength(), out_lambda);
    std::cerr << "emit " << std::endl;
    return;
  }

  if (srec.is_specular) {
    // f_r * L_i + 0 = L_r + L_e = 0
    ray_color(srec.specular_ray, background, world,
              depth - 1, output);
    // TODO what if the material changes the wavelength of
    // the ray ??
    auto f_r_power =
        srec.attenuation->evaluate(r.wavelength());
    auto out_lambda = output.evaluate(r.wavelength());
    out_lambda = out_lambda * f_r_power;
    output.update(r.wavelength(), out_lambda);
    return;
  }

  // w_o
  ray scattered = ray(rec.p, srec.pdf_ptr->generate(),
                      r.time(), r.wavelength());
  auto pdf_val = srec.pdf_ptr->value(scattered.direction());
  auto scatter_pdf =
      rec.mat_ptr->scattering_pdf(r, rec, scattered);

  // f_r
  auto f_r_lambda =
      srec.attenuation->evaluate(r.wavelength());
  f_r_lambda = f_r_lambda * scatter_pdf;

  // L_i
  ray_color(scattered, background, world, depth - 1,
            output);
  auto out_lambda = output.evaluate(scattered.wavelength());
  //
  out_lambda = out_lambda * f_r_lambda;
  out_lambda = out_lambda / pdf_val;
  out_lambda = out_lambda + emit_power;
  output.update(scattered.wavelength(), out_lambda);

  return;
}
}
