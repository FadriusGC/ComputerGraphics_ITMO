#include <cmath>

#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

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

  // Здесь начинается оптимизированный алгоритм Брезенхэма.
  // Вместо плавающей точки используется целочисленная ошибка.
  float derror = std::abs(dy / (float)dx);  // Изменение ошибки на один шаг по X
  float error = 0;                          // Накопленная ошибка
  int y = y0;                               // Текущий Y

  for (int x = x0; x <= x1; x++) {
    if (steep) {
      // Если линия была транспонирована, рисуем пиксель, "развернув" его
      // обратно
      image.set(y, x, color);
    } else {
      image.set(x, y, color);
    }

    error += derror;  // Увеличиваем накопленную ошибку

    // Если ошибка превысила полпикселя, пора сдвинуться по Y
    if (error > 0.5) {
      y += (y1 > y0
                ? 1
                : -1);  // Двигаемся вверх или вниз в зависимости от направления
      error -= 1.;      // "Сбрасываем" ошибку
    }
  }
}

int main(int argc, char** argv) {
  TGAImage image(100, 100, TGAImage::RGB);
  for (int i = 0; i < 1000000; i++) {
    line(13, 20, 80, 40, image, white);
    line(20, 13, 40, 80, image, red);
    line(80, 40, 13, 20, image, red);
  }
  image.flip_vertically();  // i want to have the origin at the left bottom
                            // corner of the image
  image.write_tga_file("output.tga");
  return 0;
}
