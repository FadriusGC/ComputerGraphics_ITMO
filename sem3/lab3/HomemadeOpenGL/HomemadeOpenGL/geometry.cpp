#include "geometry.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

template <>
template <>
Vec3<int>::Vec3(const Vec3<float>& v)
    : x(int(v.x + 0.5f)), y(int(v.y + 0.5f)), z(int(v.z + 0.5f)) {}

template <>
template <>
Vec3<float>::Vec3(const Vec3<int>& v) : x(v.x), y(v.y), z(v.z) {}

// Конструктор матрицы
Matrix::Matrix(int r, int c)
    : m(std::vector<std::vector<float> >(
          r, std::vector<float>(c, 0.f))),  // Инициализация нулями
      rows(r),
      cols(c) {}

// Методы доступа к размерам матрицы
int Matrix::nrows() { return rows; }
int Matrix::ncols() { return cols; }

// Создание единичной матрицы
Matrix Matrix::identity(int dimensions) {
  Matrix E(dimensions, dimensions);
  for (int i = 0; i < dimensions; i++) {
    for (int j = 0; j < dimensions; j++) {
      E[i][j] = (i == j ? 1.f : 0.f);  // 1 на диагонали, 0 в остальных
    }
  }
  return E;
}

// Оператор доступа к строке матрицы
std::vector<float>& Matrix::operator[](const int i) {
  assert(i >= 0 && i < rows);
  return m[i];
}

// Умножение матриц
Matrix Matrix::operator*(const Matrix& a) {
  assert(cols == a.rows);  // Проверка совместимости размеров
  Matrix result(rows, a.cols);

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < a.cols; j++) {
      result.m[i][j] = 0.f;
      for (int k = 0; k < cols; k++) {
        result.m[i][j] += m[i][k] * a.m[k][j];  // Сумма произведений
      }
    }
  }
  return result;
}

// Транспонирование матрицы
Matrix Matrix::transpose() {
  Matrix result(cols, rows);
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++) result[j][i] = m[i][j];
  return result;
}

// Обращение матрицы (метод Гаусса-Жордана)
Matrix Matrix::inverse() {
  assert(rows == cols);  // Только для квадратных матриц

  // Дополняем матрицу единичной матрицей: [A|I]
  Matrix result(rows, cols * 2);
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
      result[i][j] = m[i][j];  // Копируем исходную матрицу

  for (int i = 0; i < rows; i++)
    result[i][i + cols] = 1;  // Добавляем единичную матрицу справа

  // Прямой ход метода Гаусса
  for (int i = 0; i < rows - 1; i++) {
    // Нормализуем текущую строку
    for (int j = result.cols - 1; j >= 0; j--) result[i][j] /= result[i][i];

    // Вычитаем из остальных строк
    for (int k = i + 1; k < rows; k++) {
      float coeff = result[k][i];
      for (int j = 0; j < result.cols; j++) {
        result[k][j] -= result[i][j] * coeff;
      }
    }
  }

  // Нормализуем последнюю строку
  for (int j = result.cols - 1; j >= rows - 1; j--)
    result[rows - 1][j] /= result[rows - 1][rows - 1];

  // Обратный ход метода Гаусса
  for (int i = rows - 1; i > 0; i--) {
    for (int k = i - 1; k >= 0; k--) {
      float coeff = result[k][i];
      for (int j = 0; j < result.cols; j++) {
        result[k][j] -= result[i][j] * coeff;
      }
    }
  }

  // Отсекаем единичную матрицу, оставляя только обратную
  Matrix truncate(rows, cols);
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++) truncate[i][j] = result[i][j + cols];

  return truncate;
}

std::ostream& operator<<(std::ostream& s, Matrix& m) {
  for (int i = 0; i < m.nrows(); i++) {
    for (int j = 0; j < m.ncols(); j++) {
      s << m[i][j];
      if (j < m.ncols() - 1) s << "\t";
    }
    s << "\n";
  }
  return s;
}
