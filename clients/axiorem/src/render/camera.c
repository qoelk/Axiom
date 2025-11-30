#include "camera.h"
#include "../utils/math_utils.h"
#include <math.h>

// Internal constants
static const float MIN_ZOOM_LEVEL = 0.2f;
static const float MAX_ZOOM_LEVEL = 3.0f;
static const float EDGE_SCROLL_THRESHOLD = 20.0f;
static const float CAMERA_SMOOTHING_FACTOR = 0.1f;

void camera_init(Camera2D_RTS *camera, const CameraConfig *config,
                 const TileMap *map) {
  camera->position = (Vector2){0, 0};
  camera->target = camera->position;
  camera->zoom = 1.0f;
  camera->move_speed = config->camera_move_speed;
  camera->zoom_speed = config->camera_zoom_speed;

  // Initial viewport calculation
  float half_width =
      (config->screen_width / 2.0f) / (camera->zoom * TILE_SIZE_PIXELS);
  float half_height =
      (config->screen_height / 2.0f) / (camera->zoom * TILE_SIZE_PIXELS);

  camera->viewport =
      (Rectangle){camera->position.x / TILE_SIZE_PIXELS - half_width,
                  camera->position.y / TILE_SIZE_PIXELS - half_height,
                  half_width * 2, half_height * 2};
}

void camera_handle_zoom_input(Camera2D_RTS *camera) {
  float wheel = GetMouseWheelMove();
  if (wheel == 0)
    return;

  Vector2 mouse_world_before =
      camera_screen_to_world(camera, GetMousePosition());
  float old_zoom = camera->zoom;

  camera->zoom += wheel * camera->zoom_speed;
  camera->zoom = math_utils_clamp(camera->zoom, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL);

  // Adjust position to zoom towards mouse position
  if (camera->zoom != old_zoom) {
    Vector2 mouse_world_after =
        camera_screen_to_world(camera, GetMousePosition());
    camera->position.x += (mouse_world_after.x - mouse_world_before.x) *
                          TILE_SIZE_PIXELS * camera->zoom;
    camera->position.y += (mouse_world_after.y - mouse_world_before.y) *
                          TILE_SIZE_PIXELS * camera->zoom;
  }
}

void camera_handle_keyboard_input(Camera2D_RTS *camera) {
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
    input = math_utils_vector2_normalize(input);
  }

  float effective_speed = camera->move_speed / camera->zoom;
  camera->target.x += input.x * effective_speed;
  camera->target.y += input.y * effective_speed;
}

void camera_handle_edge_scrolling(Camera2D_RTS *camera) {
  Vector2 mouse_pos = GetMousePosition();
  int screen_width = GetScreenWidth();
  int screen_height = GetScreenHeight();
  float effective_speed = camera->move_speed / camera->zoom;

  if (mouse_pos.x < EDGE_SCROLL_THRESHOLD)
    camera->target.x -= effective_speed;
  if (mouse_pos.x > screen_width - EDGE_SCROLL_THRESHOLD)
    camera->target.x += effective_speed;
  if (mouse_pos.y < EDGE_SCROLL_THRESHOLD)
    camera->target.y -= effective_speed;
  if (mouse_pos.y > screen_height - EDGE_SCROLL_THRESHOLD)
    camera->target.y += effective_speed;
}

void camera_update(Camera2D_RTS *camera, const TileMap *map) {
  camera_handle_zoom_input(camera);
  camera_handle_keyboard_input(camera);
  camera_handle_edge_scrolling(camera);

  // Smooth camera movement
  camera->position.x = math_utils_lerp(camera->position.x, camera->target.x,
                                       CAMERA_SMOOTHING_FACTOR);
  camera->position.y = math_utils_lerp(camera->position.y, camera->target.y,
                                       CAMERA_SMOOTHING_FACTOR);

  camera_constrain_to_map(camera, map);

  // Update viewport
  float half_width =
      (GetScreenWidth() / 2.0f) / (camera->zoom * TILE_SIZE_PIXELS);
  float half_height =
      (GetScreenHeight() / 2.0f) / (camera->zoom * TILE_SIZE_PIXELS);

  camera->viewport =
      (Rectangle){camera->position.x / TILE_SIZE_PIXELS - half_width,
                  camera->position.y / TILE_SIZE_PIXELS - half_height,
                  half_width * 2, half_height * 2};
}

Vector2 camera_world_to_screen(const Camera2D_RTS *camera, Vector2 world_pos) {
  int screen_width = GetScreenWidth();
  int screen_height = GetScreenHeight();

  return (Vector2){
      (world_pos.x * TILE_SIZE_PIXELS - camera->position.x) * camera->zoom +
          screen_width / 2.0f,
      (world_pos.y * TILE_SIZE_PIXELS - camera->position.y) * camera->zoom +
          screen_height / 2.0f};
}

Vector2 camera_screen_to_world(const Camera2D_RTS *camera, Vector2 screen_pos) {
  int screen_width = GetScreenWidth();
  int screen_height = GetScreenHeight();

  return (Vector2){((screen_pos.x - screen_width / 2.0f) / camera->zoom +
                    camera->position.x) /
                       TILE_SIZE_PIXELS,
                   ((screen_pos.y - screen_height / 2.0f) / camera->zoom +
                    camera->position.y) /
                       TILE_SIZE_PIXELS};
}

void camera_constrain_to_map(Camera2D_RTS *camera, const TileMap *map) {
  int screen_width = GetScreenWidth();
  int screen_height = GetScreenHeight();

  float map_width_pixels = map->width * TILE_SIZE_PIXELS;
  float map_height_pixels = map->height * TILE_SIZE_PIXELS;
  float viewport_width = screen_width / camera->zoom;
  float viewport_height = screen_height / camera->zoom;

  float min_x = viewport_width / 2.0f;
  float max_x = map_width_pixels - viewport_width / 2.0f;
  float min_y = viewport_height / 2.0f;
  float max_y = map_height_pixels - viewport_height / 2.0f;

  // Center camera if map is smaller than viewport
  if (viewport_width > map_width_pixels) {
    min_x = max_x = map_width_pixels / 2.0f;
  }
  if (viewport_height > map_height_pixels) {
    min_y = max_y = map_height_pixels / 2.0f;
  }

  camera->position.x = math_utils_clamp(camera->position.x, min_x, max_x);
  camera->position.y = math_utils_clamp(camera->position.y, min_y, max_y);
  camera->target.x = math_utils_clamp(camera->target.x, min_x, max_x);
  camera->target.y = math_utils_clamp(camera->target.y, min_y, max_y);
}

void camera_center_on_world_position(Camera2D_RTS *camera, Vector2 world_pos) {
  camera->target.x = world_pos.x * TILE_SIZE_PIXELS;
  camera->target.y = world_pos.y * TILE_SIZE_PIXELS;
}
