#include <cmath>
#include <limits>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

const int width = 800;
const int height = 800;
const int depth = 255;

Model* model = NULL;
int* zbuffer = NULL;
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

// Функция отрисовки текстурированного треугольника
void triangle(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2,
              TGAImage& image, float intensity, int* zbuffer) {
  // Пропускаем вырожденные треугольники (все точки на одной линии)
  if (t0.y == t1.y && t0.y == t2.y) return;

  // сортиуем вершины по у (снизу вверх)
  // При сортировке вершин также сортируем соответствующие UV-координаты
  if (t0.y > t1.y) {
    std::swap(t0, t1);
    std::swap(uv0, uv1);
  }
  if (t0.y > t2.y) {
    std::swap(t0, t2);
    std::swap(uv0, uv2);
  }
  if (t1.y > t2.y) {
    std::swap(t1, t2);
    std::swap(uv1, uv2);
  }

  int total_height = t2.y - t0.y;  // Общая высота треугольника

  // идем по строчкам треугольника
  for (int i = 0; i < total_height; i++) {
    // Определяем, в верхней или нижней части треугольника мы находимся
    bool second_half = i > t1.y - t0.y || t1.y == t0.y;
    int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

    // задаем параметры для интерполяции
    float alpha = (float)i / total_height;
    float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;

    // Интерполяция координат на левом и правом ребрах
    Vec3i A = t0 + (t2 - t0) * alpha;
    Vec3i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;

    // Интерполяция UV-координат на левом и правом ребрах
    Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
    Vec2i uvB =
        second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;

    // Гарантируем, что A - левая граница, B - правая
    if (A.x > B.x) {
      std::swap(A, B);
      std::swap(uvA, uvB);
    }

    // Отрисовка горизонтальной линии между A и B
    for (int j = A.x; j <= B.x; j++) {
      // Интерполяция внутри горизонтальной линии
      float phi = (B.x == A.x) ? 1.0f : (float)(j - A.x) / (float)(B.x - A.x);

      // Интерполяция 3D и UV координат соответственно
      Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
      Vec2i uvP = uvA + (uvB - uvA) * phi;

      // Исправление координат (из-за проблем с целочисленной точностью)
      P.x = j;
      P.y = t0.y + i;

      // Вычисление индекса в Z-буфере
      int idx = P.x + P.y * width;

      // смотрим Z буффер
      if (idx < width * height && zbuffer[idx] < P.z) {
        zbuffer[idx] = P.z;  // Обновляем Z-буфер

        // Собсна текстурирование - получаем цвет из текстуры и применяем
        // освещение
        TGAColor color = model->diffuse(uvP);
        image.set(P.x, P.y,
                  TGAColor(color.r * intensity, color.g * intensity,
                           color.b * intensity));
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

  // инициализируем ZZZ буфер
  zbuffer = new int[width * height];
  for (int i = 0; i < width * height; i++) {
    zbuffer[i] =
        std::numeric_limits<int>::min();  // начинаем с минимального значения
  }

  {
    TGAImage image(width, height, TGAImage::RGB);

    // обработка граней
    for (int i = 0; i < model->nfaces(); i++) {
      std::vector<int> face = model->face(i);
      Vec3i screen_coords[3];  // Экранные координаты вершин
      Vec3f world_coords[3];   // Мировые координаты вершин

      // координаты дял каждоый вершины преобразуем
      for (int j = 0; j < 3; j++) {
        Vec3f v = model->vert(face[j]);

        screen_coords[j] =
            Vec3i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.,
                  (v.z + 1.) * depth / 2.);
        world_coords[j] = v;
      }

      // нм и освещение
      Vec3f n = (world_coords[2] - world_coords[0]) ^
                (world_coords[1] - world_coords[0]);
      n.normalize();
      float intensity = n * light_dir;

      //(back-face culling)
      if (intensity > 0) {
        // Получаем UV-координаты для вершин текущей грани
        Vec2i uv[3];
        for (int k = 0; k < 3; k++) {
          uv[k] = model->uv(i, k);
        }

        // Отрисовка текстурированного треугольника
        triangle(screen_coords[0], screen_coords[1], screen_coords[2], uv[0],
                 uv[1], uv[2], image, intensity, zbuffer);
      }
    }

    // Сохранение результата
    image.flip_vertically();
    image.write_tga_file("output.tga");
  }

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

  delete model;
  delete[] zbuffer;
  return 0;
}
