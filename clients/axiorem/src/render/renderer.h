#ifndef RENDERER_H
#define RENDERER_H

#include "../client/sim_loader.h"
#include "camera.h"
#include "raylib.h"

// Tile atlas configuration
typedef struct {
  Texture2D texture;
  int tile_width;
  int tile_height;
  int gap;
  int columns;
  int rows;
} TileAtlas;

// Function prototypes
void renderer_init_tile_atlas(const char *texture_path, int tile_width,
                              int tile_height, int gap);
void renderer_init_unit_texture(const char *texture_path);
void renderer_cleanup_unit_texture(void);
void renderer_cleanup_tile_atlas(void);

void renderer_init_tree_texture(const char *texture_path);
void renderer_cleanup_tree_texture(void);
void renderer_draw_tile(Vector2 screen_pos, float size, Color color);
bool renderer_is_position_visible(Vector2 screen_pos, float radius);
void renderer_calculate_visible_tile_range(const Camera2D_RTS *camera,
                                           const TileMap *map, int *start_x,
                                           int *start_y, int *end_x,
                                           int *end_y);

void renderer_draw_objects(const Object *objects, int count,
                           const Camera2D_RTS *camera);
void renderer_draw_units(const Unit *units, int count,
                         const Camera2D_RTS *camera);

// Texture-based rendering
void renderer_draw_map_textured(const TileMap *map, const Camera2D_RTS *camera);

// External configuration
extern TileAtlas g_tile_atlas;

#endif
