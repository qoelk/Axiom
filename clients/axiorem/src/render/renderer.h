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
void renderer_cleanup_tile_atlas(void);

Color renderer_get_tile_color(TileType tile);
void renderer_draw_tile(Vector2 screen_pos, float size, Color color,
                        bool draw_grid);
bool renderer_is_position_visible(Vector2 screen_pos, float radius);
void renderer_calculate_visible_tile_range(const Camera2D_RTS *camera,
                                           const TileMap *map, int *start_x,
                                           int *start_y, int *end_x,
                                           int *end_y);

void renderer_draw_map(const TileMap *map, const Camera2D_RTS *camera);
void renderer_draw_objects(const Object *objects, int count,
                           const Camera2D_RTS *camera);
void renderer_draw_units(const Unit *units, int count,
                         const Camera2D_RTS *camera);

// Texture-based rendering
void renderer_draw_map_textured(const TileMap *map, const Camera2D_RTS *camera);
Rectangle renderer_get_tile_source_rect(TileType tile_type, int x, int y,
                                        const TileMap *map);

// External configuration
extern TileAtlas g_tile_atlas;

#endif
