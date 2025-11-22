#ifndef __TRANSPARENT_OBJECT_H__
#define __TRANSPARENT_OBJECT_H__

#include <vector>

#include "geometry.h"
#include "tgaimage.h"

class TransparentObject {
 private:
  std::vector<Vec3f> vertices_;
  std::vector<std::vector<int>> faces_;
  TGAColor color_;
  float alpha_;

 public:
  TransparentObject(const TGAColor& color = TGAColor(100, 150, 255, 128),
                    float alpha = 0.3f);

  void createCube(const Vec3f& center, float size);

  int nverts() const { return vertices_.size(); }
  int nfaces() const { return faces_.size(); }
  Vec3f vert(int i) const { return vertices_[i]; }
  std::vector<int> face(int i) const { return faces_[i]; }
  TGAColor getColor() const { return color_; }
  float getAlpha() const { return alpha_; }

  void setColor(const TGAColor& color) { color_ = color; }
  void setAlpha(float alpha) { alpha_ = alpha; }
};

#endif  // __TRANSPARENT_OBJECT_H__
