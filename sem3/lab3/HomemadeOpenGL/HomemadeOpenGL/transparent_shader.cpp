#include "transparent_shader.h"

TransparentShader::TransparentShader(const TGAColor& color, float alpha)
    : color_(color), alpha_(alpha) {}

bool TransparentShader::fragment(Vec3f bar, TGAColor& color) {
  color = color_;
  return false;
}
