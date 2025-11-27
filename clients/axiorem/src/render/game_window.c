#include "game_window.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define TILE_SIZE_PIXELS 16.0f // Each tile is 16x16 pixels at zoom 1.0

// Custom math utilities
float CustomClamp(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

float CustomLerp(float start, float end, float amount) {
  return start + (end - start) * amount;
}

Vector2 Vector2NormalizeCustom(Vector2 v) {
  Vector2 result = {0};
  float length = sqrtf(v.x * v.x + v.y * v.y);

  if (length != 0.0f) {
    float invLength = 1.0f / length;
    result.x = v.x * invLength;
    result.y = v.y * invLength;
  }

  return result;
}

// Convert world coordinates to screen coordinates
Vector2 WorldToScreen(Vector2 worldPos, Camera2D_RTS *camera, int screenWidth,
                      int screenHeight) {
  Vector2 screenPos;
  screenPos.x =
      (worldPos.x * TILE_SIZE_PIXELS - camera->position.x) * camera->zoom +
      screenWidth / 2.0f;
  screenPos.y =
      (worldPos.y * TILE_SIZE_PIXELS - camera->position.y) * camera->zoom +
      screenHeight / 2.0f;
  return screenPos;
}

// Convert screen coordinates to world coordinates
Vector2 ScreenToWorld(Vector2 screenPos, Camera2D_RTS *camera, int screenWidth,
                      int screenHeight) {
  Vector2 worldPos;
  worldPos.x =
      ((screenPos.x - screenWidth / 2.0f) / camera->zoom + camera->position.x) /
      TILE_SIZE_PIXELS;
  worldPos.y = ((screenPos.y - screenHeight / 2.0f) / camera->zoom +
                camera->position.y) /
               TILE_SIZE_PIXELS;
  return worldPos;
}

void InitRTSCamera(Camera2D_RTS *camera, int screenWidth, int screenHeight,
                   TileMap *map) {
  camera->position = (Vector2){0, 0};
  camera->zoom = 1.0f;
  camera->target = camera->position;
  camera->moveSpeed = 100.0f; // Increased for larger tile size
  camera->zoomSpeed = 0.1f;

  // Calculate initial viewport in world units (tiles)
  float halfWidth = (screenWidth / 2.0f) / (camera->zoom * TILE_SIZE_PIXELS);
  float halfHeight = (screenHeight / 2.0f) / (camera->zoom * TILE_SIZE_PIXELS);
  camera->viewport =
      (Rectangle){camera->position.x / TILE_SIZE_PIXELS - halfWidth,
                  camera->position.y / TILE_SIZE_PIXELS - halfHeight,
                  halfWidth * 2, halfHeight * 2};
}
void ConstrainCameraToMap(Camera2D_RTS *camera, TileMap *map, int screenWidth,
                          int screenHeight) {
  // Calculate the world boundaries in pixels
  float mapWidthPixels = map->width * TILE_SIZE_PIXELS;
  float mapHeightPixels = map->height * TILE_SIZE_PIXELS;

  // Calculate the camera viewport size in pixels
  float viewportWidth = (screenWidth / camera->zoom);
  float viewportHeight = (screenHeight / camera->zoom);

  // Calculate minimum and maximum camera positions
  float minX = viewportWidth / 2.0f;
  float maxX = mapWidthPixels - viewportWidth / 2.0f;
  float minY = viewportHeight / 2.0f;
  float maxY = mapHeightPixels - viewportHeight / 2.0f;

  // If the map is smaller than the viewport, center the camera
  if (viewportWidth > mapWidthPixels) {
    minX = maxX = mapWidthPixels / 2.0f;
  }
  if (viewportHeight > mapHeightPixels) {
    minY = maxY = mapHeightPixels / 2.0f;
  }

  // Constrain camera position
  camera->position.x = CustomClamp(camera->position.x, minX, maxX);
  camera->position.y = CustomClamp(camera->position.y, minY, maxY);
  camera->target.x = CustomClamp(camera->target.x, minX, maxX);
  camera->target.y = CustomClamp(camera->target.y, minY, maxY);
}

void UpdateRTSCamera(Camera2D_RTS *camera, TileMap *map) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  // Mouse scroll for zoom
  float wheel = GetMouseWheelMove();
  if (wheel != 0) {
    Vector2 mouseWorldPosBefore =
        ScreenToWorld(GetMousePosition(), camera, screenWidth, screenHeight);

    float oldZoom = camera->zoom;
    camera->zoom += wheel * camera->zoomSpeed;
    camera->zoom = CustomClamp(camera->zoom, 0.2f, 3.0f); // Min/max zoom

    // Only adjust position if zoom actually changed
    if (camera->zoom != oldZoom) {
      Vector2 mouseWorldPosAfter =
          ScreenToWorld(GetMousePosition(), camera, screenWidth, screenHeight);
      camera->position.x += (mouseWorldPosAfter.x - mouseWorldPosBefore.x) *
                            TILE_SIZE_PIXELS * camera->zoom;
      camera->position.y += (mouseWorldPosAfter.y - mouseWorldPosBefore.y) *
                            TILE_SIZE_PIXELS * camera->zoom;
    }
  }

  // Keyboard camera movement (WASD or Arrow keys)
  Vector2 input = {0, 0};
  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
    input.y -= 1;
  if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
    input.y += 1;
  if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    input.x -= 1;
  if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    input.x += 1;

  // Normalize diagonal movement
  if (input.x != 0 && input.y != 0) {
    input = Vector2NormalizeCustom(input);
  }

  // Apply movement speed (faster when zoomed out)
  float effectiveSpeed = camera->moveSpeed / camera->zoom;
  camera->target.x += input.x * effectiveSpeed;
  camera->target.y += input.y * effectiveSpeed;

  // Edge scrolling with mouse
  Vector2 mousePos = GetMousePosition();
  float edgeSize = 20.0f; // Pixels from edge to trigger scrolling

  if (mousePos.x < edgeSize)
    camera->target.x -= effectiveSpeed;
  if (mousePos.x > screenWidth - edgeSize)
    camera->target.x += effectiveSpeed;
  if (mousePos.y < edgeSize)
    camera->target.y -= effectiveSpeed;
  if (mousePos.y > screenHeight - edgeSize)
    camera->target.y += effectiveSpeed;

  // Smooth camera movement towards target
  camera->position.x = CustomLerp(camera->position.x, camera->target.x, 0.1f);
  camera->position.y = CustomLerp(camera->position.y, camera->target.y, 0.1f);

  // Constrain camera to map boundaries
  ConstrainCameraToMap(camera, map, screenWidth, screenHeight);

  // Update viewport in world units (tiles)
  float halfWidth = (screenWidth / 2.0f) / (camera->zoom * TILE_SIZE_PIXELS);
  float halfHeight = (screenHeight / 2.0f) / (camera->zoom * TILE_SIZE_PIXELS);
  camera->viewport =
      (Rectangle){camera->position.x / TILE_SIZE_PIXELS - halfWidth,
                  camera->position.y / TILE_SIZE_PIXELS - halfHeight,
                  halfWidth * 2, halfHeight * 2};
}
Color GetTileColor(TileType tile) {
  switch (tile) {
  case TILE_WATER:
    return BLUE;
  case TILE_LAND:
    return GREEN;
  case TILE_DIRT:
    return BROWN;
  case TILE_ROCK:
    return GRAY;
  default:
    return BLACK;
  }
}

void RenderMap(TileMap *map, Camera2D_RTS *camera) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  // Calculate visible tile range based on camera viewport
  int startX = (int)fmax(0, floor(camera->viewport.x));
  int startY = (int)fmax(0, floor(camera->viewport.y));
  int endX =
      (int)fmin(map->width, ceil(camera->viewport.x + camera->viewport.width));
  int endY = (int)fmin(map->height,
                       ceil(camera->viewport.y + camera->viewport.height));

  for (int y = startY; y < endY; y++) {
    for (int x = startX; x < endX; x++) {
      TileType tile = map->tiles[y * map->width + x];
      Color color = GetTileColor(tile);

      Vector2 screenPos = WorldToScreen((Vector2){x + 0.5f, y + 0.5f}, camera,
                                        screenWidth, screenHeight);
      float tileSize = TILE_SIZE_PIXELS * camera->zoom;

      if (screenPos.x + tileSize / 2 > 0 &&
          screenPos.x - tileSize / 2 < screenWidth &&
          screenPos.y + tileSize / 2 > 0 &&
          screenPos.y - tileSize / 2 < screenHeight) {

        DrawRectangle(screenPos.x - tileSize / 2, screenPos.y - tileSize / 2,
                      tileSize, tileSize, color);

        // Only draw grid lines when zoomed in enough
        if (camera->zoom > 0.5f) {
          DrawRectangleLines(screenPos.x - tileSize / 2,
                             screenPos.y - tileSize / 2, tileSize, tileSize,
                             Fade(BLACK, 0.3f));
        }
      }
    }
  }
}

void RenderObjects(Object *objects, int count, Camera2D_RTS *camera) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  for (int i = 0; i < count; i++) {
    Object obj = objects[i];
    Vector2 screenPos = WorldToScreen((Vector2){obj.x, obj.y}, camera,
                                      screenWidth, screenHeight);
    float radius = obj.size * TILE_SIZE_PIXELS * camera->zoom / 2.0f;

    // Only render if on screen
    if (screenPos.x + radius > 0 && screenPos.x - radius < screenWidth &&
        screenPos.y + radius > 0 && screenPos.y - radius < screenHeight) {

      DrawCircle(screenPos.x, screenPos.y, radius, PURPLE);
      DrawCircleLines(screenPos.x, screenPos.y, radius, DARKPURPLE);
    }
  }
}

void RenderUnits(Unit *units, int count, Camera2D_RTS *camera) {
  int screenWidth = GetScreenWidth();
  int screenHeight = GetScreenHeight();

  for (int i = 0; i < count; i++) {
    Unit unit = units[i];
    Vector2 screenPos = WorldToScreen((Vector2){unit.x, unit.y}, camera,
                                      screenWidth, screenHeight);
    float radius = unit.size * TILE_SIZE_PIXELS * camera->zoom / 2.0f;

    // Only render if on screen
    if (screenPos.x + radius > 0 && screenPos.x - radius < screenWidth &&
        screenPos.y + radius > 0 && screenPos.y - radius < screenHeight) {

      // Different colors for different owners
      Color unitColor = (unit.owner == 1) ? RED : YELLOW;

      DrawCircle(screenPos.x, screenPos.y, radius, unitColor);
      DrawCircleLines(screenPos.x, screenPos.y, radius, BLACK);

      // Draw facing direction
      float endX = screenPos.x + cos(unit.facing * DEG2RAD) * radius * 1.5f;
      float endY = screenPos.y + sin(unit.facing * DEG2RAD) * radius * 1.5f;
      DrawLine(screenPos.x, screenPos.y, endX, endY, BLACK);
    }
  }
}

void RenderUI(SimulationState *sim, int screenWidth, Camera2D_RTS *camera) {
  // Draw info panel
  DrawRectangle(screenWidth - 250, 0, 250, 1024, Fade(LIGHTGRAY, 0.8f));
  DrawText("Axiom - AI Battlefield", screenWidth - 240, 20, 20, MAROON);

  char infoText[256];
  sprintf(infoText, "Map: %dx%d tiles", sim->map.width, sim->map.height);
  DrawText(infoText, screenWidth - 240, 60, 16, DARKGRAY);

  sprintf(infoText, "Tile Size: %dpx", TILE_SIZE_PIXELS);
  DrawText(infoText, screenWidth - 240, 90, 16, DARKGRAY);

  sprintf(infoText, "Objects: %d", sim->objectCount);
  DrawText(infoText, screenWidth - 240, 120, 16, DARKGRAY);

  sprintf(infoText, "Units: %d", sim->unitCount);
  DrawText(infoText, screenWidth - 240, 150, 16, DARKGRAY);

  sprintf(infoText, "Camera: (%.1f, %.1f)", camera->position.x,
          camera->position.y);
  DrawText(infoText, screenWidth - 240, 180, 16, DARKGRAY);

  sprintf(infoText, "Zoom: %.1fx", camera->zoom);
  DrawText(infoText, screenWidth - 240, 210, 16, DARKGRAY);

  DrawText("Controls:", screenWidth - 240, 250, 16, DARKGRAY);
  DrawText("WASD/Arrows - Move camera", screenWidth - 240, 280, 14, DARKGRAY);
  DrawText("Mouse Wheel - Zoom", screenWidth - 240, 300, 14, DARKGRAY);
  DrawText("Edge Scroll - Move camera", screenWidth - 240, 320, 14, DARKGRAY);
  DrawText("SPACE - Next tick", screenWidth - 240, 340, 14, DARKGRAY);
  DrawText("R - Reset simulation", screenWidth - 240, 360, 14, DARKGRAY);
  DrawText("P - Pause", screenWidth - 240, 380, 14, DARKGRAY);
}

int RunGameWindow(SimulationState *sim) {
  if (sim == NULL) {
    return 1;
  }

  const int screenWidth = 1280;
  const int screenHeight = 1024;

  InitWindow(screenWidth, screenHeight, "Axiom - AI Battlefield");
  SetTargetFPS(60);

  // Initialize RTS camera with map reference
  Camera2D_RTS camera;
  InitRTSCamera(&camera, screenWidth, screenHeight, &sim->map);

  int currentTick = 0;
  bool paused = false;

  while (!WindowShouldClose()) {
    // Update camera with map reference
    UpdateRTSCamera(&camera, &sim->map);

    // Handle input
    if (IsKeyPressed(KEY_SPACE)) {
      currentTick++;
    }

    if (IsKeyPressed(KEY_R)) {
      FreeState(sim);
      sim = LoadState();
      currentTick = 0;
      InitRTSCamera(&camera, screenWidth, screenHeight,
                    &sim->map); // Reset camera
    }

    if (IsKeyPressed(KEY_P)) {
      paused = !paused;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Render game elements with camera
    RenderMap(&sim->map, &camera);
    RenderObjects(sim->objects, sim->objectCount, &camera);
    RenderUnits(sim->units, sim->unitCount, &camera);

    // Render UI
    RenderUI(sim, screenWidth, &camera);

    // Render tick info
    char tickText[64];
    sprintf(tickText, "Tick: %d", currentTick);
    DrawText(tickText, 20, screenHeight - 40, 20, DARKGRAY);

    if (paused) {
      DrawText("PAUSED", screenWidth / 2 - 40, 20, 30, RED);
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
