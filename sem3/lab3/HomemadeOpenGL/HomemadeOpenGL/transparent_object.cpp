#include "transparent_object.h"

#include <cmath>

TransparentObject::TransparentObject(const TGAColor& color, float alpha)
    : color_(color), alpha_(alpha) {}

void TransparentObject::createCube(const Vec3f& center, float size) {
  vertices_.clear();
  faces_.clear();

  float half = size / 2.0f;

  vertices_ = {Vec3f(center.x - half, center.y - half, center.z - half),
               Vec3f(center.x + half, center.y - half, center.z - half),
               Vec3f(center.x + half, center.y + half, center.z - half),
               Vec3f(center.x - half, center.y + half, center.z - half),
               Vec3f(center.x - half, center.y - half, center.z + half),
               Vec3f(center.x + half, center.y - half, center.z + half),
               Vec3f(center.x + half, center.y + half, center.z + half),
               Vec3f(center.x - half, center.y + half, center.z + half)};

  faces_ = {{0, 1, 2}, {0, 2, 3},

            {4, 6, 5}, {4, 7, 6},

            {0, 3, 7}, {0, 7, 4},

            {1, 5, 6}, {1, 6, 2},

            {3, 2, 6}, {3, 6, 7},

            {0, 4, 5}, {0, 5, 1}};
}
