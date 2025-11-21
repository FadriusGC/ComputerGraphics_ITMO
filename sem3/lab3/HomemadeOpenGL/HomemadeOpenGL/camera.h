#ifndef __CAMERA_H__
#define __CAMERA_H__

#include <iostream>

#include "geometry.h"
class Camera {
 private:
  Vec3f eye_;
  Vec3f center_;
  Vec3f up_;
  Matrix viewport_;
  Matrix projection_;
  Matrix modelview_;

 public:
  Camera(const Vec3f& eye = Vec3f(1, 1, 3),
         const Vec3f& center = Vec3f(0, 0, 0),
         const Vec3f& up = Vec3f(0, 1, 0));

  void updateModelView();
  void updateProjection(float coeff = 0.0f);
  void updateViewport(int x, int y, int w, int h);

  Matrix getViewport() const { return viewport_; }
  Matrix getProjection() const { return projection_; }
  Matrix getModelView() const { return modelview_; }

  Vec3f getEye() const { return eye_; }
  Vec3f getCenter() const { return center_; }
  Vec3f getUp() const { return up_; }

  void setEye(const Vec3f& eye);
  void setCenter(const Vec3f& center);
  void setUp(const Vec3f& up);

  void lookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up);
  float getFocalLength() const;
};

#endif  // __CAMERA_H__
