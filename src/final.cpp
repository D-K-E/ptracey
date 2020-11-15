//
#include <camera.hpp>
#include <color.hpp>
#include <common.hpp>
#include <hittable_list.hpp>
#include <ray.hpp>
#include <scenes.hpp>
#include <spectrum.hpp>
#include <utils.hpp>
#include <vec3.hpp>

using immat =
    std::vector<std::vector<shared_ptr<spectrum>>>;

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

    //
    return srec.attenuation->multip(rs);
  }

  ray scattered =
      ray(rec.p, srec.pdf_ptr->generate(), r.time());
  auto pdf_val = srec.pdf_ptr->value(scattered.direction());
  shared_ptr<spectrum> recs =
      make_shared<spectrum>(srec.attenuation->multip(
          rec.mat_ptr->scattering_pdf(r, rec, scattered)));
  recs = recs->multip(
      ray_color(scattered, background, world, depth - 1));
  recs = make_shared<spectrum>(recs->div(pdf_val));

  return emitted->add(recs);
}

struct InnerParams {
  camera cam;
  int imwidth;
  int imheight;
  int psample;
  int mdepth;
  int startx;
  int endx;
  hittable_list scene;
  shared_ptr<spectrum> background;
  InnerParams(int sx, int ex, const camera &c, int imw,
              int imh, int ps, int d,
              const hittable_list &hs,
              const shared_ptr<spectrum> &b)
      : startx(sx), endx(ex), cam(c), imwidth(imw),
        imheight(imh), psample(ps), mdepth(d), scene(hs),
        background(b) {}
};

struct InnerRet {
  immat img;
  int startx;
  int endx;
};

InnerRet innerLoop(InnerParams params) {
  // inner loop for write_color
  camera cam = params.cam;
  int imwidth = params.imwidth;
  int imheight = params.imheight;
  int psample = params.psample;
  int mdepth = params.mdepth;
  int startx = params.startx;
  int endx = params.endx;
  int xrange = endx - startx;
  shared_ptr<spectrum> background = params.background;
  hittable_list scene = params.scene;
  immat imv;
  imv.resize(xrange);
  for (int i = 0; i < xrange; i++) {
    imv[i].resize(imheight, make_shared<spectrum>(0.0));
  }

  for (int a = 0; a < xrange; a++) {
    for (int j = 0; j < imheight; j++) {
      int i = a + startx;
      //
      shared_ptr<spectrum> rcolor =
          make_shared<spectrum>(0.0);
      for (int k = 0; k < psample; k++) {
        Real t = Real(i + random_double()) / (imwidth - 1);
        Real s = Real(j + random_double()) / (imheight - 1);
        ray r = cam.get_ray(t, s);
        shared_ptr<spectrum> rcol =
            ray_color(r, background, scene, mdepth);
        rcolor = rcolor->add(rcol);
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

void fill_imvec(InnerRet ret, immat &imvec,
                int image_height) {
  immat img = ret.img;
  int startx = ret.startx;
  int endx = ret.endx;
  int xrange = endx - startx;
  for (int i = 0; i < xrange; i++) {
    for (int j = 0; j < image_height; j++) {
      imvec[i + startx][j] = img[i][j];
    }
  }
}

int main() {
  auto start = std::chrono::high_resolution_clock::now();

  // Image
  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 400;
  int samples_per_pixel = 100;
  int max_depth = 200;
  immat imvec;

  // World

  hittable_list world;
  int choice = 2;
  camera cam;
  int image_height;
  shared_ptr<spectrum> background;
  choose_scene(choice, cam, world, samples_per_pixel,
               aspect_ratio, image_width, image_height,
               background);
  imvec.resize(image_width);
  for (int i = 0; i < image_width; i++) {
    imvec[i].resize(image_height,
                    make_shared<spectrum>(0.0));
  }
  int wslicelen = int(image_width / THREAD_NB);

  // Render

  std::cout << "P3\n"
            << image_width << ' ' << image_height
            << "\n255\n";

  std::vector<std::future<InnerRet>> futures(THREAD_NB);

  // resim yazim
  for (int t = 0; t < THREAD_NB; t++) {
    int startx = wslicelen * t;
    int endx = fmin(startx + wslicelen, image_width);
    InnerParams params(startx, endx, cam, image_width,
                       image_height, samples_per_pixel,
                       max_depth, world, background);
    if (THREAD_NB > 1)
      futures[t] = std::async(&innerLoop, params);
  }
  if (THREAD_NB > 1) {
    for (auto &f : futures) {
      InnerRet ret = f.get();
      fill_imvec(ret, imvec, image_height);
    }
  } else {
    InnerParams params(0, wslicelen, cam, image_width,
                       image_height, samples_per_pixel,
                       max_depth, world, background);

    InnerRet ret = innerLoop(params);
    fill_imvec(ret, imvec, image_height);
  }
  for (int j = image_height - 1; j >= 0; j -= 1) {
    std::cerr << "\rKalan Tarama Çizgisi:" << ' ' << j
              << ' ' << std::flush;
    for (int i = 0; i < image_width; i += 1) {
      write_color(std::cout, imvec[i][j],
                  samples_per_pixel);
    }
  }
  auto stop = std::chrono::high_resolution_clock::now();
  std::chrono::duration<Real> elapsed = stop - start;

  std::cerr << "duration: " << elapsed.count() << std::endl;

  std::cerr << "\nDone.\n";
}
