#pragma once

#include <raylib.h>
#include <raymath.h>

class Engine3D {
public:
  void run();

private:
  void setup();
  void update();
  void updateCamera();
  void draw();
  void processInput();

  // Input event callbacks
  void keyPressed(int key);
  void mouseMoved();
  void mouseScrolled(float scrollY);

public:
  const int W = 1024;
  const int H = 768;

  // Camera
  Camera3D camera = {};

  // Orbital body state
  Vector3 pos = {200.0f, 0.0f, 0.0f};
  Vector3 vel = {0.0f, 0.0f, 150.0f};

  // Physics constants
  float GM = 5000000.0f;
  float bodyRadius = 10.0f;
  float sunRadius = 20.0f;

  // Camera settings
  float cameraSpeed = 200.0f;
  float mouseSensitivity = 0.003f;
};
