//
#include <camera.hpp>
#include <colorio.hpp>
#include <common.hpp>
#include <hittable_list.hpp>
#include <ray.hpp>
#include <render.hpp>
#include <scenes.hpp>
#include <spectrum.hpp>
#include <utils.hpp>
#include <vec3.hpp>

using namespace ptracey;
namespace ptracey {
using immat = std::vector<std::vector<spectrum>>;

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
    imv[i].resize(imheight, spectrum(0.0));
  }

  for (int a = 0; a < xrange; a++) {
    for (int j = 0; j < imheight; j++) {
      int i = a + startx;
      //
      spectrum rcolor = spectrum(0.0);
      for (int k = 0; k < psample; k++) {
        Real t = Real(i + random_real()) / (imwidth - 1);
        Real s = Real(j + random_real()) / (imheight - 1);
        unsigned int wavelength =
            static_cast<unsigned int>(random_int(
                VISIBLE_LAMBDA_START, VISIBLE_LAMBDA_END));
        ray r = cam.get_ray(t, s, wavelength);
        spectrum r_color = spectrum(0.0);
        ray_color(r, background, scene, mdepth, r_color);
        rcolor += r_color;
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

extern "C" int main(int ac, char **av) {
  auto start = std::chrono::high_resolution_clock::now();

  // Image
  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 400;
  int samples_per_pixel = 10;
  int max_depth = 10;
  immat imvec;

  // World

  hittable_list world;
  int choice = 12;
  camera cam;
  int image_height;
  shared_ptr<spectrum> background;
  choose_scene(choice, cam, world, samples_per_pixel,
               aspect_ratio, image_width, image_height,
               background);
  imvec.resize(image_width);
  for (int i = 0; i < image_width; i++) {
    imvec[i].resize(image_height, spectrum(0.0));
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
  return 0;
}
}
