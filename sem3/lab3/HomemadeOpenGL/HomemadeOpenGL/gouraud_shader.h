#ifndef __GOURAUD_SHADER_H__
#define __GOURAUD_SHADER_H__

#include "shader.h"

class GouraudShader : public IShader {
 private:
  Model* model_;
  Vec3f light_dir_;
  float varying_intensity_[3];
  Vec2i varying_uv_[3];

 public:
  GouraudShader(Model* model, const Vec3f& light_dir);
  virtual bool fragment(Vec3f bar, TGAColor& color) override;
  void setVaryingIntensity(int index, float intensity);
  void setVaryingUV(int index, const Vec2i& uv);
};

#endif  // __GOURAUD_SHADER_H__
