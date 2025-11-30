#ifndef CAMERA_H
#define CAMERA_H

#include "../client/sim_loader.h"
#include "raylib.h"

/**
 * @brief RTS-style camera configuration
 */
typedef struct {
  int screen_width;
  int screen_height;
  float camera_move_speed;
  float camera_zoom_speed;
} CameraConfig;

/**
 * @brief RTS-style 2D camera for battlefield view
 */
typedef struct {
  Vector2 position;
  Vector2 target;
  float zoom;
  float move_speed;
  float zoom_speed;
  Rectangle viewport;
} Camera2D_RTS;

// Camera lifecycle management
void camera_init(Camera2D_RTS *camera, const CameraConfig *config,
                 const TileMap *map);
void camera_update(Camera2D_RTS *camera, const TileMap *map);
void camera_constrain_to_map(Camera2D_RTS *camera, const TileMap *map);

// Coordinate transformation
Vector2 camera_world_to_screen(const Camera2D_RTS *camera, Vector2 world_pos);
Vector2 camera_screen_to_world(const Camera2D_RTS *camera, Vector2 screen_pos);
void camera_center_on_world_position(Camera2D_RTS *camera, Vector2 world_pos);

// Input handling
void camera_handle_zoom_input(Camera2D_RTS *camera);
void camera_handle_keyboard_input(Camera2D_RTS *camera);
void camera_handle_edge_scrolling(Camera2D_RTS *camera);

#endif
