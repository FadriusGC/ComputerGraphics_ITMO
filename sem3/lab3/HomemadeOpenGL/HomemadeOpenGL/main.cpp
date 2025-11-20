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

void triangle(Vec3f t0, Vec3f t1, Vec3f t2, TGAImage& image, TGAColor color,
              float* zbuffer) {
  // Игнорируем вырожденные треугольники
  if (t0.y == t1.y && t0.y == t2.y) return;

  // Сортируем вершины по Y (от самой нижней к самой верхней)
  if (t0.y > t1.y) std::swap(t0, t1);
  if (t0.y > t2.y) std::swap(t0, t2);
  if (t1.y > t2.y) std::swap(t1, t2);

  int total_height = t2.y - t0.y;

  for (int i = 0; i < total_height; i++) {
    bool second_half = i > t1.y - t0.y || t1.y == t0.y;
    int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

    float alpha = (float)i / total_height;
    float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;

    // Вычисляем точки A и B с учетом Z-координаты
    Vec3f A = t0 + (t2 - t0) * alpha;
    Vec3f B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;

    if (A.x > B.x) std::swap(A, B);

    // Для каждого пикселя в горизонтальной линии
    for (int j = A.x; j <= B.x; j++) {
      // Интерполируем Z-координату между A и B
      float phi = (B.x == A.x) ? 1.0f : (float)(j - A.x) / (float)(B.x - A.x);
      Vec3f P = A + (B - A) * phi;

      // Вычисляем индекс в Z-буфере
      int idx = int(P.x) + int(P.y) * width;

      // Проверяем, находится ли текущий пиксель ближе к камере
      if (idx < width * height && zbuffer[idx] < P.z) {
        zbuffer[idx] = P.z;          // Обновляем Z-буфер
        image.set(P.x, P.y, color);  // Рисуем пиксель
      }
    }
  }
}

int main(int argc, char** argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);

  // СОЗДАЕМ И ИНИЦИАЛИЗИРУЕМ ZZZZZ-БУФЕР
  float* zbuffer = new float[width * height];
  for (int i = 0; i < width * height; i++) {
    zbuffer[i] = -std::numeric_limits<float>::max();  // Инициализируем очень
                                                      // маленькими значениями
  }

  Vec3f light_dir(0, 0, -1);

  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3f screen_coords[3];  // Теперь используем Vec3f вместо Vec2i
    Vec3f world_coords[3];

    for (int j = 0; j < 3; j++) {
      Vec3f v = model->vert(face[j]);
      // Сохраняем Z-координату из мировых координат
      screen_coords[j] =
          Vec3f((v.x + 1.) * width / 2., (v.y + 1.) * height / 2., v.z);
      world_coords[j] = v;
    }

    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.normalize();
    float intensity = n * light_dir;

    if (intensity > 0) {
      TGAColor color =
          TGAColor(intensity * 255, intensity * 255, intensity * 255, 255);
      // Передаем zbuffer в функцию triangle
      triangle(screen_coords[0], screen_coords[1], screen_coords[2], image,
               color, zbuffer);
    }
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");

  // ОСВОБОЖДАЕМ ПАМЯТЬ ZZZZZZZZ-БУФЕРА
  delete[] zbuffer;
  delete model;
  return 0;
}
