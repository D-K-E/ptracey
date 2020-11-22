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
void ray_color(const ray &r,
               const shared_ptr<spectrum> background,
               const hittable &world, int depth,
               spectrum &output) {
  hit_record rec;
  auto wlength = r.wavelength();
  // add the wavelength if it is not already present
  output.insert(wlength, 0.0);

  // If we've exceeded the ray bounce limit, no more light
  // is gathered.
  if (depth <= 0) {
    // L_e + L_r = 0 = L_o
    return;
  }

  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, FLT_MAX, rec)) {
    // L_e + L_r = background = L_o
    auto back_lambda = background->evaluate(wlength);
    auto out_lambda2 = output.add(wlength, back_lambda);
    output.update(wlength, out_lambda2);
    return;
  }

  scatter_record srec;
  shared_ptr<spectrum> emitted =
      rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);
  auto emit_power = emitted->evaluate(wlength);

  if (!rec.mat_ptr->scatter(r, rec, srec)) {
    // L_e + 0 = L_o
    auto out_lambda2 = output.add(wlength, emit_power);
    output.update(wlength, out_lambda2);
    return;
  }

  if (srec.is_specular) {
    // f_r * L_i + 0 = L_r + L_e = 0
    ray_color(srec.specular_ray, background, world,
              depth - 1, output);
    // TODO what if the material changes the wavelength of
    // the ray ??
    auto f_r_power = srec.attenuation->evaluate(wlength);
    auto out_lambda2 = output.multip(wlength, f_r_power);
    output.update(wlength, out_lambda2);
    return;
  }

  // w_o
  ray scattered = ray(rec.p, srec.pdf_ptr->generate(),
                      r.time(), wlength);
  Real pdf_val = srec.pdf_ptr->value(scattered.direction());
  Real scatter_pdf =
      rec.mat_ptr->scattering_pdf(r, rec, scattered);

  // f_r
  spectrum f_r_lambda = srec.attenuation->evaluate(wlength);
  auto f_r_lambda2 =
      f_r_lambda.multip(wlength, scatter_pdf);

  // L_i
  ray_color(scattered, background, world, depth - 1,
            output);
  auto sc_wavel = scattered.wavelength();
  wlength = sc_wavel != wlength ? sc_wavel : wlength;
  spectrum out_lambda = output.evaluate(wlength);

  //
  auto out_lambda2 = output.multip(wlength, f_r_lambda2);
  auto out_lambda3 = out_lambda2.div(wlength, pdf_val);
  auto out_lambda4 = out_lambda3.add(wlength, emit_power);
  output.update(wlength, out_lambda4);

  return;
}
}
