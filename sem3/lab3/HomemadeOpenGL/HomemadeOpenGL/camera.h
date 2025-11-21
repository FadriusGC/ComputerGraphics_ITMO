#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "geometry.h"

class Camera {
 private:
  Vec3f eye_;        // Позиция камеры
  Vec3f center_;     // Точка, куда смотрит камера
  Vec3f up_;         // Вектор "вверх"
  Vec3f x_, y_, z_;  // Локальные оси камеры

  float fov_;     // Поле зрения
  float aspect_;  // Соотношение сторон
  float near_;    // Ближняя плоскость отсечения
  float far_;     // Дальняя плоскость отсечения

  void updateAxes();

 public:
  Camera(Vec3f eye = Vec3f(0, 0, 3), Vec3f center = Vec3f(0, 0, 0),
         Vec3f up = Vec3f(0, 1, 0), float fov = 45.0f, float aspect = 1.0f,
         float near = 0.1f, float far = 50.0f);

  // Установка параметров камеры
  void setPosition(const Vec3f& eye);
  void setTarget(const Vec3f& center);
  void setUpVector(const Vec3f& up);
  void setProjection(float fov, float aspect, float near, float far);

  // Получение матриц
  Matrix getViewMatrix();
  Matrix getProjectionMatrix();

  // Движение камеры
  void move(const Vec3f& direction);
  void rotate(float yaw, float pitch);  // Поворот камеры

  // Геттеры
  Vec3f getPosition() const { return eye_; }
  Vec3f getTarget() const { return center_; }
  Vec3f getUpVector() const { return up_; }
};

#endif  // __CAMERA_H__
