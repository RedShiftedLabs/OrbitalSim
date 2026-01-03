#include "Engine3D.hpp"

void Engine3D::setup() {
  // Setup camera
  camera.position = {0.0f, 400.0f, 400.0f};
  camera.target = {0.0f, 0.0f, 0.0f};
  camera.up = {0.0f, 1.0f, 0.0f};
  camera.fovy = 60.0f;
  camera.projection = CAMERA_PERSPECTIVE;

  // Disable cursor for free camera mode
  DisableCursor();

  // Setup lighting
  setupLighting();
}

void Engine3D::setupLighting() {
  // Load lighting shader
  lightingShader = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");

  // Get shader uniform locations
  lightPosLoc = GetShaderLocation(lightingShader, "lightPos");
  viewPosLoc = GetShaderLocation(lightingShader, "viewPos");
  lightColorLoc = GetShaderLocation(lightingShader, "lightColor");
  objectColorLoc = GetShaderLocation(lightingShader, "objectColor");
  ambientLoc = GetShaderLocation(lightingShader, "ambientStrength");

  // Set shader's MVP matrix location
  lightingShader.locs[SHADER_LOC_MATRIX_MODEL] =
      GetShaderLocation(lightingShader, "matModel");
  lightingShader.locs[SHADER_LOC_MATRIX_MVP] =
      GetShaderLocation(lightingShader, "mvp");
  lightingShader.locs[SHADER_LOC_MATRIX_NORMAL] =
      GetShaderLocation(lightingShader, "matNormal");

  // Create sphere models
  Mesh planetMesh = GenMeshSphere(bodyRadius, 32, 32);
  planetModel = LoadModelFromMesh(planetMesh);
  planetModel.materials[0].shader = lightingShader;

  Mesh sunMesh = GenMeshSphere(sunRadius, 32, 32);
  sunModel = LoadModelFromMesh(sunMesh);

  // Set light color (sun is bright yellow-white)
  float lightColor[4] = {1.0f, 0.95f, 0.8f, 1.0f};
  SetShaderValue(lightingShader, lightColorLoc, lightColor,
                 SHADER_UNIFORM_VEC4);

  // Set ambient strength
  float ambient = 0.1f;
  SetShaderValue(lightingShader, ambientLoc, &ambient, SHADER_UNIFORM_FLOAT);
}

void Engine3D::processInput() {
  // Poll keyboard events
  for (int key = 0; key < 512; key++) {
    if (IsKeyPressed(key))
      keyPressed(key);
  }

  // Poll mouse movement
  Vector2 mouseDelta = GetMouseDelta();
  if (mouseDelta.x != 0 || mouseDelta.y != 0) {
    mouseMoved();
  }

  // Poll mouse scroll
  float scrollY = GetMouseWheelMove();
  if (scrollY != 0) {
    mouseScrolled(scrollY);
  }
}

void Engine3D::keyPressed(int key) {
  if (key == KEY_R) {
    camera.position = {0.0f, 400.0f, 400.0f};
    camera.target = {0.0f, 0.0f, 0.0f};
  }
}

void Engine3D::mouseMoved() {
  Vector2 mouseDelta = GetMouseDelta();

  Vector3 forward =
      Vector3Normalize(Vector3Subtract(camera.target, camera.position));
  Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));

  // Yaw (horizontal rotation)
  Matrix yawRotation = MatrixRotateY(-mouseDelta.x * mouseSensitivity);
  Vector3 direction = Vector3Subtract(camera.target, camera.position);
  direction = Vector3Transform(direction, yawRotation);

  // Pitch (vertical rotation)
  Matrix pitchRotation = MatrixRotate(right, -mouseDelta.y * mouseSensitivity);
  direction = Vector3Transform(direction, pitchRotation);

  camera.target = Vector3Add(camera.position, direction);
}

void Engine3D::mouseScrolled(float scrollY) {
  camera.fovy -= scrollY * 2.0f;
  if (camera.fovy < 10.0f)
    camera.fovy = 10.0f;
  if (camera.fovy > 120.0f)
    camera.fovy = 120.0f;
}

void Engine3D::updateCamera() {
  float dt = GetFrameTime();

  Vector3 forward =
      Vector3Normalize(Vector3Subtract(camera.target, camera.position));
  Vector3 right = Vector3Normalize(Vector3CrossProduct(forward, camera.up));
  float speed = cameraSpeed * dt;

  if (IsKeyDown(KEY_W)) {
    camera.position = Vector3Add(camera.position, Vector3Scale(forward, speed));
    camera.target = Vector3Add(camera.target, Vector3Scale(forward, speed));
  }
  if (IsKeyDown(KEY_S)) {
    camera.position =
        Vector3Subtract(camera.position, Vector3Scale(forward, speed));
    camera.target =
        Vector3Subtract(camera.target, Vector3Scale(forward, speed));
  }
  if (IsKeyDown(KEY_A)) {
    camera.position =
        Vector3Subtract(camera.position, Vector3Scale(right, speed));
    camera.target = Vector3Subtract(camera.target, Vector3Scale(right, speed));
  }
  if (IsKeyDown(KEY_D)) {
    camera.position = Vector3Add(camera.position, Vector3Scale(right, speed));
    camera.target = Vector3Add(camera.target, Vector3Scale(right, speed));
  }
  if (IsKeyDown(KEY_E)) {
    camera.position.y += speed;
    camera.target.y += speed;
  }
  if (IsKeyDown(KEY_Q)) {
    camera.position.y -= speed;
    camera.target.y -= speed;
  }
}

void Engine3D::update() {
  float dt = GetFrameTime();

  // Process input events
  processInput();

  // Update camera movement
  updateCamera();

  // Update shader uniforms for lighting
  Vector3 sunPos = {0.0f, 0.0f, 0.0f};
  SetShaderValue(lightingShader, lightPosLoc, &sunPos, SHADER_UNIFORM_VEC3);
  SetShaderValue(lightingShader, viewPosLoc, &camera.position,
                 SHADER_UNIFORM_VEC3);

  // Physics: gravitational force toward origin (sun at 0,0,0)
  Vector3 rVec = Vector3Negate(pos);
  float distance = Vector3Length(rVec);

  if (distance > 5.0f) {
    Vector3 forceDir = Vector3Normalize(rVec);
    float accelMag = GM / (distance * distance);
    Vector3 acceleration = Vector3Scale(forceDir, accelMag);

    // Semi-implicit Euler integration
    vel = Vector3Add(vel, Vector3Scale(acceleration, dt));
    pos = Vector3Add(pos, Vector3Scale(vel, dt));
  }
}

void Engine3D::draw() {
  BeginDrawing();
  ClearBackground((Color){5, 5, 15, 255});

  BeginMode3D(camera);

  // Draw sun (emissive, no lighting needed - just bright color)
  DrawModel(sunModel, {0, 0, 0}, 1.0f, YELLOW);

  // Draw planet with lighting (sun-facing side bright, opposite dark)
  float planetColor[4] = {0.3f, 0.6f, 0.9f, 1.0f}; // Blue color
  SetShaderValue(lightingShader, objectColorLoc, planetColor,
                 SHADER_UNIFORM_VEC4);
  DrawModel(planetModel, pos, 1.0f, WHITE);

  // Draw grid
  DrawGrid(20, 50.0f);

  EndMode3D();

  DrawFPS(10, 10);
  DrawText("3D Orbital Simulation", 10, 30, 20, WHITE);
  DrawText("WASD: Move | EQ: Up/Down | Mouse: Look | Scroll: Zoom | R: Reset",
           10, H - 30, 16, GRAY);

  EndDrawing();
}

void Engine3D::run() {
  InitWindow(W, H, "Orbital 3D");
  SetTargetFPS(60);

  setup();

  while (!WindowShouldClose()) {
    update();
    draw();
  }

  // Cleanup
  UnloadModel(planetModel);
  UnloadModel(sunModel);
  UnloadShader(lightingShader);

  CloseWindow();
}
