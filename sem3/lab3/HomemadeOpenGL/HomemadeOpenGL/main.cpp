#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "camera.h"
#include "geometry.h"
#include "model.h"
#include "rasterizer.h"
#include "shader.h"
#include "tgaimage.h"

const int width = 800;
const int height = 800;

int main(int argc, char** argv) {
  Model* model =
      (argc == 2) ? new Model(argv[1]) : new Model("obj/african_head.obj");

  TGAImage image(width, height, TGAImage::RGB);
  TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

  Camera camera;
  camera.lookAt(Vec3f(1, 1, 1), Vec3f(0, 0, 0), Vec3f(0, 1, 0));
  camera.updateViewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
  camera.updateProjection(-1.0f / camera.getFocalLength());

  Rasterizer rasterizer(image, zbuffer);
  Vec3f light_dir(1, 1, 1);
  DiffuseShader shader(model, light_dir);

  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3i screen_coords[3];

    for (int j = 0; j < 3; j++) {
      Vec3f world_coord = model->vert(face[j]);
      screen_coords[j] = Vec3f(camera.getViewport() * camera.getProjection() *
                               camera.getModelView() * Matrix(world_coord));
      shader.setVaryingUV(j, model->uv(i, j));
    }

    rasterizer.triangle(screen_coords, shader);
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");

  zbuffer.flip_vertically();
  zbuffer.write_tga_file("zbuffer.tga");

  delete model;
  return 0;
}
