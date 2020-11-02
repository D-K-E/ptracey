#include <utils/commons.hpp> // done
//
#include <hittable/hittables.hpp> // done
//
#include <utils/camera.hpp> // done
#include <utils/color.hpp>  // done
//
#include <hittable/box.hpp> // done
#include <material/pdf.hpp>

//#include <bvh.hpp>      // done

//#include <mediumc.hpp>  // done

//
#include <filesystem>
#include <hittable/sphere.hpp> // done
#include <iostream>
//
#include <thread>

#include <future>
#define THREAD_NB 6
using immat = std::vector<std::vector<color>>;

color ray_color(const Ray &r, const color &background,
                const HittableList &scene, shared_ptr<Hittable> lights,
                int depth) {
  // carpan isini renklendirir
  HitRecord record;
  if (depth <= 0) {
    // final case
    return color(0);
  }
  if (!scene.hit(r, 0.001, INF, record)) {
    return background;
  }
  // recursive case
  color emittedColor =
      record.mat_ptr->emitted(r, record, record.u, record.v, record.point);
  ScatterRecord srec;
  if (!record.mat_ptr->scatter(r, record, srec)) {
    return emittedColor;
  }
  if (srec.is_specular) {
    return srec.attenuation *
           ray_color(srec.r_out, background, scene, lights, depth - 1);
  }

  // trying mixed pdf

  shared_ptr<Pdf> hpdf = make_shared<HittablePdf>(lights, record.point);

  MixturePdf pdf(hpdf, srec.pdf_ptr);

  vec3 generated_dir = pdf.generate();
  Ray r_out = Ray(record.point, generated_dir, r.time());
  double pdf_val = pdf.value(r_out.dir());
  double s_pdf = record.mat_ptr->scattering_pdf(r, record, r_out);

  // bidirectional surface scattering distribution function
  // rendering equation = L^e + L^r
  return emittedColor +
         (srec.attenuation * s_pdf *
          ray_color(r_out, background, scene, lights, depth - 1) / pdf_val);
}

HittableList cornell_box() {

  HittableList scene;

  // ---------- materials -----------------
  auto red = make_shared<Lambertian>(make_shared<SolidColor>(0.65, 0.05, 0.05));
  auto white =
      make_shared<Lambertian>(make_shared<SolidColor>(0.88, 0.88, 0.88));
  auto green =
      make_shared<Lambertian>(make_shared<SolidColor>(0.18, 0.78, 0.18));
  auto light =
      make_shared<DiffuseLight>(make_shared<SolidColor>(15.0, 15.0, 15.0));

  // --------- objects -------------------
  scene.add(
      make_shared<FlipFace>(make_shared<YZRect>(0, 555, 0, 555, 555, green)));
  scene.add(make_shared<YZRect>(0, 555, 0, 555, 0, red));
  scene.add(make_shared<XZRect>(213, 343, 227, 332, 554, light));
  scene.add(
      make_shared<FlipFace>(make_shared<XZRect>(0, 555, 0, 555, 0, white)));
  scene.add(make_shared<XZRect>(0, 555, 0, 555, 555, white));
  scene.add(
      make_shared<FlipFace>(make_shared<XYRect>(0, 555, 0, 555, 555, white)));

  // --------- make boxes --------------
  shared_ptr<Material> aluminum =
      make_shared<Metal>(color(0.8, 0.85, 0.88), 0.0);
  shared_ptr<Hittable> box1 =
      make_shared<Box>(point3(0, 0, 0), point3(165, 330, 165), white);
  box1 = make_shared<RotateY>(box1, 15);
  box1 = make_shared<Translate>(box1, vec3(265, 0, 295));
  scene.add(box1);

  shared_ptr<Material> glass = make_shared<Dielectric>(1.5);
  shared_ptr<Hittable> glass_sphere =
      make_shared<Sphere>(point3(190, 90, 190), 90, glass);

  scene.add(glass_sphere);

  // add other objects etc

  return scene;
}

struct InnerParams {
  TimeRayCamera camera;
  int imwidth;
  int imheight;
  int psample;
  int mdepth;
  int startx;
  int endx;
  HittableList scene;
};

InnerParams makeInner(double startx, double endx, const TimeRayCamera &camera,
                      double imwidth, double imheight, double pixel_sample,
                      double mdepth, const HittableList &scene) {
  InnerParams params;
  params.startx = startx;
  params.endx = endx;
  params.camera = camera;
  params.imwidth = imwidth;
  params.imheight = imheight;
  params.psample = pixel_sample;
  params.mdepth = mdepth;
  params.scene = scene;
  return params;
}

struct InnerRet {
  immat img;
  int startx;
  int endx;
};

InnerRet innerLoop(InnerParams params) {
  // inner loop for write_color
  TimeRayCamera camera = params.camera;
  int imwidth = params.imwidth;
  int imheight = params.imheight;
  int psample = params.psample;
  int mdepth = params.mdepth;
  int startx = params.startx;
  int endx = params.endx;
  int xrange = endx - startx;
  HittableList scene = params.scene;
  immat imv(xrange, std::vector<color>(imheight, color(0)));
  color background(0);
  shared_ptr<HittableList> lights = make_shared<HittableList>();
  lights->add(
      make_shared<XZRect>(213, 343, 227, 332, 554, shared_ptr<Material>()));
  lights->add(
      make_shared<Sphere>(point3(190, 90, 190), 90, shared_ptr<Material>()));

  for (int a = 0; a < xrange; a++) {
    for (int j = 0; j < imheight; j++) {
      int i = a + startx;
      //
      color rcolor(0.0, 0.0, 0.0);
      for (int k = 0; k < psample; k++) {
        double t = double(i + random_double()) / (imwidth - 1);
        double s = double(j + random_double()) / (imheight - 1);
        Ray r = camera.get_ray(t, s);
        rcolor += ray_color(r, background, scene, lights, mdepth);
      }
      imv[a][j] = rcolor;
    }
  }
  InnerRet ret;
  ret.img = imv;
  ret.startx = startx;
  ret.endx = endx;
  return ret;
}

void joinRet2Imv(InnerRet ret, immat &imvec, double imheight) {
  // join returned promise to image
  immat img = ret.img;
  int startx = ret.startx;
  int endx = ret.endx;
  int xrange = endx - startx;
  for (int i = 0; i < xrange; i++) {
    for (int j = 0; j < imheight; j++) {
      imvec[i + startx][j] = img[i][j];
    }
  }
}

int main(void) {
  // resmin yazma fonksiyonu
  double aspect_ratio = 16.0 / 9.0;
  const int imwidth = 320;
  const int imheight = static_cast<int>(imwidth / aspect_ratio);
  int pixel_sample = 500;
  int mdepth = 50;
  int wslicelen = int(imwidth / THREAD_NB);
  immat imvec(imwidth, std::vector<color>(imheight, color(0)));

  // ppm için gerekli olanlar
  std::cout << "P3" << std::endl;
  std::cout << imwidth << ' ' << imheight << std::endl;
  std::cout << "255" << std::endl;

  // konacak objelerin deklarasyonu
  HittableList scene = cornell_box();

  // kamera
  vec3 vup(0, 1, 0);
  double dist_to_focus = 10.0;
  double aperature = 0.1;
  TimeRayCamera camera(point3(278, 278, -800), point3(278, 278, 0), vup, 40,
                       aspect_ratio, aperature, dist_to_focus, 0.0, 1.0);

  std::vector<std::future<InnerRet>> futures(THREAD_NB);
  // resim yazim
  for (int t = 0; t < THREAD_NB; t++) {
    int startx = wslicelen * t;
    int endx = fmin(startx + wslicelen, imwidth);
    InnerParams params = makeInner(startx, endx, camera, imwidth, imheight,
                                   pixel_sample, mdepth, scene);
    futures[t] = std::async(&innerLoop, params);
  }
  for (auto &f : futures) {
    InnerRet ret = f.get();
    joinRet2Imv(ret, imvec, imheight);
  }
  for (int j = imheight - 1; j >= 0; j -= 1) {
    std::cerr << "\rKalan Tarama Çizgisi:" << ' ' << j << ' ' << std::flush;
    for (int i = 0; i < imwidth; i += 1) {
      write_color(std::cout, imvec[i][j], pixel_sample);
    }
  }
}
