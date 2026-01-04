#include "Engine.hpp"
#include <cstddef>
#include <raylib.h>
#include <raymath.h>

void Engine::setup() {
  objects.reserve(4);
  auto Planet =
      objects.emplace_back(Vector2{center.x - 2200.0f, center.y}, // position
                           Vector2{0.0f, -1200.0f},               // velocity
                           8.0f,                                  // radius
                           2.0f,                                  // mass
                           BLUE);
  auto moon = objects.emplace_back(
      Vector2{Planet.pos.x - 90, Planet.pos.y},  // position
      Vector2{Planet.vel.x, Planet.vel.y - 260}, // velocity
      2.0f,                                      // radius
      0.05f,                                     // mass
      ORANGE);
  auto Star = objects.emplace_back(Vector2{center.x, center.y}, // position
                                   Vector2{0.0f, 0.0f},         // velocity
                                   16.0f,                       // radius
                                   800.0f,                      // mass
                                   RED);
}

Vector2 Engine::screenToWorld(Vector2 screenPos) {
  Vector2 fromCenter = Vector2Subtract(screenPos, center);
  Vector2 scaled = Vector2Scale(fromCenter, 1.0f / zoomScale);
  Vector2 worldPos = Vector2Subtract(Vector2Add(scaled, center), cameraOffset);
  return worldPos;
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
    if (IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER) ||
        IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
      zoom(scroll.y * 0.1f);
    } else {
      Vector2 panDelta = {scroll.x * 30.0f, scroll.y * 30.0f};
      panView(panDelta);
    }
  }

  if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) {
    zoom(0.1f);
  }
  if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) {
    zoom(-0.1f);
  }

  if (IsKeyPressed(KEY_ZERO)) {
    zoomScale = 1.0f;
    cameraOffset = {0, 0};
  }

  lastMousePos = mousePos;
}

void Engine::panView(Vector2 delta) {
  cameraOffset = Vector2Add(cameraOffset, delta);
}

void Engine::zoom(float delta) {
  zoomScale += delta;
  if (zoomScale < 0.1f)
    zoomScale = 0.1f;
  if (zoomScale > 5.0f)
    zoomScale = 5.0f;
}

void Engine::mousePressed(int x, int y) {
  Vector2 worldPos = screenToWorld({(float)x, (float)y});

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
    Vector2 worldPos = screenToWorld({(float)x, (float)y});
    objects[draggedObjIndex].pos = worldPos;
    objects[draggedObjIndex].vel = {0, 0};
  }
}

void Engine::mouseReleased(int x, int y) {
  if (isDragging && draggedObjIndex >= 0) {
    Vector2 mousePos = {(float)x, (float)y};
    Vector2 dragVel = Vector2Subtract(mousePos, lastMousePos);
    objects[draggedObjIndex].vel = Vector2Scale(dragVel, 10.0f / zoomScale);
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
    Vector2 worldOffset = Vector2Add(obj.pos, cameraOffset);
    Vector2 fromCenter = Vector2Subtract(worldOffset, center);
    Vector2 scaled = Vector2Scale(fromCenter, zoomScale);
    Vector2 screenPos = Vector2Add(scaled, center);

    float screenRadius = obj.radius * zoomScale;
    DrawCircleV(screenPos, screenRadius, obj.color);
  }

  DrawFPS(10, 10);
  DrawText(TextFormat("Zoom: %.1fx", zoomScale), 10, 30, 16, WHITE);
  DrawText("Scroll: Pan | Cmd+Scroll: Zoom | +/-: Zoom | 0: Reset", 10, H - 30,
           14, GRAY);
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