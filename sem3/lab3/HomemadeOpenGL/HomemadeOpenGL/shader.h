#ifndef __SHADER_H__
#define __SHADER_H__

#include <iostream>

#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

class IShader {
 public:
  virtual ~IShader() {}
  virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

class DiffuseShader : public IShader {
 private:
  Model* model_;
  Vec3f light_dir_;
  Vec2i varying_uv_[3];

 public:
  DiffuseShader(Model* model, const Vec3f& light_dir);
  virtual bool fragment(Vec3f bar, TGAColor& color) override;
  void setVaryingUV(int index, const Vec2i& uv);
};

#endif  // __SHADER_H__
