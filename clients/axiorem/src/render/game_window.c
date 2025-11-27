#include "game_window.h"
#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define TILE_SIZE_PIXELS 128.0f // Each tile is 16x16 pixels at zoom 1.0

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

// Add this function to render the mini-map
void RenderMiniMap(SimulationState *sim, Camera2D_RTS *camera, int minimapX,
                   int minimapY, int minimapSize) {
  // Mini-map background
  DrawRectangle(minimapX, minimapY, minimapSize, minimapSize, DARKBLUE);
  DrawRectangleLines(minimapX, minimapY, minimapSize, minimapSize, GOLD);

  // Calculate scaling factors from world to mini-map coordinates
  float scaleX = (float)minimapSize / sim->map.width;
  float scaleY = (float)minimapSize / sim->map.height;

  // Draw map tiles on mini-map
  for (int y = 0; y < sim->map.height; y++) {
    for (int x = 0; x < sim->map.width; x++) {
      TileType tile = sim->map.tiles[y * sim->map.width + x];
      Color color = GetTileColor(tile);

      // Scale down the color intensity for mini-map
      color = Fade(color, 0.7f);

      int pixelX = minimapX + (int)(x * scaleX);
      int pixelY = minimapY + (int)(y * scaleY);
      int pixelWidth = (int)ceil(scaleX);
      int pixelHeight = (int)ceil(scaleY);

      DrawRectangle(pixelX, pixelY, pixelWidth, pixelHeight, color);
    }
  }

  // Draw objects on mini-map (as small dots)
  for (int i = 0; i < sim->objectCount; i++) {
    Object obj = sim->objects[i];
    int pixelX = minimapX + (int)(obj.x * scaleX);
    int pixelY = minimapY + (int)(obj.y * scaleY);

    DrawCircle(pixelX, pixelY, 2, PURPLE);
  }

  // Draw units on mini-map (colored by owner)
  for (int i = 0; i < sim->unitCount; i++) {
    Unit unit = sim->units[i];
    int pixelX = minimapX + (int)(unit.x * scaleX);
    int pixelY = minimapY + (int)(unit.y * scaleY);

    Color unitColor = (unit.owner == 1) ? RED : YELLOW;
    DrawCircle(pixelX, pixelY, 2, unitColor);
  }

  // Draw camera viewport rectangle on mini-map
  Rectangle viewport = camera->viewport;
  int viewportX = minimapX + (int)(viewport.x * scaleX);
  int viewportY = minimapY + (int)(viewport.y * scaleY);
  int viewportWidth = (int)(viewport.width * scaleX);
  int viewportHeight = (int)(viewport.height * scaleY);

  DrawRectangleLines(viewportX, viewportY, viewportWidth, viewportHeight,
                     YELLOW);

  // Mini-map interaction: Click to move camera
  if (CheckCollisionPointRec(
          GetMousePosition(),
          (Rectangle){minimapX, minimapY, minimapSize, minimapSize})) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mousePos = GetMousePosition();

      // Convert mini-map click to world coordinates
      float worldX = (mousePos.x - minimapX) / scaleX;
      float worldY = (mousePos.y - minimapY) / scaleY;

      // Center camera on clicked position
      camera->target.x = worldX * TILE_SIZE_PIXELS;
      camera->target.y = worldY * TILE_SIZE_PIXELS;
    }

    // Draw hover effect
    DrawRectangleLines(minimapX, minimapY, minimapSize, minimapSize, WHITE);
  }
}

// Update the RenderUI function to use the working mini-map
void RenderUI(SimulationState *sim, int screenWidth, int screenHeight,
              Camera2D_RTS *camera) {
  int panelHeight = 150; // Height of the bottom control panel

  // Draw main bottom panel (like StarCraft's command card area)
  DrawRectangle(0, screenHeight - panelHeight, screenWidth, panelHeight,
                Fade(BLACK, 0.9f));

  // Draw top border for the panel
  DrawRectangle(0, screenHeight - panelHeight, screenWidth, 2, GOLD);

  // Draw mini-map area (top-right of panel)
  int minimapSize = 120;
  int minimapX = screenWidth - minimapSize - 10;
  int minimapY = screenHeight - panelHeight + 10;

  // Draw the actual mini-map
  RenderMiniMap(sim, camera, minimapX, minimapY, minimapSize);

  // Draw resource/status panel (left side)
  int statusWidth = 200;
  DrawRectangle(10, screenHeight - panelHeight + 10, statusWidth,
                panelHeight - 20, Fade(DARKGRAY, 0.7f));

  // Draw command buttons area (center)
  int commandX = statusWidth + 30;
  int commandWidth = screenWidth - statusWidth - minimapSize - 50;
  DrawRectangle(commandX, screenHeight - panelHeight + 10, commandWidth,
                panelHeight - 20, Fade(DARKBROWN, 0.6f));

  // Draw title and resources
  DrawText("AXIOM BATTLEFIELD", 20, screenHeight - panelHeight + 20, 18, GOLD);

  char infoText[256];

  // Resources/status info
  sprintf(infoText, "Units: %d", sim->unitCount);
  DrawText(infoText, 20, screenHeight - panelHeight + 50, 16, LIME);

  sprintf(infoText, "Objects: %d", sim->objectCount);
  DrawText(infoText, 20, screenHeight - panelHeight + 75, 16, SKYBLUE);

  sprintf(infoText, "Map: %dx%d", sim->map.width, sim->map.height);
  DrawText(infoText, 20, screenHeight - panelHeight + 100, 14, LIGHTGRAY);

  // Camera status
  sprintf(infoText, "Zoom: %.1fx", camera->zoom);
  DrawText(infoText, 120, screenHeight - panelHeight + 100, 14, YELLOW);

  // Command buttons (simplified version)
  int buttonWidth = 80;
  int buttonHeight = 30;
  int buttonSpacing = 10;

  // Row 1: Basic commands
  DrawRectangle(commandX + 10, screenHeight - panelHeight + 20, buttonWidth,
                buttonHeight, DARKGREEN);
  DrawText("MOVE", commandX + 25, screenHeight - panelHeight + 28, 12, WHITE);

  DrawRectangle(commandX + 10 + buttonWidth + buttonSpacing,
                screenHeight - panelHeight + 20, buttonWidth, buttonHeight,
                DARKBLUE);
  DrawText("ATTACK", commandX + 20 + buttonWidth + buttonSpacing,
           screenHeight - panelHeight + 28, 12, WHITE);

  DrawRectangle(commandX + 10 + 2 * (buttonWidth + buttonSpacing),
                screenHeight - panelHeight + 20, buttonWidth, buttonHeight,
                DARKPURPLE);
  DrawText("PATROL", commandX + 25 + 2 * (buttonWidth + buttonSpacing),
           screenHeight - panelHeight + 28, 12, WHITE);

  // Row 2: Simulation controls
  DrawRectangle(commandX + 10, screenHeight - panelHeight + 60, buttonWidth,
                buttonHeight, MAROON);
  DrawText(sim->paused ? "RESUME" : "PAUSE", commandX + 20,
           screenHeight - panelHeight + 68, 12, WHITE);

  DrawRectangle(commandX + 10 + buttonWidth + buttonSpacing,
                screenHeight - panelHeight + 60, buttonWidth, buttonHeight,
                ORANGE);
  DrawText("NEXT TICK", commandX + 15 + buttonWidth + buttonSpacing,
           screenHeight - panelHeight + 68, 12, WHITE);

  DrawRectangle(commandX + 10 + 2 * (buttonWidth + buttonSpacing),
                screenHeight - panelHeight + 60, buttonWidth, buttonHeight,
                DARKGRAY);
  DrawText("RESTART", commandX + 22 + 2 * (buttonWidth + buttonSpacing),
           screenHeight - panelHeight + 68, 12, WHITE);

  // Selected unit info (if any)
  DrawRectangle(commandX + 10, screenHeight - panelHeight + 100,
                commandWidth - 20, 25, Fade(BLUE, 0.5f));
  DrawText("No unit selected", commandX + 20, screenHeight - panelHeight + 106,
           14, WHITE);

  // Mini-map text
  DrawText("MINI-MAP", minimapX + 30, minimapY + minimapSize + 5, 12, GOLD);
  DrawText("Click to move", minimapX + 20, minimapY + minimapSize + 20, 10,
           LIGHTGRAY);

  // Draw top info bar (optional - like StarCraft's top resource bar)
  int topBarHeight = 25;
  DrawRectangle(0, 0, screenWidth, topBarHeight, Fade(BLACK, 0.8f));

  sprintf(infoText, "Simulation Time: %.1fs | FPS: %d", GetTime(), GetFPS());
  DrawText(infoText, screenWidth / 2 - MeasureText(infoText, 16) / 2, 5, 16,
           GREEN);

  // Draw control hints on top bar
  DrawText("WASD: Move  |  Mouse Wheel: Zoom  |  R: Reset  |  P: Pause", 10, 5,
           14, LIGHTGRAY);
}
int RunGameWindow(SimulationState *sim) {
  if (sim == NULL) {
    return 1;
  }

  const int screenWidth = 800;
  const int screenHeight = 600;

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
    RenderUI(sim, screenWidth, screenHeight, &camera);

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
