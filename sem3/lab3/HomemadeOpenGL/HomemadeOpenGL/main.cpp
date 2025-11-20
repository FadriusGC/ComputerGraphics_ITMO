#include <cmath>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
Model* model = NULL;
const int width = 800;
const int height = 800;

const int depth = 255;  // константа глубины

int* zbuffer = NULL;  // Целочисленный Z-буфер
Vec3f light_dir(0, 0, -1);

void line(int x0, int y0, int x1, int y1, TGAImage& image, TGAColor color) {
  bool steep = false;  // Флаг "крутизны" линии. Крутая = когда изменение по Y
                       // больше, чем по X.

  // Если линия крутая, мы мысленно "транспонируем" её (меняем X и Y местами)
  if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
    std::swap(x0, y0);
    std::swap(x1, y1);
    steep = true;
  }

  // Гарантируем, что будем рисовать слева направо (от меньшего X к большему)
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  // Вычисляем изменения
  int dx = x1 - x0;
  int dy = y1 - y0;

  // оптимизированный алгоритм Брезенхэма.
  // Вместо плавающей точки используется целочисленная ошибка.
  float derror = std::abs(dy / (float)dx);  // Изменение ошибки на один шаг по X
  float error = 0;
  int y = y0;

  for (int x = x0; x <= x1; x++) {
    if (steep) {
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }

    error += derror;

    // Если ошибка превысила полпикселя, пора сдвинуться по Y
    if (error > 0.5) {
      y += (y1 > y0
                ? 1
                : -1);  // Двигаемся вверх или вниз в зависимости от направления
      error -= 1.;
    }
  }
}

// Ообновлегная TRIANGLE
void triangle(Vec3i t0, Vec3i t1, Vec3i t2, TGAImage& image, TGAColor color,
              int* zbuffer) {
  if (t0.y == t1.y && t0.y == t2.y) return;

  if (t0.y > t1.y) std::swap(t0, t1);
  if (t0.y > t2.y) std::swap(t0, t2);
  if (t1.y > t2.y) std::swap(t1, t2);

  int total_height = t2.y - t0.y;

  for (int i = 0; i < total_height; i++) {
    bool second_half = i > t1.y - t0.y || t1.y == t0.y;
    int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

    float alpha = (float)i / total_height;
    float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;

    Vec3i A = t0 + (t2 - t0) * alpha;
    Vec3i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;

    if (A.x > B.x) std::swap(A, B);

    for (int j = A.x; j <= B.x; j++) {
      float phi = (B.x == A.x) ? 1.0f : (float)(j - A.x) / (float)(B.x - A.x);
      Vec3i P = A + (B - A) * phi;

      // исправление проблем с целочисленным приведением
      P.x = j;
      P.y = t0.y + i;

      int idx = P.x + P.y * width;

      if (zbuffer[idx] < P.z) {
        zbuffer[idx] = P.z;
        image.set(P.x, P.y, color);
      }
    }
  }
}

// СОЗДАЕМ И ИНИЦИАЛИЗИРУЕМ ZZZZZ-БУФЕР
int main(int argc, char** argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  zbuffer = new int[width * height];
  for (int i = 0; i < width * height; i++) {
    zbuffer[i] = std::numeric_limits<int>::min();
  }

  TGAImage image(width, height, TGAImage::RGB);

  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3i screen_coords[3];  // Vec3i вместо Vec3f
    Vec3f world_coords[3];

    for (int j = 0; j < 3; j++) {
      Vec3f v = model->vert(face[j]);
      // важная хрень - преобразование Z-координаты
      screen_coords[j] =
          Vec3i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.,
                (v.z + 1.) * depth / 2.);
      world_coords[j] = v;
    }

    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.normalize();
    float intensity = n * light_dir;

    if (intensity > 0) {
      TGAColor color =
          TGAColor(intensity * 255, intensity * 255, intensity * 255, 255);
      triangle(screen_coords[0], screen_coords[1], screen_coords[2], image,
               color, zbuffer);
    }
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");

  // сохранение Z-буфера для отладки
  {
    TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        zbimage.set(i, j, TGAColor(zbuffer[i + j * width], 1));
      }
    }
    zbimage.flip_vertically();
    zbimage.write_tga_file("zbuffer.tga");
  }

  delete[] zbuffer;
  delete model;
  return 0;
}
