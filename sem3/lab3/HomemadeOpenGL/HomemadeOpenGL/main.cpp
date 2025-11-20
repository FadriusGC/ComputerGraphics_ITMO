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

// Функция заливки треугольника
void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, TGAColor color) {
  // Игнорируем вырожденные треугольники (где все точки на одной линии)
  if (t0.y == t1.y && t0.y == t2.y) return;

  // Сортируем вершины по Y (от самой нижней к самой верхней)
  if (t0.y > t1.y) std::swap(t0, t1);
  if (t0.y > t2.y) std::swap(t0, t2);
  if (t1.y > t2.y) std::swap(t1, t2);

  int total_height = t2.y - t0.y;  // Общая высота треугольника

  // Проходим по всем строкам (сканирующим линиям) треугольника
  for (int i = 0; i < total_height; i++) {
    // Определяем, находимся ли мы в верхней или нижней части треугольника
    // Верхняя часть начинается после точки t1
    bool second_half = i > t1.y - t0.y || t1.y == t0.y;

    // Высота текущего сегмента (нижнего или верхнего)
    int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;

    // Параметры интерполяции:
    // alpha - прогресс по всему треугольнику (от t0 до t2)
    // beta - прогресс в текущем сегменте
    float alpha = (float)i / total_height;
    float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;

    // Вычисляем границы текущей сканирующей линии:
    // A - точка на длинном ребре (t0-t2)
    // B - точка на коротком ребре (t0-t1 для нижней части, t1-t2 для верхней)
    Vec2i A = t0 + (t2 - t0) * alpha;
    Vec2i B = second_half ? t1 + (t2 - t1) * beta : t0 + (t1 - t0) * beta;

    // Гарантируем, что A будет левой границей, а B - правой
    if (A.x > B.x) std::swap(A, B);

    // Рисуем горизонтальную линию между точками A и B
    for (int j = A.x; j <= B.x; j++) {
      image.set(j, t0.y + i, color);
    }
  }
}

int main(int argc, char** argv) {
  // Загружаем модель из файла
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  TGAImage image(width, height, TGAImage::RGB);

  // Направление источника света (смотрит "вглубь" экрана)
  Vec3f light_dir(0, 0, -1);

  // Обрабатываем все грани модели
  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec2i screen_coords[3];  // Экранные координаты (2D)
    Vec3f world_coords[3];   // Мировые координаты (3D)

    // Для каждой вершины треугольника
    for (int j = 0; j < 3; j++) {
      Vec3f v = model->vert(face[j]);
      // Преобразуем 3D координаты в 2D экранные координаты
      // (x+1)*width/2 переводит из диапазона [-1,1] в [0,width]
      screen_coords[j] =
          Vec2i((v.x + 1.) * width / 2., (v.y + 1.) * height / 2.);
      world_coords[j] = v;  // Сохраняем оригинальные 3D координаты
    }

    // Вычисляем нормаль треугольника с помощью векторного произведения
    // Нормаль = (v2-v0) × (v1-v0)
    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.normalize();  // Нормализуем вектор (делаем длину = 1)

    // Вычисляем интенсивность освещения как скалярное произведение нормали и
    // направления света Это дает косинус угла между ними
    float intensity = n * light_dir;

    // Если интенсивность положительна, треугольник обращен к свету и мы его
    // рисуем Если отрицательна - треугольник смотрит от нас (back-face culling)
    if (intensity > 0) {
      // Создаем цвет на основе интенсивности освещения
      // Умножаем на 255 чтобы перейти из диапазона [0,1] в [0,255]
      TGAColor color =
          TGAColor(intensity * 255, intensity * 255, intensity * 255, 255);
      triangle(screen_coords[0], screen_coords[1], screen_coords[2], image,
               color);
    }
  }

  // Переворачиваем изображение по вертикали, так как в TGA формат ожидает (0,0)
  // в левом нижнем углу
  image.flip_vertically();
  image.write_tga_file("output.tga");
  delete model;
  return 0;
}
