#include "Engine.hpp"
#include <cstddef>
#include <raylib.h>
#include <raymath.h>

void Engine::setup() {
  objects.reserve(2);
  objects.emplace_back(Vector2{center.x - 400.0f, center.y}, // position
                       Vector2{0.0f, -500.0f},               // velocity
                       8.0f,                                 // radius
                       2.0f,                                 // mass
                       BLUE);
  objects.emplace_back(Vector2{center.x + 400.0f, center.y}, // position
                       Vector2{0.0f, 500.0f},                // velocity
                       8.0f,                                 // radius
                       1.0f,                                 // mass
                       GREEN);
  objects.emplace_back(Vector2{center.x, center.y}, // position
                       Vector2{0.0f, 0.0f},         // velocity
                       16.0f,                       // radius
                       80.0f,                       // mass
                       RED);
}

void Engine::processInput() {
  Vector2 mousePos = GetMousePosition();

  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    mousePressed((int)mousePos.x, (int)mousePos.y);
  }

  if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
    mouseReleased((int)mousePos.x, (int)mousePos.y);
  }

  if (isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
    mouseDragged((int)mousePos.x, (int)mousePos.y);
  }

  Vector2 scroll = GetMouseWheelMoveV();
  if (scroll.x != 0 || scroll.y != 0) {
    Vector2 panDelta = {scroll.x * 30.0f, scroll.y * 30.0f};
    panView(panDelta);
  }

  lastMousePos = mousePos;
}

void Engine::panView(Vector2 delta) {
  cameraOffset = Vector2Add(cameraOffset, delta);
}

void Engine::mousePressed(int x, int y) {
  Vector2 worldPos = {(float)x - cameraOffset.x, (float)y - cameraOffset.y};

  for (size_t i = 0; i < objects.size(); i++) {
    float dist = Vector2Distance(worldPos, objects[i].pos);
    if (dist <= objects[i].radius) {
      isDragging = true;
      draggedObjIndex = (int)i;
      break;
    }
  }
}

void Engine::mouseDragged(int x, int y) {
  if (isDragging && draggedObjIndex >= 0) {
    Vector2 worldPos = {(float)x - cameraOffset.x, (float)y - cameraOffset.y};
    objects[draggedObjIndex].pos = worldPos;
    objects[draggedObjIndex].vel = {0, 0};
  }
}

void Engine::mouseReleased(int x, int y) {
  if (isDragging && draggedObjIndex >= 0) {
    Vector2 mousePos = {(float)x, (float)y};
    Vector2 dragVel = Vector2Subtract(mousePos, lastMousePos);
    objects[draggedObjIndex].vel = Vector2Scale(dragVel, 10.0f);
  }
  isDragging = false;
  draggedObjIndex = -1;
}

void Engine::update() {
  float dt = GetFrameTime();
  const float eps = 1e-4f;

  processInput();

  const size_t n = objects.size();

  std::vector<Vector2> accel(n, Vector2{0.0f, 0.0f});

  for (size_t i = 0; i < n; i++) {
    for (size_t j = i + 1; j < n; j++) {
      Vector2 r = objects[i].pos - objects[j].pos;
      float distSq = Vector2LengthSqr(r) + eps;

      float invDist = -1.0f / sqrtf(distSq);
      Vector2 dir = Vector2Scale(r, invDist);

      float forceMag =
          GM * objects[i].mass * objects[j].mass * invDist * invDist;

      Vector2 force = Vector2Scale(dir, forceMag);
      accel[i] =
          Vector2Add(accel[i], Vector2Scale(force, 1.0f / objects[i].mass));

      accel[j] =
          Vector2Add(accel[j], Vector2Scale(force, -1.0f / objects[j].mass));
    }
  }

  for (size_t i = 0; i < n; i++) {
    if (isDragging && (int)i == draggedObjIndex)
      continue;

    objects[i].vel = Vector2Add(objects[i].vel, Vector2Scale(accel[i], dt));
    objects[i].pos =
        Vector2Add(objects[i].pos, Vector2Scale(objects[i].vel, dt));
  }
}

void Engine::draw() {
  BeginDrawing();
  ClearBackground((Color){10, 10, 10, 255});

  for (const auto &obj : objects) {
    Vector2 screenPos = Vector2Add(obj.pos, cameraOffset);
    DrawCircleV(screenPos, obj.radius, obj.color);
  }

  DrawFPS(10, 10);
  DrawText("Click: Drag objects | Two-finger scroll: Pan view", 10, 30, 16,
           GRAY);
  EndDrawing();
}

void Engine::run() {
  InitWindow(W, H, "Orbital");
  SetTargetFPS(60);

  setup();

  while (!WindowShouldClose()) {
    update();
    draw();
  }

  CloseWindow();
}