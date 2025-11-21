#include <algorithm>

#include "gouraud_shader.h"

GouraudShader::GouraudShader(Model* model, const Vec3f& light_dir)
    : model_(model), light_dir_(light_dir.normalize()) {
  // Инициализируем массив интенсивностей
  for (int i = 0; i < 3; i++) {
    varying_intensity_[i] = 0.0f;
  }
}

bool GouraudShader::fragment(Vec3f bar, TGAColor& color) {
  // Интерполяция интенсивности (вычисленной в вершинном шейдере)
  float intensity = varying_intensity_[0] * bar.x +
                    varying_intensity_[1] * bar.y +
                    varying_intensity_[2] * bar.z;

  intensity = std::max(0.0f, std::min(1.0f, intensity));

  // Интерполяция текстурных координат
  Vec2i uv =
      varying_uv_[0] * bar.x + varying_uv_[1] * bar.y + varying_uv_[2] * bar.z;

  // Получаем цвет текстуры и применяем интерполированную интенсивность
  TGAColor diffuse_color = model_->diffuse(uv);

  color =
      TGAColor(static_cast<unsigned char>(diffuse_color.bgra[2] * intensity),
               static_cast<unsigned char>(diffuse_color.bgra[1] * intensity),
               static_cast<unsigned char>(diffuse_color.bgra[0] * intensity),
               diffuse_color.bgra[3]);

  return false;
}

void GouraudShader::setVaryingIntensity(int index, float intensity) {
  if (index >= 0 && index < 3) {
    varying_intensity_[index] = intensity;
  }
}

void GouraudShader::setVaryingUV(int index, const Vec2i& uv) {
  if (index >= 0 && index < 3) {
    varying_uv_[index] = uv;
  }
}
