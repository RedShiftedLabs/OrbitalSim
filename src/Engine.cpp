#include "Engine.hpp"

void Engine::setup() {
  // Initialize simulation state here
}

void Engine::update() {
  float dt = GetFrameTime();
  Vector2 rVec = Vector2Scale(pos, -1.0f);
  float distance = Vector2Length(rVec);

  if (distance > 5.0f) {
    Vector2 forceDir = Vector2Normalize(rVec);
    float accelMag = GM / (distance * distance);
    Vector2 acceleration = Vector2Scale(forceDir, accelMag);

    // Semi-implicit Euler integration
    vel = Vector2Add(vel, Vector2Scale(acceleration, dt));
    pos = Vector2Add(pos, Vector2Scale(vel, dt));
  }
}

void Engine::draw() {
  BeginDrawing();
  ClearBackground((Color){10, 10, 10, 255});

  DrawCircleV(center, 20, YELLOW);

  Vector2 planetScreenPos = Vector2Add(center, pos);
  DrawCircleV(planetScreenPos, r, SKYBLUE);

  DrawFPS(10, 10);
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