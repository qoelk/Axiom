#include "renderer.h"
#include "../utils/math_utils.h"
#include "sim_loader.h"
#include <math.h>
#include <stddef.h>

// Internal constants
static const float GRID_VISIBILITY_THRESHOLD = 0.5f;
static const float GRID_ALPHA = 0.3f;

// Global tile atlas
TileAtlas g_tile_atlas = {0};

// Tile type to atlas coordinates mapping (multiple variations per tile type)
typedef struct {
  TileType tile_type;
  int variation_count;
  int atlas_coords[8][2]; // [x,y] pairs for up to 8 variations per tile type
} TileMapping;

static TileMapping TILE_MAPPINGS[] = {
    {
        .tile_type = TILE_WATER,
        .variation_count = 1,
        .atlas_coords = {{1, 1}} // Water tile
    },
    {
        .tile_type = TILE_LAND,
        .variation_count = 2,
        .atlas_coords = {{3, 3}, {3, 4}} // Grass variations
    },
    {
        .tile_type = TILE_DIRT,
        .variation_count = 2,
        .atlas_coords = {{4, 3}, {4, 4}} // Dirt variations - NEEDS UPDATE
    },
    {
        .tile_type = TILE_ROCK,
        .variation_count = 2,
        .atlas_coords = {{5, 3}, {5, 4}} // Rock variations - NEEDS UPDATE
    }};
void renderer_init_tile_atlas(const char *texture_path, int tile_width,
                              int tile_height, int gap) {
  g_tile_atlas.texture = LoadTexture(texture_path);
  g_tile_atlas.tile_width = tile_width;
  g_tile_atlas.tile_height = tile_height;
  g_tile_atlas.gap = gap;

  // Calculate columns and rows based on texture dimensions
  g_tile_atlas.columns =
      (g_tile_atlas.texture.width + gap) / (tile_width + gap);
  g_tile_atlas.rows = (g_tile_atlas.texture.height + gap) / (tile_height + gap);
}

void renderer_cleanup_tile_atlas(void) {
  if (g_tile_atlas.texture.id > 0) {
    UnloadTexture(g_tile_atlas.texture);
    g_tile_atlas.texture.id = 0;
  }
}

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

Rectangle renderer_get_tile_source_rect(TileType tile_type, int x, int y,
                                        const TileMap *map) {
  // Find the mapping for this tile type
  for (size_t i = 0; i < sizeof(TILE_MAPPINGS) / sizeof(TILE_MAPPINGS[0]);
       i++) {
    if (TILE_MAPPINGS[i].tile_type == tile_type) {
      // Safety check
      if (TILE_MAPPINGS[i].variation_count <= 0) {
        TraceLog(LOG_ERROR, "No variations for tile type %d", tile_type);
        // Fallback to first grass tile
        return (Rectangle){0, 3 * (g_tile_atlas.tile_height + g_tile_atlas.gap),
                           g_tile_atlas.tile_width, g_tile_atlas.tile_height};
      }

      // Use position-based variation for more natural distribution
      int variation_index = (x + y * 7) % TILE_MAPPINGS[i].variation_count;
      int atlas_x = TILE_MAPPINGS[i].atlas_coords[variation_index][0];
      int atlas_y = TILE_MAPPINGS[i].atlas_coords[variation_index][1];

      // Calculate source rectangle considering gaps
      int source_x = atlas_x * (g_tile_atlas.tile_width + g_tile_atlas.gap);
      int source_y = atlas_y * (g_tile_atlas.tile_height + g_tile_atlas.gap);

      return (Rectangle){source_x, source_y, g_tile_atlas.tile_width,
                         g_tile_atlas.tile_height};
    }
  }

  // Fallback: return first grass tile
  return (Rectangle){0, 3 * (g_tile_atlas.tile_height + g_tile_atlas.gap),
                     g_tile_atlas.tile_width, g_tile_atlas.tile_height};
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

void renderer_draw_map_textured(const TileMap *map,
                                const Camera2D_RTS *camera) {
  // Check if tile atlas is loaded
  if (g_tile_atlas.texture.id == 0) {
    // Fall back to colored rendering
    renderer_draw_map(map, camera);
    return;
  }

  int start_x, start_y, end_x, end_y;
  renderer_calculate_visible_tile_range(camera, map, &start_x, &start_y, &end_x,
                                        &end_y);

  for (int y = start_y; y < end_y; y++) {
    for (int x = start_x; x < end_x; x++) {
      TileType tile = map->tiles[y * map->width + x];
      Rectangle source_rect = renderer_get_tile_source_rect(tile, x, y, map);

      Vector2 screen_pos =
          camera_world_to_screen(camera, (Vector2){x + 0.5f, y + 0.5f});
      float tile_size = TILE_SIZE_PIXELS * camera->zoom;
      bool draw_grid = (camera->zoom > GRID_VISIBILITY_THRESHOLD);

      if (renderer_is_position_visible(screen_pos, tile_size / 2)) {
        // Draw the texture
        Rectangle dest_rect = {screen_pos.x - tile_size / 2,
                               screen_pos.y - tile_size / 2, tile_size,
                               tile_size};
        DrawTexturePro(g_tile_atlas.texture, source_rect, dest_rect,
                       (Vector2){0, 0}, 0.0f, WHITE);

        // Draw grid if needed
        if (draw_grid) {
          DrawRectangleLines(dest_rect.x, dest_rect.y, dest_rect.width,
                             dest_rect.height, Fade(BLACK, GRID_ALPHA));
        }
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
