#ifndef __PHONG_SHADER_H__
#define __PHONG_SHADER_H__

#include <iostream>

#include "shader.h"

class PhongShader : public IShader {
 private:
  Model* model_;
  Vec3f light_dir_;
  Vec3f view_dir_;
  Vec3f varying_normal_[3];
  Vec2i varying_uv_[3];

  // Параметры материала
  float ambient_strength_;
  float specular_strength_;
  float shininess_;

 public:
  PhongShader(Model* model, const Vec3f& light_dir, const Vec3f& view_dir,
              float ambient = 0.1f, float specular = 0.5f,
              float shininess = 32.0f);

  virtual bool fragment(Vec3f bar, TGAColor& color) override;
  void setVaryingNormal(int index, const Vec3f& normal);
  void setVaryingUV(int index, const Vec2i& uv);
};

#endif  // __PHONG_SHADER_H__
