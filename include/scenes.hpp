#pragma once
//
#include <aarect.hpp>
#include <box.hpp>
#include <bvh.hpp>
#include <camera.hpp>
#include <common.hpp>
#include <hittable.hpp>
#include <hittable_list.hpp>
#include <iostream>
#include <mediumc.hpp>
#include <mesh.hpp>
#include <moving_sphere.hpp>
#include <ray.hpp>
#include <spectrum.hpp>
#include <sphere.hpp>
#include <texture.hpp>
#include <utils.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {

shared_ptr<hittable>
model_random_material(matrix modelMat,
                      std::string modelpath) {
  int choice = random_int(0, 4);
  std::cerr << "choice " << choice << std::endl;
  shared_ptr<spectrum> rcolor =
      make_shared<spectrum>(spectrum::random());
  shared_ptr<material> mat;
  if (choice == 0) {
    mat = make_shared<lambertian>(rcolor);
    std::cerr << "model material :: lambertian"
              << std::endl;
  } else if (choice == 1) {
    mat = make_shared<metal>(rcolor, random_real());
    std::cerr << "model material :: metal" << std::endl;
  } else if (choice == 2) {
    mat = make_shared<dielectric>(random_real(0.001, 3.0));
    std::cerr << "model material :: dielectric"
              << std::endl;
  } else if (choice == 3) {
    mat = make_shared<diffuse_light>(rcolor);
    std::cerr << "model material :: diffuse_light"
              << std::endl;
  } else {
    shared_ptr<hittable> mod =
        make_shared<model>(modelpath, modelMat);
    return make_shared<constant_medium>(
        mod, random_real(0, 2.0),
        make_shared<solid_color>(
            make_shared<spectrum>(spectrum::random())));
    std::cerr << "model material :: constant medium"
              << std::endl;
  }
  return make_shared<model>(modelpath, modelMat, mat);
}

hittable_list cornell_box_restlife() {
  hittable_list objects;

  auto red = make_shared<lambertian>(
      make_shared<spectrum>(.65, .05, .05));
  auto white = make_shared<lambertian>(
      make_shared<spectrum>(.73, .73, .73));
  auto green = make_shared<lambertian>(
      make_shared<spectrum>(.12, .45, .15));
  auto light = make_shared<diffuse_light>(
      make_shared<spectrum>(15, 15, 15));

  objects.add(
      make_shared<yz_rect>(0, 555, 0, 555, 555, green));
  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
  objects.add(make_shared<flip_face>(make_shared<xz_rect>(
      213, 343, 227, 332, 554, light)));
  objects.add(
      make_shared<xz_rect>(0, 555, 0, 555, 555, white));
  objects.add(
      make_shared<xz_rect>(0, 555, 0, 555, 0, white));
  objects.add(
      make_shared<xy_rect>(0, 555, 0, 555, 555, white));

  shared_ptr<material> aluminum = make_shared<metal>(
      make_shared<spectrum>(0.8, 0.85, 0.88), 0.0);
  shared_ptr<hittable> box1 = make_shared<box>(
      point3(0, 0, 0), point3(165, 330, 165), aluminum);
  box1 = make_shared<rotate_y>(box1, 15);
  box1 = make_shared<translate>(box1, vec3(265, 0, 295));
  objects.add(box1);

  auto glass = make_shared<dielectric>(1.5);
  objects.add(
      make_shared<sphere>(point3(190, 90, 190), 90, glass));

  return objects;
}
hittable_list random_scene() {
  hittable_list world;

  auto checker = make_shared<checker_texture>(
      make_shared<spectrum>(0.2, 0.3, 0.1),
      make_shared<spectrum>(0.9, 0.9, 0.9));

  world.add(make_shared<sphere>(
      point3(0, -1000, 0), 1000,
      make_shared<lambertian>(checker)));

  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      auto choose_mat = random_real();
      point3 center(a + 0.9 * random_real(), 0.2,
                    b + 0.9 * random_real());

      if ((center - vec3(4, 0.2, 0)).length() > 0.9) {
        shared_ptr<material> sphere_material;

        if (choose_mat < 0.8) {
          // diffuse
          shared_ptr<spectrum> randcol1 =
              make_shared<spectrum>(color::random());
          shared_ptr<spectrum> randcol2 =
              make_shared<spectrum>(color::random());
          auto albedo = randcol1->multip(randcol2);
          sphere_material = make_shared<lambertian>(albedo);
          auto center2 =
              center + vec3(0, random_real(0, .5), 0);
          world.add(make_shared<moving_sphere>(
              center, center2, 0.0, 1.0, 0.2,
              sphere_material));
        } else if (choose_mat < 0.95) {
          // metal
          auto albedo = make_shared<spectrum>(
              spectrum::random(0.5, 1));
          auto fuzz = random_real(0, 0.5);
          sphere_material =
              make_shared<metal>(albedo, fuzz);
          world.add(make_shared<sphere>(center, 0.2,
                                        sphere_material));
        } else {
          // glass
          sphere_material = make_shared<dielectric>(1.5);
          world.add(make_shared<sphere>(center, 0.2,
                                        sphere_material));
        }
      }
    }
  }

  auto material1 = make_shared<dielectric>(1.5);
  world.add(
      make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = make_shared<lambertian>(
      make_shared<spectrum>(0.4, 0.2, 0.1));
  world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0,
                                material2));

  auto material3 = make_shared<metal>(
      make_shared<spectrum>(0.7, 0.6, 0.5), 0.0);
  world.add(
      make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

  return hittable_list(
      make_shared<bvh_node>(world, 0.0, 1.0));
}
hittable_list two_spheres() {
  hittable_list objects;

  auto checker = make_shared<checker_texture>(
      make_shared<spectrum>(0.2, 0.3, 0.1),
      make_shared<spectrum>(0.9, 0.9, 0.9));

  objects.add(make_shared<sphere>(
      point3(0, -10, 0), 10,
      make_shared<lambertian>(checker)));
  objects.add(make_shared<sphere>(
      point3(0, 10, 0), 10,
      make_shared<lambertian>(checker)));

  return objects;
}
hittable_list two_perlin_spheres() {
  hittable_list objects;

  auto pertext = make_shared<noise_texture>(
      4, make_shared<spectrum>(1.0));
  objects.add(make_shared<sphere>(
      point3(0, -1000, 0), 1000,
      make_shared<lambertian>(pertext)));
  objects.add(make_shared<sphere>(
      point3(0, 2, 0), 2,
      make_shared<lambertian>(pertext)));

  return objects;
}
hittable_list earth() {
  auto earth_texture =
      make_shared<image_texture>("media/earthmap.jpg");
  auto earth_surface =
      make_shared<lambertian>(earth_texture);
  auto globe = make_shared<sphere>(point3(0, 0, 0), 2,
                                   earth_surface);

  return hittable_list(globe);
}
hittable_list simple_light() {
  hittable_list objects;

  auto pertext = make_shared<noise_texture>(
      4, make_shared<spectrum>(1.0));
  objects.add(make_shared<sphere>(
      point3(0, -1000, 0), 1000,
      make_shared<lambertian>(pertext)));
  objects.add(make_shared<sphere>(
      point3(0, 2, 0), 2,
      make_shared<lambertian>(pertext)));

  auto difflight = make_shared<diffuse_light>(
      make_shared<spectrum>(4, 4, 4));
  objects.add(
      make_shared<sphere>(point3(0, 7, 0), 2, difflight));
  objects.add(
      make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

  return objects;
}
hittable_list cornell_box() {
  hittable_list objects;

  auto red = make_shared<lambertian>(
      make_shared<spectrum>(.65, .05, .05));
  auto white = make_shared<lambertian>(
      make_shared<spectrum>(.73, .73, .73));
  auto green = make_shared<lambertian>(
      make_shared<spectrum>(.12, .45, .15));
  auto light = make_shared<diffuse_light>(
      make_shared<spectrum>(15.0, 15.0, 15.0));

  objects.add(make_shared<yz_rect>(0.0, 555.0, 0.0, 555.0,
                                   555.0, green));
  objects.add(make_shared<yz_rect>(0.0, 555.0, 0.0, 555.0,
                                   0.0, red));
  objects.add(make_shared<flip_face>(make_shared<xz_rect>(
      213, 343, 227, 332, 554, light)));
  objects.add(
      make_shared<xz_rect>(0, 555, 0, 555, 555, white));
  objects.add(
      make_shared<xz_rect>(0, 555, 0, 555, 0, white));
  objects.add(
      make_shared<xy_rect>(0, 555, 0, 555, 555, white));

  shared_ptr<hittable> box1 = make_shared<box>(
      point3(0, 0, 0), point3(165, 330, 165), white);
  box1 = make_shared<rotate_y>(box1, 15);
  box1 = make_shared<translate>(box1, vec3(265, 0, 295));
  objects.add(box1);

  shared_ptr<hittable> box2 = make_shared<box>(
      point3(0, 0, 0), point3(165, 165, 165), white);
  box2 = make_shared<rotate_y>(box2, -18);
  box2 = make_shared<translate>(box2, vec3(130, 0, 65));
  objects.add(box2);

  return objects;
}
hittable_list cornell_smoke() {
  hittable_list objects;

  auto red = make_shared<lambertian>(
      make_shared<spectrum>(.65, .05, .05));
  auto white = make_shared<lambertian>(
      make_shared<spectrum>(.73, .73, .73));
  auto green = make_shared<lambertian>(
      make_shared<spectrum>(.12, .45, .15));
  auto light = make_shared<diffuse_light>(
      make_shared<spectrum>(7, 7, 7));

  objects.add(
      make_shared<yz_rect>(0, 555, 0, 555, 555, green));
  objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
  objects.add(make_shared<flip_face>(make_shared<xz_rect>(
      113, 443, 127, 432, 554, light)));
  objects.add(
      make_shared<xz_rect>(0, 555, 0, 555, 555, white));
  objects.add(
      make_shared<xz_rect>(0, 555, 0, 555, 0, white));
  objects.add(
      make_shared<xy_rect>(0, 555, 0, 555, 555, white));

  shared_ptr<hittable> box1 = make_shared<box>(
      point3(0, 0, 0), point3(165, 330, 165), white);
  box1 = make_shared<rotate_y>(box1, 15);
  box1 = make_shared<translate>(box1, vec3(265, 0, 295));

  shared_ptr<hittable> box2 = make_shared<box>(
      point3(0, 0, 0), point3(165, 165, 165), white);
  box2 = make_shared<rotate_y>(box2, -18);
  box2 = make_shared<translate>(box2, vec3(130, 0, 65));

  objects.add(make_shared<constant_medium>(
      box1, 0.01, make_shared<spectrum>(0, 0, 0)));
  objects.add(make_shared<constant_medium>(
      box2, 0.01, make_shared<spectrum>(1, 1, 1)));

  return objects;
}
hittable_list final_scene_nextweek() {
  hittable_list boxes1;
  auto ground = make_shared<lambertian>(
      make_shared<spectrum>(0.48, 0.83, 0.53));

  const int boxes_per_side = 20;
  for (int i = 0; i < boxes_per_side; i++) {
    for (int j = 0; j < boxes_per_side; j++) {
      auto w = 100.0;
      auto x0 = -1000.0 + i * w;
      auto z0 = -1000.0 + j * w;
      auto y0 = 0.0;
      auto x1 = x0 + w;
      auto y1 = random_real(1, 101);
      auto z1 = z0 + w;

      boxes1.add(make_shared<box>(
          point3(x0, y0, z0), point3(x1, y1, z1), ground));
    }
  }

  hittable_list objects;

  objects.add(make_shared<bvh_node>(boxes1, 0, 1));

  auto light = make_shared<diffuse_light>(
      make_shared<spectrum>(7, 7, 7));
  objects.add(
      make_shared<xz_rect>(123, 423, 147, 412, 554, light));

  auto center1 = point3(400, 400, 200);
  auto center2 = center1 + vec3(30, 0, 0);
  auto moving_sphere_material = make_shared<lambertian>(
      make_shared<spectrum>(0.7, 0.3, 0.1));
  objects.add(make_shared<moving_sphere>(
      center1, center2, 0, 1, 50, moving_sphere_material));

  objects.add(
      make_shared<sphere>(point3(260, 150, 45), 50,
                          make_shared<dielectric>(1.5)));
  objects.add(make_shared<sphere>(
      point3(0, 150, 145), 50,
      make_shared<metal>(
          make_shared<spectrum>(0.8, 0.8, 0.9), 1.0)));

  auto boundary =
      make_shared<sphere>(point3(360, 150, 145), 70,
                          make_shared<dielectric>(1.5));
  objects.add(boundary);
  objects.add(make_shared<constant_medium>(
      boundary, 0.2, make_shared<spectrum>(0.2, 0.4, 0.9)));
  boundary = make_shared<sphere>(
      point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
  objects.add(make_shared<constant_medium>(
      boundary, .0001, make_shared<spectrum>(1, 1, 1)));

  auto emat = make_shared<lambertian>(
      make_shared<image_texture>("media/earthmap.jpg"));
  objects.add(make_shared<sphere>(point3(400, 200, 400),
                                  100, emat));
  auto pertext = make_shared<noise_texture>(
      0.1, make_shared<spectrum>(1.0));
  objects.add(make_shared<sphere>(
      point3(220, 280, 300), 80,
      make_shared<lambertian>(pertext)));

  hittable_list boxes2;
  auto white = make_shared<lambertian>(
      make_shared<spectrum>(.73, .73, .73));
  int ns = 1000;
  for (int j = 0; j < ns; j++) {
    boxes2.add(make_shared<sphere>(point3::random(0, 165),
                                   10, white));
  }

  objects.add(make_shared<translate>(
      make_shared<rotate_y>(
          make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
      vec3(-100, 270, 395)));

  return objects;
}
hittable_list model_cat() {
  matrix modelMat =
      scale_translate(point3(150, 167, 45), vec3(55.0));
  std::string modelpath = "media/models/kedi.obj";

  shared_ptr<hittable> cat =
      model_random_material(modelMat, modelpath);

  hittable_list objects;

  auto red = make_shared<lambertian>(
      make_shared<spectrum>(.65, .05, .05));
  auto white = make_shared<lambertian>(
      make_shared<spectrum>(.73, .73, .73));
  auto green = make_shared<lambertian>(
      make_shared<spectrum>(.12, .45, .15));
  auto light = make_shared<diffuse_light>(
      make_shared<spectrum>(15.0, 15.0, 15.0));

  objects.add(make_shared<yz_rect>(0.0, 555.0, 0.0, 555.0,
                                   555.0, green));
  objects.add(make_shared<yz_rect>(0.0, 555.0, 0.0, 555.0,
                                   0.0, red));
  objects.add(make_shared<flip_face>(make_shared<xz_rect>(
      213, 343, 227, 332, 554, light)));
  objects.add(
      make_shared<xz_rect>(0, 555, 0, 555, 555, white));
  objects.add(
      make_shared<xz_rect>(0, 555, 0, 555, 0, white));
  objects.add(
      make_shared<xy_rect>(0, 555, 0, 555, 555, white));

  shared_ptr<hittable> box1 = make_shared<box>(
      point3(0, 0, 0), point3(165, 330, 165), white);
  box1 = make_shared<rotate_y>(box1, 15);
  box1 = make_shared<translate>(box1, vec3(265, 0, 295));
  objects.add(box1);

  shared_ptr<hittable> box2 = make_shared<box>(
      point3(0, 0, 0), point3(165, 165, 165), white);
  box2 = make_shared<rotate_y>(box2, -18);
  box2 = make_shared<translate>(box2, point3(130, 0, 65));

  objects.add(box2);
  objects.add(cat);

  return objects;
}
hittable_list model_test2() {
  matrix modelMat =
      scale_translate(point3(130, 167, 65), vec3(5.0));
  std::string modelpath =
      "media/models/redgranite/stele.obj";

  hittable_list objects;

  auto red = make_shared<lambertian>(
      make_shared<spectrum>(.65, .05, .05));
  auto white = make_shared<lambertian>(
      make_shared<spectrum>(.73, .73, .73));
  auto green = make_shared<lambertian>(
      make_shared<spectrum>(.12, .45, .15));
  auto light = make_shared<diffuse_light>(
      make_shared<spectrum>(15.0, 15.0, 15.0));

  shared_ptr<hittable> cat =
      make_shared<model>(modelpath, modelMat, light);

  // objects.add(make_shared<yz_rect>(0.0, 555.0, 0.0,
  // 555.0,
  //                                 555.0, green));
  // objects.add(make_shared<yz_rect>(0.0, 555.0, 0.0,
  // 555.0,
  //                                 0.0, red));
  // objects.add(make_shared<flip_face>(make_shared<xz_rect>(
  //    213, 343, 227, 332, 554, light)));
  // objects.add(
  //    make_shared<xz_rect>(0, 555, 0, 555, 555, white));
  // objects.add(
  //    make_shared<xz_rect>(0, 555, 0, 555, 0, white));
  // objects.add(
  //    make_shared<xy_rect>(0, 555, 0, 555, 555, white));

  // shared_ptr<hittable> box1 = make_shared<box>(
  //    point3(0, 0, 0), point3(165, 330, 165), white);
  // box1 = make_shared<rotate_y>(box1, 15);
  // box1 = make_shared<translate>(box1, vec3(265, 0, 295));
  // objects.add(box1);

  // shared_ptr<hittable> box2 = make_shared<box>(
  //    point3(0, 0, 0), point3(165, 165, 165), white);
  // box2 = make_shared<rotate_y>(box2, -18);
  // box2 = make_shared<translate>(box2, point3(130, 0,
  // 65));

  // objects.add(box2);
  objects.add(cat);

  return objects;
}

void choose_scene(int choice, camera &cam,
                  hittable_list &world,
                  int samples_per_pixel, float aspect_ratio,
                  int &image_width, int &image_height,
                  shared_ptr<spectrum> &background) {
  //
  point3 lookfrom;
  point3 lookat;
  auto vfov = 40.0;
  auto aperture = 0.0;
  background = make_shared<spectrum>(0.0);

  switch (choice) {
  case 1: {
    world = random_scene();
    color b_rgb(0.70, 0.80, 1.00);
    background = make_shared<spectrum>(0.70, 0.80, 1.00);
    lookfrom = point3(13, 2, 3);
    lookat = point3(0, 0, 0);
    vfov = 20.0;
    aperture = 0.1;
    break;
  }

  case 2: {
    world = two_spheres();
    color b_rgb(0.70, 0.80, 1.00);
    // background = sampled_spectrum::fromRgb(
    //    b_rgb, SpectrumType::Reflectance);
    background = make_shared<spectrum>(0.70, 0.80, 1.00);

    lookfrom = point3(13, 2, 3);
    lookat = point3(0, 0, 0);
    vfov = 20.0;
    break;
  }

  case 3: {
    world = two_perlin_spheres();
    color b_rgb(0.70, 0.80, 1.00);
    // background = sampled_spectrum::fromRgb(
    //    b_rgb, SpectrumType::Reflectance);
    background = make_shared<spectrum>(0.70, 0.80, 1.00);

    lookfrom = point3(13, 2, 3);
    lookat = point3(0, 0, 0);
    vfov = 20.0;
    break;
  }

  case 4: {
    world = earth();
    color b_rgb(0.70, 0.80, 1.00);
    // background = sampled_spectrum::fromRgb(
    //    b_rgb, SpectrumType::Reflectance);
    background = make_shared<spectrum>(0.70, 0.80, 1.00);

    lookfrom = point3(0, 0, 12);
    lookat = point3(0, 0, 0);
    vfov = 20.0;
    break;
  }

  case 5: {
    world = simple_light();
    samples_per_pixel = 400;
    lookfrom = point3(26, 3, 6);
    lookat = point3(0, 2, 0);
    vfov = 20.0;
    break;
  }

  case 6: {
    world = cornell_box();
    aspect_ratio = 1.0;
    image_width = 320;
    samples_per_pixel = 200;
    lookfrom = point3(278, 278, -800);
    lookat = point3(278, 278, 0);
    break;
  }

  case 7: {
    world = cornell_smoke();
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 200;
    lookfrom = point3(278, 278, -800);
    lookat = point3(278, 278, 0);
    break;
  }

  case 8: {
    world = cornell_box_restlife();
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 200;
    lookfrom = point3(278, 278, -800);
    lookat = point3(278, 278, 0);
    break;
  }

  case 9: {
    hittable_list world2 = final_scene_nextweek();
    shared_ptr<hittable> wh =
        make_shared<bvh_node>(world2, 0.0, 1.0);
    world = hittable_list(wh);
    aspect_ratio = 1.0;
    image_width = 600;
    samples_per_pixel = 100;
    lookfrom = point3(478, 278, -600);
    lookat = point3(278, 278, 0);
    break;
  }

  case 10: {
    world = model_cat();
    aspect_ratio = 1.0;
    image_width = 400;
    samples_per_pixel = 10000;
    lookfrom = point3(278, 278, -800);
    lookat = point3(278, 278, 0);
    break;
  }
  case 11: {
    world = model_test2();
    aspect_ratio = 1.0;
    image_width = 300;
    samples_per_pixel = 50;
    lookfrom = point3(278, 278, -800);
    lookat = point3(278, 278, 0);
    break;
  }
  }

  // Camera

  const vec3 vup(0, 1, 0);
  const auto dist_to_focus = 10.0;
  image_height =
      static_cast<int>(image_width / aspect_ratio);

  cam = camera(lookfrom, lookat, vup, vfov, aspect_ratio,
               aperture, dist_to_focus, 0.0, 1.0);
}
}
