#include "renderer_ui.h"
#define NOMINMAX
#include <Windows.h>

RendererUI::RendererUI() : total_faces_(0), processed_faces_(0) {}

void RendererUI::localise() {
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  return;
}

void RendererUI::printHeader() {
  std::cout << u8"==============================\n";
  std::cout << u8"       My little OpenGL\n";
  std::cout << u8"   Made by: Grishin Daniil\n";
  std::cout << u8"==============================\n";
}

ShaderType RendererUI::selectShader() {
  std::cout << u8"\nВыберите метод рендеринга:\n";
  std::cout << u8"1. Diffuse Lighting\n";
  std::cout << u8"2. Gouraud Shading\n";
  std::cout << u8"3. Phong Shading\n";
  std::cout << u8"Введите номер (1-3): ";

  int choice;
  std::cin >> choice;

  while (choice < 1 || choice > 3) {
    std::cout << u8"Неверный выбор! Введите число от 1 до 3: ";
    std::cin >> choice;
  }
  std::cout << u8"\n==============================\n";
  return static_cast<ShaderType>(choice);
}

void RendererUI::showModelInfo(Model* model) {
  std::cout << u8"\n==============================\n";
  std::cout << u8"Модель загружена успешно!\n";
  std::cout << u8"Статистика модели:\n";
  std::cout << u8"• Вершин: " << model->nverts() << "\n";
  std::cout << u8"• Полигонов: " << model->nfaces() << "\n";
  std::cout << u8"\n==============================";
}

void RendererUI::showRenderSettings(ShaderType shader, int width, int height) {
  std::cout << u8"\nНастройки рендеринга:\n";

  switch (shader) {
    case DIFFUSE:
      std::cout << u8"• Метод: Diffuse Lighting\n";
      std::cout << u8"• Освещение: Модель Ламберта\n";
      break;
    case GOURAUD:
      std::cout << u8"• Метод: Gouraud Shading\n";
      std::cout << u8"• Интерполяция: По вершинам\n";
      break;
    case PHONG:
      std::cout << u8"• Метод: Phong Shading\n";
      std::cout << u8"• Интерполяция: По пикселям\n";
      break;
  }
  std::cout << u8"• Разрешение: " << width << u8"x" << height << u8"\n";
}

void RendererUI::startRender(int total_faces) {
  total_faces_ = total_faces;
  processed_faces_ = 0;
  start_time_ = std::chrono::high_resolution_clock::now();

  std::cout << u8"\n         Поехали!\n";
}

void RendererUI::updateProgress(int current_face) {
  processed_faces_ = current_face;
  float progress = static_cast<float>(current_face) / total_faces_;

  // Показываем прогресс каждые 2% или для последних 10 треугольников
  if (current_face % (total_faces_ / 50) == 0 ||
      current_face >= total_faces_ - 10) {
    showProgressBar(progress);
  }
}

void RendererUI::finishRender() {
  showProgressBar(1.0f);
  std::cout << u8"\n";
}

void RendererUI::showStatistics(Model* model, long long render_time_ms) {
  std::cout << u8"\n==============================";
  std::cout << u8"\nСтатистика рендеринга:\n";
  std::cout << u8"• Время рендеринга: " << render_time_ms << u8" мс\n";
  std::cout << u8"• Скорость: " << (model->nfaces() * 1000.0 / render_time_ms)
            << u8" полигонов/сек\n";
  std::cout << u8"• Файлы сохранены: output.tga, zbuffer.tga\n";

  std::cout << u8"\nРендеринг завершен успешно!\n";
  std::cout << u8"==============================\n";
}

void RendererUI::showProgressBar(float progress, int barWidth) {
  std::cout << u8"[";
  int pos = barWidth * progress;
  for (int i = 0; i < barWidth; ++i) {
    if (i < pos)
      std::cout << u8"█";
    else if (i == pos)
      std::cout << u8">";
    else
      std::cout << u8" ";
  }
  std::cout << u8"] " << int(progress * 100.0) << " %\r";
  std::cout.flush();
}
