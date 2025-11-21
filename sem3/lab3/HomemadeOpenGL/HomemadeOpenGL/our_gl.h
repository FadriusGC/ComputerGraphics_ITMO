#ifndef __OUR_GL_H__
#define __OUR_GL_H__

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

// ”бираем extern объ€влени€ глобальных матриц, так как они теперь в классе
// Camera extern Matrix ModelView; extern Matrix Viewport; extern Matrix
// Projection;

// ќбъ€вл€ем функции, которые теперь реализованы в классе Camera
// Ёти функции € удалю как только полностью перееду на класс камеры
Matrix viewport(int x, int y, int w, int h);
Matrix projection(float coeff = 0.f);
Matrix lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
  virtual ~IShader() {}
  virtual bool fragment(Vec3f bar, TGAColor& color) = 0;
};

void triangle(Vec3i* pts, IShader& shader, TGAImage& image, TGAImage& zbuffer);

#endif  //__OUR_GL_H__
