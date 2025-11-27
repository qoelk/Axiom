#ifndef RENDERER_H
#define RENDERER_H

#include "../client/sim_loader.h"
#include "camera.h"
#include "raylib.h"

/**
 * @brief Rendering system for game entities
 *
 * Handles rendering of map tiles, objects, and units with
 * visibility culling and level-of-detail optimizations.
 */

// Tile rendering
Color renderer_get_tile_color(TileType tile);
void renderer_draw_tile(Vector2 screen_pos, float size, Color color,
                        bool draw_grid);

// Entity rendering
void renderer_draw_map(const TileMap *map, const Camera2D_RTS *camera);
void renderer_draw_objects(const Object *objects, int count,
                           const Camera2D_RTS *camera);
void renderer_draw_units(const Unit *units, int count,
                         const Camera2D_RTS *camera);

// Visibility and culling
bool renderer_is_position_visible(Vector2 screen_pos, float radius);
void renderer_calculate_visible_tile_range(const Camera2D_RTS *camera,
                                           const TileMap *map, int *start_x,
                                           int *start_y, int *end_x,
                                           int *end_y);

#endif
