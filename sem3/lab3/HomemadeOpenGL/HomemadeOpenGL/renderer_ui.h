#ifndef __RENDERER_UI_H__
#define __RENDERER_UI_H__
#define NOMINMAX
#include <Windows.h>

#include <chrono>
#include <iostream>

#include "model.h"

enum ShaderType { DIFFUSE = 1, GOURAUD = 2, PHONG = 3 };

class RendererUI {
 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;
  int total_faces_;
  int processed_faces_;

 public:
  RendererUI();

  void printHeader();
  ShaderType selectShader();
  void showModelInfo(Model* model);
  void showRenderSettings(ShaderType shader, int width, int height);
  void startRender(int total_faces);
  void updateProgress(int current_face);
  void finishRender();
  void showStatistics(Model* model, long long render_time_ms);
  void localise();

 private:
  void showProgressBar(float progress, int barWidth = 25);
};

#endif  // __RENDERER_UI_H__
