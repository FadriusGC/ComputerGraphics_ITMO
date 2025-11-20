#include "geometry.h"

// Специализация для преобразования Vec3<float> -> Vec3<int> с округлением
template <>
template <>
Vec3<int>::Vec3(const Vec3<float>& v)
    : x(int(v.x + 0.5f)), y(int(v.y + 0.5f)), z(int(v.z + 0.5f)) {}

// Специализация для преобразования Vec3<int> -> Vec3<float>
template <>
template <>
Vec3<float>::Vec3(const Vec3<int>& v) : x(v.x), y(v.y), z(v.z) {}
