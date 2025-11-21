#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "camera.h"
#include "geometry.h"
#include "gouraud_shader.h"
#include "model.h"
#include "phong_shader.h"
#include "rasterizer.h"
#include "shader.h"
#include "tgaimage.h"

const int width = 800;
const int height = 800;

// Перечисление для выбора шейдера
enum ShaderType { DIFFUSE = 0, PHONG = 1, GOURAUD = 2 };

int main(int argc, char** argv) {
  Model* model =
      (argc == 2) ? new Model(argv[1]) : new Model("obj/african_head.obj");

  TGAImage image(width, height, TGAImage::RGB);
  TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

  // Инициализация камеры
  Camera camera;
  camera.lookAt(Vec3f(1, 1, 3), Vec3f(0, 0, 0), Vec3f(0, 1, 0));
  camera.updateViewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
  camera.updateProjection(-1.0f / camera.getFocalLength());

  Rasterizer rasterizer(image, zbuffer);
  Vec3f light_dir(1, 1, 1);

  // Выбор шейдера (можно сделать через аргументы командной строки)
  ShaderType current_shader =
      GOURAUD;  // Измените на DIFFUSE, PHONG или GOURAUD

  // Вектор взгляда для Phong shading
  Vec3f view_dir = (camera.getEye() - camera.getCenter()).normalize();

  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3i screen_coords[3];
    Vec3f world_coords[3];

    if (current_shader == DIFFUSE) {
      // Diffuse shader (Ламберт)
      DiffuseShader shader(model, light_dir);

      for (int j = 0; j < 3; j++) {
        Vec3f world_coord = model->vert(face[j]);
        screen_coords[j] = Vec3f(camera.getViewport() * camera.getProjection() *
                                 camera.getModelView() * Matrix(world_coord));
        shader.setVaryingUV(j, model->uv(i, j));
      }
      rasterizer.triangle(screen_coords, shader);

    } else if (current_shader == PHONG) {
      // Phong shader
      PhongShader shader(model, light_dir, view_dir, 0.1f, 0.5f, 32.0f);

      for (int j = 0; j < 3; j++) {
        Vec3f world_coord = model->vert(face[j]);
        screen_coords[j] = Vec3f(camera.getViewport() * camera.getProjection() *
                                 camera.getModelView() * Matrix(world_coord));

        shader.setVaryingUV(j, model->uv(i, j));
        shader.setVaryingNormal(j, model->norm(i, j));
      }
      rasterizer.triangle(screen_coords, shader);

    } else if (current_shader == GOURAUD) {
      // Gouraud shader
      GouraudShader shader(model, light_dir);

      for (int j = 0; j < 3; j++) {
        Vec3f world_coord = model->vert(face[j]);
        screen_coords[j] = Vec3f(camera.getViewport() * camera.getProjection() *
                                 camera.getModelView() * Matrix(world_coord));

        // Вычисляем интенсивность освещения в вершине
        Vec3f normal = model->norm(i, j);
        float intensity = std::max(normal * light_dir, 0.0f);

        shader.setVaryingUV(j, model->uv(i, j));
        shader.setVaryingIntensity(j, intensity);
      }
      rasterizer.triangle(screen_coords, shader);
    }
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");

  zbuffer.flip_vertically();
  zbuffer.write_tga_file("zbuffer.tga");

  delete model;
  return 0;
}
