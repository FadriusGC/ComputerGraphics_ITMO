#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "camera.h"
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

const int width = 800;
const int height = 800;
const int depth = 255;

Model* model = NULL;
int* zbuffer = NULL;
Vec3f light_dir = Vec3f(1, -1, 1).normalize();  // Как в статье

Vec3f m2v(Matrix m) {
  return Vec3f(m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]);
}

Matrix v2m(Vec3f v) {
  Matrix m(4, 1);
  m[0][0] = v.x;
  m[1][0] = v.y;
  m[2][0] = v.z;
  m[3][0] = 1.f;
  return m;
}

Matrix viewport(int x, int y, int w, int h) {
  Matrix m = Matrix::identity(4);
  m[0][3] = x + w / 2.f;
  m[1][3] = y + h / 2.f;
  m[2][3] = depth / 2.f;
  m[0][0] = w / 2.f;
  m[1][1] = h / 2.f;
  m[2][2] = depth / 2.f;
  return m;
}

// Упрощенная функция треугольника как в статье
void triangle(Vec3i t0, Vec3i t1, Vec3i t2, float ity0, float ity1, float ity2,
              TGAImage& image, int* zbuffer) {
  if (t0.y == t1.y && t0.y == t2.y) return;
  if (t0.y > t1.y) {
    std::swap(t0, t1);
    std::swap(ity0, ity1);
  }
  if (t0.y > t2.y) {
    std::swap(t0, t2);
    std::swap(ity0, ity2);
  }
  if (t1.y > t2.y) {
    std::swap(t1, t2);
    std::swap(ity1, ity2);
  }

  int total_height = t2.y - t0.y;
  for (int i = 0; i < total_height; i++) {
    bool second_half = i > t1.y - t0.y || t1.y == t0.y;
    int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
    float alpha = (float)i / total_height;
    float beta = (float)(i - (second_half ? t1.y - t0.y : 0)) / segment_height;
    Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
    Vec3i B =
        second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
    float ityA = ity0 + (ity2 - ity0) * alpha;
    float ityB =
        second_half ? ity1 + (ity2 - ity1) * beta : ity0 + (ity1 - ity0) * beta;
    if (A.x > B.x) {
      std::swap(A, B);
      std::swap(ityA, ityB);
    }
    for (int j = A.x; j <= B.x; j++) {
      float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
      Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
      float ityP = ityA + (ityB - ityA) * phi;
      int idx = P.x + P.y * width;
      if (P.x >= width || P.y >= height || P.x < 0 || P.y < 0) continue;
      if (zbuffer[idx] < P.z) {
        zbuffer[idx] = P.z;
        // Исправляем создание цвета - используем конструктор с тремя
        // параметрами
        TGAColor color = TGAColor(255, 255, 255) * ityP;
        image.set(P.x, P.y, color);
      }
    }
  }
}

int main(int argc, char** argv) {
  std::cout << "Starting renderer..." << std::endl;

  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  if (model->nverts() == 0) {
    std::cout << "ERROR: Failed to load model!" << std::endl;
    return 1;
  }

  std::cout << "Model loaded: " << model->nverts() << " vertices, "
            << model->nfaces() << " faces" << std::endl;

  zbuffer = new int[width * height];
  for (int i = 0; i < width * height; i++) {
    zbuffer[i] = std::numeric_limits<int>::min();
  }

  {  // draw the model
    Vec3f eye(0, 0, 3);
    Vec3f center(0, 0, 0);
    Vec3f up(0, 1, 0);

    Camera camera(eye, center, up);
    Matrix View = camera.getViewMatrix();
    Matrix Projection = Matrix::identity(4);
    Projection[3][2] = -1.f / (eye - center).norm();
    Matrix ViewPort =
        viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

    TGAImage image(width, height, TGAImage::RGB);

    std::cout << "Rendering..." << std::endl;
    for (int i = 0; i < model->nfaces(); i++) {
      std::vector<int> face = model->face(i);
      Vec3i screen_coords[3];
      float intensity[3];

      for (int j = 0; j < 3; j++) {
        Vec3f v = model->vert(face[j]);
        screen_coords[j] = m2v(ViewPort * Projection * View * v2m(v));

        // Используем новый метод norm из модели
        Vec3f n = model->norm(i, j);
        intensity[j] = n * light_dir;
        if (intensity[j] < 0) intensity[j] = 0;
        if (intensity[j] > 1) intensity[j] = 1;
      }

      // Отладочная информация для первых треугольников
      if (i < 5) {
        std::cout << "Triangle " << i << " intensities: " << intensity[0]
                  << ", " << intensity[1] << ", " << intensity[2] << std::endl;
      }

      triangle(screen_coords[0], screen_coords[1], screen_coords[2],
               intensity[0], intensity[1], intensity[2], image, zbuffer);
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    std::cout << "Saved output.tga" << std::endl;
  }

  {  // dump z-buffer
    TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        zbimage.set(i, j, TGAColor(zbuffer[i + j * width]));
      }
    }
    zbimage.flip_vertically();
    zbimage.write_tga_file("zbuffer.tga");
    std::cout << "Saved zbuffer.tga" << std::endl;
  }

  delete model;
  delete[] zbuffer;
  std::cout << "Done!" << std::endl;
  return 0;
}
