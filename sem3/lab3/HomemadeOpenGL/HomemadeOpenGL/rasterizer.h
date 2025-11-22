#ifndef __RASTERIZER_H__
#define __RASTERIZER_H__

#include <algorithm>
#include <limits>

#include "geometry.h"
#include "shader.h"
#include "tgaimage.h"

class Rasterizer {
 private:
  TGAImage& image_;
  TGAImage& zbuffer_;

  Vec3f barycentric(Vec3i A, Vec3i B, Vec3i C, Vec3i P);

 public:
  Rasterizer(TGAImage& image, TGAImage& zbuffer);
  void triangle(Vec3i* pts, IShader& shader, bool transparent = false);
};

#endif  // __RASTERIZER_H__
