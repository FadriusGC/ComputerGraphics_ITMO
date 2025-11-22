#include <chrono>
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
#include "renderer_ui.h"
#include "shader.h"
#include "tgaimage.h"
#include "transparent_object.h"
#include "transparent_shader.h"
#define NOMINMAX
#include "Windows.h"

const int width = 800;
const int height = 800;

int main(int argc, char** argv) {
  RendererUI ui;
  ui.localise();
  ui.printHeader();

  ShaderType current_shader = ui.selectShader();

  std::cout << u8"\nЗагрузка модели...\n";

  // Загрузка модели
  Model* model = nullptr;
  try {
    model =
        (argc == 2) ? new Model(argv[1]) : new Model("obj/african_head.obj");
    ui.showModelInfo(model);
  } catch (const std::exception& e) {
    std::cout << u8"Ошибка загрузки модели: " << e.what() << "\n";
    return 1;
  }

  // Инициализация буферов и камеры
  TGAImage image(width, height, TGAImage::RGB);
  TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

  Camera camera;
  camera.lookAt(Vec3f(-1.5, 1, 3), Vec3f(0, 0, 0), Vec3f(0, 1, 0));
  camera.updateViewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
  camera.updateProjection(-1.0f / camera.getFocalLength());

  Rasterizer rasterizer(image, zbuffer);
  Vec3f light_dir(1, 1, 1);
  Vec3f view_dir = (camera.getEye() - camera.getCenter()).normalize();

  TransparentObject iceCube;
  iceCube.createCube(Vec3f(0, 0.2f, 0.5f), 1.5f);
  iceCube.setColor(TGAColor(100, 200, 255, 128));
  iceCube.setAlpha(0.4f);

  TransparentShader iceShader(iceCube.getColor(), iceCube.getAlpha());

  // Показываем настройки рендеринга
  ui.showRenderSettings(current_shader, width, height);

  // Общее количество полигонов для прогресс-бара
  int total_faces = model->nfaces() + iceCube.nfaces();
  ui.startRender(total_faces);
  auto start_time = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3i screen_coords[3];

    ui.updateProgress(i);

    switch (current_shader) {
      case DIFFUSE: {
        DiffuseShader shader(model, light_dir);
        for (int j = 0; j < 3; j++) {
          Vec3f world_coord = model->vert(face[j]);
          screen_coords[j] =
              Vec3f(camera.getViewport() * camera.getProjection() *
                    camera.getModelView() * Matrix(world_coord));
          shader.setVaryingUV(j, model->uv(i, j));
        }
        rasterizer.triangle(screen_coords, shader);
        break;
      }

      case GOURAUD: {
        GouraudShader shader(model, light_dir);
        for (int j = 0; j < 3; j++) {
          Vec3f world_coord = model->vert(face[j]);
          screen_coords[j] =
              Vec3f(camera.getViewport() * camera.getProjection() *
                    camera.getModelView() * Matrix(world_coord));

          Vec3f normal = model->norm(i, j);
          float intensity = (std::max)(normal * light_dir, 0.0f);

          shader.setVaryingUV(j, model->uv(i, j));
          shader.setVaryingIntensity(j, intensity);
        }
        rasterizer.triangle(screen_coords, shader);
        break;
      }

      case PHONG: {
        PhongShader shader(model, light_dir, view_dir, 0.1f, 0.5f, 32.0f);
        for (int j = 0; j < 3; j++) {
          Vec3f world_coord = model->vert(face[j]);
          screen_coords[j] =
              Vec3f(camera.getViewport() * camera.getProjection() *
                    camera.getModelView() * Matrix(world_coord));

          shader.setVaryingUV(j, model->uv(i, j));
          shader.setVaryingNormal(j, model->norm(i, j));
        }
        rasterizer.triangle(screen_coords, shader);
        break;
      }
    }
  }

  for (int i = 0; i < iceCube.nfaces(); i++) {
    std::vector<int> face = iceCube.face(i);
    Vec3i screen_coords[3];

    for (int j = 0; j < 3; j++) {
      Vec3f world_coord = iceCube.vert(face[j]);
      screen_coords[j] = Vec3f(camera.getViewport() * camera.getProjection() *
                               camera.getModelView() * Matrix(world_coord));
    }

    rasterizer.triangle(screen_coords, iceShader, true);  // true = прозрачный
    ui.updateProgress(model->nfaces() + i);
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  ui.finishRender();

  std::cout << u8"\nСохранение результатов...";

  // Сохранение результатов
  image.flip_vertically();
  image.write_tga_file("frozen_output.tga");

  zbuffer.flip_vertically();
  zbuffer.write_tga_file("frozen_zbuffer.tga");

  ui.showStatistics(model, duration.count());

  delete model;
  return 0;
}
