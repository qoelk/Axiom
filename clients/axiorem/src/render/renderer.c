#include "renderer.h"
#include "../map/map.h"
#include "../utils/math_utils.h"
#include "sim_loader.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>

TileAtlas g_tile_atlas = {0};
Texture2D g_unit_texture = {0};
Texture2D g_tree_texture = {0};

void renderer_init_tile_atlas(const char *texture_path, int tile_width,
                              int tile_height, int gap) {
  g_tile_atlas.texture = LoadTexture(texture_path);
  g_tile_atlas.tile_width = tile_width;
  g_tile_atlas.tile_height = tile_height;
  g_tile_atlas.gap = gap;

  // Calculate columns and rows based on texture dimensions
  g_tile_atlas.columns = g_tile_atlas.texture.width / (tile_width + gap);
  g_tile_atlas.rows = (g_tile_atlas.texture.height + gap) / (tile_height + gap);
}

void renderer_init_unit_texture(const char *texture_path) {
  if (g_unit_texture.id > 0) {
    UnloadTexture(g_unit_texture);
  }
  g_unit_texture = LoadTexture(texture_path);
}

void renderer_init_tree_texture(const char *texture_path) {
  if (g_tree_texture.id > 0) {
    UnloadTexture(g_tree_texture);
  }
  g_tree_texture = LoadTexture(texture_path);
}

void renderer_cleanup_tile_atlas(void) {
  if (g_tile_atlas.texture.id > 0) {
    UnloadTexture(g_tile_atlas.texture);
    g_tile_atlas.texture.id = 0;
  }
}

void renderer_cleanup_unit_texture(void) {
  if (g_unit_texture.id > 0) {
    UnloadTexture(g_unit_texture);
    g_unit_texture.id = 0;
  }
}

void renderer_cleanup_tree_texture(void) {
  if (g_tree_texture.id > 0) {
    UnloadTexture(g_tree_texture);
    g_tree_texture.id = 0;
  }
}

void renderer_draw_tile(Vector2 screen_pos, float size, Color color) {
  DrawRectangle(screen_pos.x - size / 2, screen_pos.y - size / 2, size, size,
                color);
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

Rectangle renderer_get_tile_source_rect_from_tile(const Tile *tile) {

  int atlas_x = tile->texture_index_x;
  int atlas_y = tile->texture_index_y;

  int source_x = atlas_x * (g_tile_atlas.tile_width + g_tile_atlas.gap);
  int source_y = atlas_y * (g_tile_atlas.tile_height + g_tile_atlas.gap);

  return (Rectangle){source_x, source_y, g_tile_atlas.tile_width,
                     g_tile_atlas.tile_height};
}

void renderer_draw_map_textured(const TileMap *map,
                                const Camera2D_RTS *camera) {
  if (g_tile_atlas.texture.id == 0)
    return;

  int start_x, start_y, end_x, end_y;
  renderer_calculate_visible_tile_range(camera, map, &start_x, &start_y, &end_x,
                                        &end_y);

  for (int y = start_y; y < end_y; y++) {
    for (int x = start_x; x < end_x; x++) {
      Tile *tile = &map->tiles[y * map->width + x];
      Rectangle source_rect = renderer_get_tile_source_rect_from_tile(tile);

      Vector2 screen_pos =
          camera_world_to_screen(camera, (Vector2){x + 0.5f, y + 0.5f});
      float tile_size = TILE_SIZE_PIXELS * camera->zoom;

      if (renderer_is_position_visible(screen_pos, tile_size / 2)) {
        // Draw the texture
        Rectangle dest_rect = {screen_pos.x - tile_size / 2,
                               screen_pos.y - tile_size / 2, tile_size,
                               tile_size};
        DrawTexturePro(g_tile_atlas.texture, source_rect, dest_rect,
                       (Vector2){0, 0}, 0.0f, WHITE);
      }
    }
  }
}
void renderer_draw_objects(const Object *objects, int count,
                           const Camera2D_RTS *camera) {
  // Check if tree texture is loaded
  if (g_tree_texture.id == 0) {
    // Fall back to colored circles
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
    return;
  }

  // Use texture for objects (trees)
  for (int i = 0; i < count; i++) {
    Object obj = objects[i];
    Vector2 screen_pos =
        camera_world_to_screen(camera, (Vector2){obj.x, obj.y});
    float obj_size = obj.size * TILE_SIZE_PIXELS * camera->zoom;

    if (!renderer_is_position_visible(screen_pos, obj_size / 2))
      continue;

    // Trees might be taller than wide, so we can adjust proportions if needed
    float width = obj_size;
    float height =
        obj_size * ((float)g_tree_texture.height / g_tree_texture.width);

    Rectangle dest_rect = {screen_pos.x - width / 2,
                           screen_pos.y -
                               height / 2, // Center vertically on position
                           width, height};

    // Draw the tree texture
    DrawTexturePro(
        g_tree_texture,
        (Rectangle){0, 0, g_tree_texture.width, g_tree_texture.height},
        dest_rect, (Vector2){0, 0}, // No rotation needed for trees
        0.0f,
        WHITE); // Use original tree colors

    // Optional: Draw a subtle shadow or base circle
    // DrawCircle(screen_pos.x, screen_pos.y + height * 0.4f, width * 0.3f,
    // (Color){0, 0, 0, 128});
  }
}

void renderer_draw_units(const Unit *units, int count,
                         const Camera2D_RTS *camera) {
  // Check if unit texture is loaded
  if (g_unit_texture.id == 0) {
    // Fall back to colored circles
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
    return;
  }

  // Use texture for units - maintain aspect ratio
  for (int i = 0; i < count; i++) {
    Unit unit = units[i];
    Vector2 screen_pos =
        camera_world_to_screen(camera, (Vector2){unit.x, unit.y});
    float unit_size = unit.size * TILE_SIZE_PIXELS * camera->zoom;

    if (!renderer_is_position_visible(screen_pos, unit_size / 2))
      continue;

    // Calculate destination rectangle while maintaining aspect ratio
    float aspect_ratio =
        (float)g_unit_texture.width / (float)g_unit_texture.height;
    float draw_width, draw_height;

    if (aspect_ratio > 1.0f) {
      // Texture is wider than tall
      draw_width = unit_size;
      draw_height = unit_size / aspect_ratio;
    } else {
      // Texture is taller than wide or square
      draw_width = unit_size * aspect_ratio;
      draw_height = unit_size;
    }

    Rectangle dest_rect = {screen_pos.x - draw_width / 2,
                           screen_pos.y - draw_height / 2, draw_width,
                           draw_height};

    // Rotate based on facing direction
    float rotation = unit.facing;

    DrawTexturePro(
        g_unit_texture,
        (Rectangle){0, 0, g_unit_texture.width, g_unit_texture.height},
        dest_rect,
        (Vector2){draw_width / 2, draw_height / 2}, // rotation center
        rotation, WHITE);
  }
}
