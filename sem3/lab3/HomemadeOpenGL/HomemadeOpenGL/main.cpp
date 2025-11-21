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
Vec3f light_dir(0, 0, -1);

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

void triangle(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2,
              TGAImage& image, float intensity, int* zbuffer) {
  if (t0.y == t1.y && t0.y == t2.y) return;

  if (t0.y > t1.y) {
    std::swap(t0, t1);
    std::swap(uv0, uv1);
  }
  if (t0.y > t2.y) {
    std::swap(t0, t2);
    std::swap(uv0, uv2);
  }
  if (t1.y > t2.y) {
    std::swap(t1, t2);
    std::swap(uv1, uv2);
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
    Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
    Vec2i uvB =
        second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;

    if (A.x > B.x) {
      std::swap(A, B);
      std::swap(uvA, uvB);
    }

    for (int j = A.x; j <= B.x; j++) {
      float phi = B.x == A.x ? 1. : (float)(j - A.x) / (float)(B.x - A.x);
      Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
      Vec2i uvP = uvA + (uvB - uvA) * phi;

      int idx = P.x + P.y * width;
      if (idx >= 0 && idx < width * height && zbuffer[idx] < P.z) {
        zbuffer[idx] = P.z;
        TGAColor color = model->diffuse(uvP);
        image.set(P.x, P.y,
                  TGAColor(color.r * intensity, color.g * intensity,
                           color.b * intensity));
      }
    }
  }
}

void renderScene(const std::string& output_filename, Camera& camera) {
  std::cout << "Rendering: " << output_filename << std::endl;

  zbuffer = new int[width * height];
  for (int i = 0; i < width * height; i++) {
    zbuffer[i] = std::numeric_limits<int>::min();
  }

  Matrix ViewPort =
      viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

  Matrix View = camera.getViewMatrix();
  Matrix Projection = camera.getProjectionMatrix();

  std::cout << "View matrix created" << std::endl;
  std::cout << "Projection matrix created" << std::endl;

  TGAImage image(width, height, TGAImage::RGB);
  std::cout << "TGAImage created: " << width << "x" << height << std::endl;

  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);
    Vec3i screen_coords[3];
    Vec3f world_coords[3];

    for (int j = 0; j < 3; j++) {
      Vec3f v = model->vert(face[j]);

      screen_coords[j] = m2v(ViewPort * Projection * v2m(v));
      world_coords[j] = v;
    }

    Vec3f n = (world_coords[2] - world_coords[0]) ^
              (world_coords[1] - world_coords[0]);
    n.normalize();
    float intensity = n * light_dir;

    // Back-face culling
    if (intensity > 0) {
      Vec2i uv[3];
      for (int k = 0; k < 3; k++) {
        uv[k] = model->uv(i, k);
      }
      triangle(screen_coords[0], screen_coords[1], screen_coords[2], uv[0],
               uv[1], uv[2], image, intensity, zbuffer);
    }
  }

  image.flip_vertically();
  image.write_tga_file("output_front.tga");

  // Z-buffer image
  TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
  for (int i = 0; i < width; i++) {
    for (int j = 0; j < height; j++) {
      int zval = zbuffer[i + j * width];
      if (zval > std::numeric_limits<int>::min()) {
        zbimage.set(i, j, TGAColor(zval, 1));
      }
    }
  }
  zbimage.flip_vertically();
  zbimage.write_tga_file("zbuffer2.tga");

  delete[] zbuffer;
  return;
}

int main(int argc, char** argv) {
  std::cout << "Starting renderer..." << std::endl;

  // Load model
  const char* model_path = "obj/african_head.obj";
  if (argc >= 2) {
    model_path = argv[1];
  }

  std::cout << "Loading model: " << model_path << std::endl;
  model = new Model(model_path);

  if (model->nverts() == 0) {
    std::cout << "ERROR: Failed to load model!" << std::endl;
    return 1;
  }

  std::cout << "Model loaded: " << model->nverts() << " vertices, "
            << model->nfaces() << " faces" << std::endl;

  float aspect = (float)width / (float)height;

  std::cout << "\n=== Camera 1: Front view ===" << std::endl;
  Camera camera1(Vec3f(0, 0, 3), Vec3f(0, 0, 0), Vec3f(0, 1, 0), 45.0f, aspect);
  renderScene("output_front.tga", camera1);

  delete model;
  std::cout << "Done!" << std::endl;
  return 0;
}
