#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>

#include "geometry.h"
#include "model.h"
#include "our_gl.h"
#include "tgaimage.h"

const int width = 800;
const int height = 800;
Model* model = NULL;

TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

Vec3f light_dir = Vec3f(1, 1, 1).normalize();

Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);

struct Shader : public IShader {
  virtual ~Shader() {}

  Vec2i varying_uv[3];
  float varying_inty[3];

  // Фрагментный шейдер - вызывается для каждого пикселя внутри треугольника
  // bar - баррицентрические координаты текущего пикселя
  // color - выходной цвет пикселя (результат работы шейдера)
  virtual bool fragment(Vec3f bar, TGAColor& color) {
    // Интерполяция текстурных координат через баррицентрические координаты,
    // каждая компонента bar (x, y, z) представляет вес соответствующей вершины.
    // В конечном итоге получаем текстурные координаты для текущего пикселя

    Vec2i uv =
        varying_uv[0] * bar.x + varying_uv[1] * bar.y + varying_uv[2] * bar.z;

    // Расчет освещения по модели Ламберта
    // model->norm(uv) получает нормаль из карты нормалей в текселе uv
    // light_dir - направление на источник света
    // Скалярное произведение дает косинус угла между нормалью и светом
    float inty = model->norm(uv) * light_dir;

    color = model->diffuse(uv) * inty;

    return false;
  }
};

int main(int argc, char** argv) {
  if (2 == argc) {
    model = new Model(argv[1]);
  } else {
    model = new Model("obj/african_head.obj");
  }

  // настройка матриц
  // Матрица вида (ModelView) - определяет положение и ориентацию камеры
  // eye - позиция камеры, center - точка на которую смотрим, Vec3f(0,1,0) -
  // вектор "вверх"
  lookat(eye, center, Vec3f(0, 1, 0));

  // Матрица вьюпорта - преобразует из нормализованных координат в экранные
  // Аргументы: начальная позиция (x,y) и размеры (width,height) области вывода
  viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);

  // Матрица проекции - применяет перспективное искажение
  // Коэффициент -1/focal_length создает перспективный эффект
  projection(-1.f / (eye - center).norm());

  TGAImage image(width, height, TGAImage::RGB);

  // Создание экземпляра шейдера
  Shader shader;

  // по сути главный цикел рендера
  // идем по всем треугольникам
  for (int i = 0; i < model->nfaces(); i++) {
    std::vector<int> face = model->face(i);

    Vec3i screen_coords[3];  // Координаты в экранном пространстве (после всех
                             // преобразований)
    Vec3f world_coords[3];   // Координаты в мировом пространстве (оригинальные)

    // Обрабатываем три вершины треугольника
    for (int j = 0; j < 3; j++) {
      // Получаем мировые координаты j-й вершины треугольника
      Vec3f v = model->vert(face[j]);

      // Преобразуем вершину в экранные координаты:
      // матричные преобразования:
      // 1. ModelView - в пространство камеры
      // 2. Projection - перспективное преобразование
      // 3. Viewport - в экранные координаты
      screen_coords[j] = Vec3f(Viewport * Projection * ModelView * Matrix(v));

      // Сохраняем оригинальные мировые координаты
      world_coords[j] = v;

      // Интенсивность освещения для вершины
      // model->norm(i, j) - нормаль вершины из модели
      // Умножение на light_dir дает косинус угла между нормалью и светом
      shader.varying_inty[j] = model->norm(i, j) * light_dir;

      // Текстурные координаты для вершины
      // model->uv(i, j) возвращает координаты текстуры для j-й вершины i-го
      // полигона
      shader.varying_uv[j] = model->uv(i, j);
    }

    // вызываем растеризатор
    // screen_coords - вершины треугольника в экранных координатах
    // shader - шейдер для расчета цвета каждого пикселя
    // image - целевое изображение для отрисовки
    // zbuffer - буфер глубины для корректного отображения перекрывающихся
    // объектов
    triangle(screen_coords, shader, image, zbuffer);
  }

  image.flip_vertically();
  image.write_tga_file("output.tga");
  zbuffer.flip_vertically();
  zbuffer.write_tga_file("zbuffer.tga");
  delete model;
  return 0;
}
