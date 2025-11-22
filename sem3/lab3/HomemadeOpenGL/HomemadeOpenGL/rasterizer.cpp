#include "rasterizer.h"

Rasterizer::Rasterizer(TGAImage& image, TGAImage& zbuffer)
    : image_(image), zbuffer_(zbuffer) {}

Vec3f Rasterizer::barycentric(Vec3i A, Vec3i B, Vec3i C, Vec3i P) {
  Vec3f u = Vec3f(C.x - A.x, B.x - A.x, A.x - P.x) ^
            Vec3f(C.y - A.y, B.y - A.y, A.y - P.y);

  if (std::abs(u.z) > 0.5f) {
    return Vec3f(1.0f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
  }
  return Vec3f(-1.0f, 1.0f, 1.0f);
}

void Rasterizer::triangle(Vec3i* pts, IShader& shader, bool transparent) {
  Vec2i bboxmin(std::numeric_limits<int>::max(),
                std::numeric_limits<int>::max());
  Vec2i bboxmax(-std::numeric_limits<int>::max(),
                -std::numeric_limits<int>::max());
  Vec2i clamp(image_.get_width() - 1, image_.get_height() - 1);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      bboxmin[j] = std::max(0, std::min(bboxmin[j], pts[i][j]));
      bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
    }
  }

  Vec3i P;
  for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
    for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
      Vec3f bc = barycentric(pts[0], pts[1], pts[2], P);

      if (bc.x < 0 || bc.y < 0 || bc.z < 0) continue;

      P.z = static_cast<int>(pts[0].z * bc.x + pts[1].z * bc.y +
                             pts[2].z * bc.z + 0.5f);
      P.z = std::max(0, std::min(255, P.z));

      TGAColor color;
      bool discard = shader.fragment(bc, color);

      if (!discard) {
        if (transparent) {
          // Для прозрачных объектов рисуем поверх
          image_.set(P.x, P.y, color);
        } else {
          // Для непрозрачных проверяем з буффер
          if (zbuffer_.get(P.x, P.y)[0] <= P.z) {
            zbuffer_.set(P.x, P.y, TGAColor(static_cast<unsigned char>(P.z)));
            image_.set(P.x, P.y, color);
          }
        }
      }
    }
  }
}
