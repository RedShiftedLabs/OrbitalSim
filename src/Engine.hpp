#pragma once

#include <raylib.h>
#include <raymath.h>
#include <vector>

struct Obj {
  Vector2 pos{};
  Vector2 vel{};
  float radius{5.0f};
  float mass{1.0f};
  Color color;

  Obj() = default;

  Obj(Vector2 p, Vector2 v, float r, float m, Color c)
      : pos(p), vel(v), radius(r), mass(m), color(c) {}
};

class Engine {
public:
  void run();

private:
  void setup();
  void update();
  void draw();
  void processInput();

  void mouseDragged(int x, int y);
  void mousePressed(int x, int y);
  void mouseReleased(int x, int y);
  void panView(Vector2 delta);

public:
  const int W = 1024;
  const int H = 768;
  Vector2 center = {(float)W / 2, (float)H / 2};

  std::vector<Obj> objects;
  float GM = 5000000.0f;

  Vector2 cameraOffset = {0, 0};
  bool isDragging = false;
  int draggedObjIndex = -1;
  Vector2 lastMousePos = {0, 0};
  bool isPanning = false;
};