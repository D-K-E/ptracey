#include <camera.hpp>
#include <color.hpp>
#include <external.hpp>
#include <hittable_list.hpp>
#include <material.hpp>
#include <ray.hpp>
#include <scenes.hpp>
#include <utils.hpp>
#include <vec3.hpp>

using immat = std::vector<std::vector<color>>;

color ray_color(const ray &r, const color &background,
                const hittable &world, int depth) {
  hit_record rec;

  // If we've exceeded the ray bounce limit, no more light
  // is gathered.
  if (depth <= 0)
    return color(0);

  // If the ray hits nothing, return the background color.
  if (!world.hit(r, 0.001, FLT_MAX, rec))
    return background;

  scatter_record srec;
  color emitted =
      rec.mat_ptr->emitted(r, rec, rec.u, rec.v, rec.p);

  if (!rec.mat_ptr->scatter(r, rec, srec))
    return emitted;

  if (srec.is_specular) {
    return srec.attenuation * ray_color(srec.specular_ray,
                                        background, world,
                                        depth - 1);
  }

  ray scattered =
      ray(rec.p, srec.pdf_ptr->generate(), r.time());
  auto pdf_val = srec.pdf_ptr->value(scattered.direction());

  return emitted +
         srec.attenuation *
             rec.mat_ptr->scattering_pdf(r, rec,
                                         scattered) *
             ray_color(scattered, background, world,
                       depth - 1) /
             pdf_val;
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
  color background;
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
  color background = params.background;
  hittable_list scene = params.scene;
  immat imv;
  imv.resize(xrange);
  for (int i = 0; i < xrange; i++) {
    imv[i].resize(imheight, color(0));
  }

  for (int a = 0; a < xrange; a++) {
    for (int j = 0; j < imheight; j++) {
      int i = a + startx;
      //
      color rcolor(0.0, 0.0, 0.0);
      for (int k = 0; k < psample; k++) {
        double t =
            double(i + random_double()) / (imwidth - 1);
        double s =
            double(j + random_double()) / (imheight - 1);
        ray r = cam.get_ray(t, s);
        rcolor += ray_color(r, background, scene, mdepth);
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

int main() {
  auto start = std::chrono::high_resolution_clock::now();

  // Image
  auto aspect_ratio = 16.0 / 9.0;
  int image_width = 400;
  int samples_per_pixel = 100;
  int max_depth = 50;
  immat imvec;

  // World

  hittable_list world;
  int choice = 7;
  camera cam;
  int image_height;
  color background;
  choose_scene(choice, cam, world, samples_per_pixel,
               aspect_ratio, image_width, image_height,
               background);
  imvec.resize(image_width);
  for (int i = 0; i < image_width; i++) {
    imvec[i].resize(image_height, color(0));
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
    InnerParams params;
    params.startx = startx;
    params.endx = endx;
    params.cam = cam;
    params.imwidth = image_width;
    params.imheight = image_height;
    params.psample = samples_per_pixel;
    params.mdepth = max_depth;
    params.scene = world;
    params.background = background;
    futures[t] = std::async(&innerLoop, params);
  }
  for (auto &f : futures) {
    InnerRet ret = f.get();
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
  for (int j = image_height - 1; j >= 0; j -= 1) {
    std::cerr << "\rKalan Tarama Çizgisi:" << ' ' << j
              << ' ' << std::flush;
    for (int i = 0; i < image_width; i += 1) {
      write_color(std::cout, imvec[i][j],
                  samples_per_pixel);
    }
  }
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(
          stop - start);

  std::cerr << "duration" << std::endl;

  std::cerr << "\nDone.\n";
}
