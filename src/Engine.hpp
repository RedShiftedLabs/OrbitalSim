#pragma once

#include <raylib.h>
#include <raymath.h>

class Engine {
public:
  void run();

private:
  void setup();
  void update();
  void draw();

public:
  const int W = 1024;
  const int H = 768;
  Vector2 center = {(float)W / 2, (float)H / 2};
  Vector2 pos = {200, 0};
  Vector2 vel = {0, 150};
  float GM = 5000000.0f;
  float r = 10.0f;
};