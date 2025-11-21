#include "camera.h"

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera::Camera(Vec3f eye, Vec3f center, Vec3f up, float fov, float aspect,
               float near, float far)
    : eye_(eye),
      center_(center),
      up_(up),
      fov_(fov),
      aspect_(aspect),
      near_(near),
      far_(far) {
  updateAxes();
}

void Camera::updateAxes() {
  // Вычисляем локальные оси камеры
  z_ = (eye_ - center_).normalize();  // Направление взгляда (обратное)
  x_ = (up_ ^ z_).normalize();        // Правая ось (векторное произведение)
  y_ = (z_ ^ x_).normalize();         // вектор верха
}

void Camera::setPosition(const Vec3f& eye) {
  eye_ = eye;
  updateAxes();
}

void Camera::setTarget(const Vec3f& center) {
  center_ = center;
  updateAxes();
}

void Camera::setUpVector(const Vec3f& up) {
  up_ = up;
  updateAxes();
}

void Camera::setProjection(float fov, float aspect, float near, float far) {
  fov_ = fov;
  aspect_ = aspect;
  near_ = near;
  far_ = far;
}

Matrix Camera::getViewMatrix() {
  updateAxes();

  Matrix Minv = Matrix::identity(4);
  Matrix Tr = Matrix::identity(4);

  // Заполняем матрицу поворота
  for (int i = 0; i < 3; i++) {
    Minv[0][i] = x_[i];
    Minv[1][i] = y_[i];
    Minv[2][i] = z_[i];
    Tr[i][3] = -center_[i];
  }

  return Minv * Tr;
}

Matrix Camera::getProjectionMatrix() {
  float tanHalfFov = tan(fov_ * M_PI / 360.0f);  // fov/2 в радианах
  float range = near_ - far_;

  Matrix projection = Matrix::identity(4);

  // Перспективная проекция
  projection[0][0] = 1.0f / (aspect_ * tanHalfFov);
  projection[1][1] = 1.0f / tanHalfFov;
  projection[2][2] = (-near_ - far_) / range;
  projection[2][3] = 2.0f * far_ * near_ / range;
  projection[3][2] = 1.0f;
  projection[3][3] = 0.0f;

  return projection;
}

void Camera::move(const Vec3f& direction) {
  // Движение в локальных координатах камеры
  Vec3f moveVec = x_ * direction.x + y_ * direction.y + z_ * direction.z;
  eye_ = eye_ + moveVec;
  center_ = center_ + moveVec;
}

void Camera::rotate(float yaw, float pitch) {
  Vec3f direction = eye_ - center_;

  // Поворот по yaw (влево-вправо)
  float cosYaw = cos(yaw);
  float sinYaw = sin(yaw);
  float newX = direction.x * cosYaw - direction.z * sinYaw;
  float newZ = direction.x * sinYaw + direction.z * cosYaw;
  direction.x = newX;
  direction.z = newZ;

  // Поворот по pitch (вверх-вниз)
  float cosPitch = cos(pitch);
  float sinPitch = sin(pitch);
  float newY = direction.y * cosPitch - direction.z * sinPitch;
  newZ = direction.y * sinPitch + direction.z * cosPitch;
  direction.y = newY;
  direction.z = newZ;

  eye_ = center_ + direction;
  updateAxes();
}
