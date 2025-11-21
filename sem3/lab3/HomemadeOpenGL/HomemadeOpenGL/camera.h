#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "geometry.h"

class Camera {
 private:
  Vec3f eye_;
  Vec3f center_;
  Vec3f up_;
  Matrix viewport_;  // вынес крч матрицы из оур.гд (который мне пипец не
                     // нравится) вот сюда
  Matrix projection_;
  Matrix modelview_;

 public:
  Camera(const Vec3f& eye = Vec3f(1, 1, 3),
         const Vec3f& center = Vec3f(0, 0, 0),
         const Vec3f& up = Vec3f(0, 1, 0));

  // Методы для обновления матриц
  void updateModelView();
  void updateProjection(float coeff = 0.0f);
  void updateViewport(int x, int y, int w, int h);

  // Геттеры для матриц
  const Matrix& getViewport() const { return viewport_; }
  const Matrix& getProjection() const { return projection_; }
  const Matrix& getModelView() const { return modelview_; }

  // Геттеры для параметров камеры
  const Vec3f& getEye() const { return eye_; }
  const Vec3f& getCenter() const { return center_; }
  const Vec3f& getUp() const { return up_; }

  // Сеттеры для параметров камеры
  void setEye(const Vec3f& eye) { eye_ = eye; }
  void setCenter(const Vec3f& center) { center_ = center; }
  void setUp(const Vec3f& up) { up_ = up; }

  // Вспомогательные методы
  void lookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up);
  float getFocalLength() const { return (eye_ - center_).norm(); }
};

#endif  // __CAMERA_H__
