#include "camera.h"

#include <iostream>

#include "our_gl.h"

Camera::Camera(const Vec3f& eye, const Vec3f& center, const Vec3f& up)
    : eye_(eye), center_(center), up_(up) {
  viewport_ = Matrix::identity(4);
  projection_ = Matrix::identity(4);
  modelview_ = Matrix::identity(4);
}

void Camera::updateModelView() {
  // Вычисляем векторы для системы координат камеры
  Vec3f z = (eye_ - center_).normalize();  // Направление "вперед" камеры
  Vec3f x = (up_ ^ z).normalize();         // Направление "вправо" камеры
  Vec3f y = (z ^ x).normalize();           // Направление "вверх" камеры

  modelview_ = Matrix::identity(4);
  for (int i = 0; i < 3; i++) {
    modelview_[0][i] = x[i];
    modelview_[1][i] = y[i];
    modelview_[2][i] = z[i];
    modelview_[i][3] = -center_[i];  // Смещение относительно центра сцены
  }
}

void Camera::updateProjection(float coeff) {
  projection_ = Matrix::identity(4);
  projection_[3][2] = coeff;  // Устанавливаем коэффициент перспективы
}

void Camera::updateViewport(int x, int y, int w, int h) {
  viewport_ = Matrix::identity(4);

  // Устанавливаем смещение
  viewport_[0][3] = x + w / 2.0f;
  viewport_[1][3] = y + h / 2.0f;
  viewport_[2][3] = 255.0f / 2.0f;  // Для Z-буфера

  // Устанавливаем масштаб
  viewport_[0][0] = w / 2.0f;
  viewport_[1][1] = h / 2.0f;
  viewport_[2][2] = 255.0f / 2.0f;  // Для Z-буфера
}

// Удобный метод для установки всех параметров камеры сразу
void Camera::lookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
  eye_ = eye;
  center_ = center;
  up_ = up;
  updateModelView();
}
