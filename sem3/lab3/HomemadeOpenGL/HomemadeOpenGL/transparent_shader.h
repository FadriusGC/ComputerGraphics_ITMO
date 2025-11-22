#ifndef __TRANSPARENT_SHADER_H__
#define __TRANSPARENT_SHADER_H__

#include "shader.h"

class TransparentShader : public IShader {
 private:
  TGAColor color_;
  float alpha_;

 public:
  TransparentShader(const TGAColor& color, float alpha = 0.3f);
  virtual bool fragment(Vec3f bar, TGAColor& color) override;
};

#endif  // __TRANSPARENT_SHADER_H__
