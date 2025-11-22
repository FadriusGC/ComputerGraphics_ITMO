#include "transparent_shader.h"

TransparentShader::TransparentShader(const TGAColor& color, float alpha)
    : color_(color), alpha_(alpha) {
  color_.bgra[3] = static_cast<unsigned char>(alpha_ * 255);
}

bool TransparentShader::fragment(Vec3f bar, TGAColor& color) {
  color = color_;
  return false;
}
