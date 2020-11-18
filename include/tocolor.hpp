#pragma once
//

#include <common.hpp>
#include <spd.hpp>
#include <spectrum.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

color to_color(const sampled_spectrum &s_lambda) {
  return xyz2rgb(to_xyz(s_lambda.spect));
}
color to_color(const sampled_spectrum &s_lambda,
               const sampled_spectrum &r_lambda) {
  return xyz2rgb(to_xyz(s_lambda.spect, r_lambda.spect));
}
color to_color(const color &rgb) { return rgb; }
color to_color(const color &s_lambda,
               const color &r_lambda) {
  return r_lambda;
}
color to_color(const shared_ptr<spectrum> &spec_ptr) {
  return to_color(*spec_ptr);
}
color to_color(const shared_ptr<spectrum> &spec_ptr,
               const shared_ptr<spectrum> &ref_ptr) {
  return to_color(*spec_ptr, *ref_ptr);
}
}
