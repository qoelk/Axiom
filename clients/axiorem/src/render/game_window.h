#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

#include "../client/sim_loader.h"
#include "raylib.h"

typedef struct {
  Vector2 position;   // Camera position in world coordinates
  float zoom;         // Zoom level (1.0 = normal)
  Vector2 target;     // Target position for smooth movement
  float moveSpeed;    // Camera movement speed
  float zoomSpeed;    // Zoom speed
  Rectangle viewport; // Current viewport in world coordinates
} Camera2D_RTS;

void InitRTSCamera(Camera2D_RTS *camera, int screenWidth, int screenHeight,
                   TileMap *map);
void UpdateRTSCamera(Camera2D_RTS *camera, TileMap *map);
void RenderMap(TileMap *map, Camera2D_RTS *camera);
void RenderObjects(Object *objects, int count, Camera2D_RTS *camera);
void RenderUnits(Unit *units, int count, Camera2D_RTS *camera);
void RenderUI(SimulationState *sim, int screenWidth, int screenHeight,
              Camera2D_RTS *camera);
int RunGameWindow(SimulationState *sim);

#endif
