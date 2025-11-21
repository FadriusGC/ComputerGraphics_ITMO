#include "phong_shader.h"

#include <algorithm>
#include <cmath>
#include <iostream>

PhongShader::PhongShader(Model* model, const Vec3f& light_dir,
                         const Vec3f& view_dir, float ambient, float specular,
                         float shininess)
    : model_(model),
      light_dir_(light_dir.normalize()),
      view_dir_(view_dir.normalize()),
      ambient_strength_(ambient),
      specular_strength_(specular),
      shininess_(shininess) {}

bool PhongShader::fragment(Vec3f bar, TGAColor& color) {
  // Интерполяция нормали и текстурных координат
  Vec3f normal = (varying_normal_[0] * bar.x + varying_normal_[1] * bar.y +
                  varying_normal_[2] * bar.z)
                     .normalize();

  Vec2i uv =
      varying_uv_[0] * bar.x + varying_uv_[1] * bar.y + varying_uv_[2] * bar.z;

  // Диффузная компонента (как в Ламберте)
  float diff = std::max(normal * light_dir_, 0.0f);

  // Зеркальная компонента (Phong)
  Vec3f reflect_dir =
      (normal * (normal * light_dir_) * 2.0f - light_dir_).normalize();
  float spec = pow(std::max(view_dir_ * reflect_dir, 0.0f), shininess_);

  // Ambient компонента
  float ambient = ambient_strength_;

  // Итоговая интенсивность
  float intensity = ambient + diff + spec * specular_strength_;
  intensity = std::max(0.0f, std::min(1.0f, intensity));

  // Получаем цвет текстуры и применяем освещение
  TGAColor diffuse_color = model_->diffuse(uv);

  color =
      TGAColor(static_cast<unsigned char>(diffuse_color.bgra[2] * intensity),
               static_cast<unsigned char>(diffuse_color.bgra[1] * intensity),
               static_cast<unsigned char>(diffuse_color.bgra[0] * intensity),
               diffuse_color.bgra[3]);

  return false;
}

void PhongShader::setVaryingNormal(int index, const Vec3f& normal) {
  if (index >= 0 && index < 3) {
    varying_normal_[index] = normal;
  }
}

void PhongShader::setVaryingUV(int index, const Vec2i& uv) {
  if (index >= 0 && index < 3) {
    varying_uv_[index] = uv;
  }
}
