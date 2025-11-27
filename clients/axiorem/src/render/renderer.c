#include "renderer.h"
#include "../utils/math_utils.h"
#include <math.h>

// Internal constants
static const float GRID_VISIBILITY_THRESHOLD = 0.5f;
static const float GRID_ALPHA = 0.3f;

Color renderer_get_tile_color(TileType tile) {
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

void renderer_draw_tile(Vector2 screen_pos, float size, Color color,
                        bool draw_grid) {
  DrawRectangle(screen_pos.x - size / 2, screen_pos.y - size / 2, size, size,
                color);

  if (draw_grid) {
    DrawRectangleLines(screen_pos.x - size / 2, screen_pos.y - size / 2, size,
                       size, Fade(BLACK, GRID_ALPHA));
  }
}

bool renderer_is_position_visible(Vector2 screen_pos, float radius) {
  int screen_width = GetScreenWidth();
  int screen_height = GetScreenHeight();

  return (screen_pos.x + radius > 0 && screen_pos.x - radius < screen_width &&
          screen_pos.y + radius > 0 && screen_pos.y - radius < screen_height);
}

void renderer_calculate_visible_tile_range(const Camera2D_RTS *camera,
                                           const TileMap *map, int *start_x,
                                           int *start_y, int *end_x,
                                           int *end_y) {
  *start_x = (int)fmax(0, floor(camera->viewport.x));
  *start_y = (int)fmax(0, floor(camera->viewport.y));
  *end_x =
      (int)fmin(map->width, ceil(camera->viewport.x + camera->viewport.width));
  *end_y = (int)fmin(map->height,
                     ceil(camera->viewport.y + camera->viewport.height));
}

void renderer_draw_map(const TileMap *map, const Camera2D_RTS *camera) {
  int start_x, start_y, end_x, end_y;
  renderer_calculate_visible_tile_range(camera, map, &start_x, &start_y, &end_x,
                                        &end_y);

  for (int y = start_y; y < end_y; y++) {
    for (int x = start_x; x < end_x; x++) {
      TileType tile = map->tiles[y * map->width + x];
      Color color = renderer_get_tile_color(tile);

      Vector2 screen_pos =
          camera_world_to_screen(camera, (Vector2){x + 0.5f, y + 0.5f});
      float tile_size = TILE_SIZE_PIXELS * camera->zoom;
      bool draw_grid = (camera->zoom > GRID_VISIBILITY_THRESHOLD);

      if (renderer_is_position_visible(screen_pos, tile_size / 2)) {
        renderer_draw_tile(screen_pos, tile_size, color, draw_grid);
      }
    }
  }
}

void renderer_draw_objects(const Object *objects, int count,
                           const Camera2D_RTS *camera) {
  for (int i = 0; i < count; i++) {
    Object obj = objects[i];
    Vector2 screen_pos =
        camera_world_to_screen(camera, (Vector2){obj.x, obj.y});
    float radius = obj.size * TILE_SIZE_PIXELS * camera->zoom / 2.0f;

    if (renderer_is_position_visible(screen_pos, radius)) {
      DrawCircle(screen_pos.x, screen_pos.y, radius, PURPLE);
      DrawCircleLines(screen_pos.x, screen_pos.y, radius, DARKPURPLE);
    }
  }
}

void renderer_draw_units(const Unit *units, int count,
                         const Camera2D_RTS *camera) {
  for (int i = 0; i < count; i++) {
    Unit unit = units[i];
    Vector2 screen_pos =
        camera_world_to_screen(camera, (Vector2){unit.x, unit.y});
    float radius = unit.size * TILE_SIZE_PIXELS * camera->zoom / 2.0f;

    if (!renderer_is_position_visible(screen_pos, radius))
      continue;

    Color unit_color = (unit.owner == 1) ? RED : YELLOW;
    DrawCircle(screen_pos.x, screen_pos.y, radius, unit_color);
    DrawCircleLines(screen_pos.x, screen_pos.y, radius, BLACK);

    // Draw facing direction indicator
    float end_x = screen_pos.x + cos(unit.facing * DEG2RAD) * radius * 1.5f;
    float end_y = screen_pos.y + sin(unit.facing * DEG2RAD) * radius * 1.5f;
    DrawLine(screen_pos.x, screen_pos.y, end_x, end_y, BLACK);
  }
}
