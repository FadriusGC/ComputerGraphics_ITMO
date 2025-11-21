#include "shader.h"

#include <algorithm>
#include <iostream>

DiffuseShader::DiffuseShader(Model* model, const Vec3f& light_dir)
    : model_(model), light_dir_(light_dir.normalize()) {}

bool DiffuseShader::fragment(Vec3f bar, TGAColor& color) {
  Vec2i uv =
      varying_uv_[0] * bar.x + varying_uv_[1] * bar.y + varying_uv_[2] * bar.z;

  Vec3f normal = model_->norm(uv);
  float intensity = normal * light_dir_;
  intensity = std::max(0.0f, std::min(1.0f, intensity));

  TGAColor diffuse_color = model_->diffuse(uv);

  // В TGAColor цвета хранятся в массиве bgra в порядке: B, G, R, A
  unsigned char r =
      static_cast<unsigned char>(diffuse_color.bgra[2] * intensity);
  unsigned char g =
      static_cast<unsigned char>(diffuse_color.bgra[1] * intensity);
  unsigned char b =
      static_cast<unsigned char>(diffuse_color.bgra[0] * intensity);
  unsigned char a = diffuse_color.bgra[3];  // Альфу не трогаю

  color = TGAColor(r, g, b, a);
  return false;
}

void DiffuseShader::setVaryingUV(int index, const Vec2i& uv) {
  if (index >= 0 && index < 3) {
    varying_uv_[index] = uv;
  }
}
