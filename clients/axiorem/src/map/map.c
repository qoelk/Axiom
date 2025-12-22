#include "map.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static TileType TILE_MAPPINGS[] = {
    // Basic terrain tiles
    {.key = TILE_WATER,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {0, 0}},
    {.key = TILE_LAND,
     .raw_key = R_TILE_LAND,
     .variation = 1,
     .atlas_coords = {5, 0}},
    {.key = TILE_DIRT,
     .raw_key = R_TILE_DIRT,
     .variation = 2,
     .atlas_coords = {3, 3}},
    {.key = TILE_ROCK,
     .raw_key = R_TILE_ROCK,
     .variation = 2,
     .atlas_coords = {4, 4}},

    // Water-to-land transition tiles (water center with land edges)
    {.key = TILE_WATER_LAND_TL_L_T,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {2, 0}},
    {.key = TILE_WATER_LAND_TL,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {1, 2}},
    {.key = TILE_WATER_LAND_L,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {2, 1}}, // Middle-left side (water with land on left)
    {.key = TILE_WATER_LAND_BL_L_B,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {2, 2}}, // Bottom-left corner (water with land in BL)
    {.key = TILE_WATER_LAND_BL,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {1, 1}},
    {.key = TILE_WATER_LAND_T,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {3, 0}}, // Top-middle side (water with land on top)
    {.key = TILE_WATER_LAND_B,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {3, 2}}, // Bottom-middle side (water with land on bottom)
    {.key = TILE_WATER_LAND_TR_R_T,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {4, 0}}, // Top-right corner (water with land in TR)
    {.key = TILE_WATER_LAND_TR,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {0, 2}},
    {.key = TILE_WATER_LAND_R,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {4, 1}}, // Middle-right side (water with land on right)
    {.key = TILE_WATER_LAND_BR_R_B,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {4, 2}}, // Bottom-right corner (water with land in BR)
    {.key = TILE_WATER_LAND_BR,
     .raw_key = R_TILE_WATER,
     .variation = 1,
     .atlas_coords = {0, 1}},
};

Tile raw_to_tile(RawTileKey raw_key) {
  Tile tile = {.elevation = 0,
               .texture_index_x = 0,
               .texture_index_y = 0,
               .raw_key = raw_key};

  for (size_t i = 0; i < sizeof(TILE_MAPPINGS) / sizeof(TILE_MAPPINGS[0]);
       i++) {
    if (TILE_MAPPINGS[i].raw_key == raw_key) {
      tile.key = TILE_MAPPINGS[i].key;
    }
  }
  update_coordinates(&tile);
  return tile;
}

void update_coordinates(Tile *tile) {
  for (size_t i = 0; i < sizeof(TILE_MAPPINGS) / sizeof(TILE_MAPPINGS[0]);
       i++) {
    if (TILE_MAPPINGS[i].key == tile->key) {
      tile->texture_index_x = TILE_MAPPINGS[i].atlas_coords[0];
      tile->texture_index_y = TILE_MAPPINGS[i].atlas_coords[1];
      return;
    }
  }
  abort();
}

TileKey get_neighbor_at_offset(TileMap *map, int x, int y, int dx, int dy) {
  int nx = x + dx;
  int ny = y + dy;

  // Check bounds
  if (nx < 0 || nx >= map->width || ny < 0 || ny >= map->height) {
    return TILE_UNKNOWN;
  }

  int idx = ny * map->width + nx;
  return map->tiles[idx].key;
}

void preprocess_map(TileMap *map) {
  for (int y = 0; y < map->height; y++) {
    for (int x = 0; x < map->width; x++) {
      Tile tile = map->tiles[y * map->width + x];
      if (tile.raw_key != R_TILE_WATER) {
        continue;
      }

      // Get neighbor tile keys
      TileKey tl = get_neighbor_at_offset(map, x, y, -1, -1);
      TileKey t = get_neighbor_at_offset(map, x, y, 0, -1);
      TileKey tr = get_neighbor_at_offset(map, x, y, 1, -1);
      TileKey l = get_neighbor_at_offset(map, x, y, -1, 0);
      TileKey r = get_neighbor_at_offset(map, x, y, 1, 0);
      TileKey bl = get_neighbor_at_offset(map, x, y, -1, 1);
      TileKey b = get_neighbor_at_offset(map, x, y, 0, 1);
      TileKey br = get_neighbor_at_offset(map, x, y, 1, 1);

      // Convert to booleans
      bool is_tl_land = (tl == TILE_LAND);
      bool is_t_land = (t == TILE_LAND);
      bool is_tr_land = (tr == TILE_LAND);
      bool is_l_land = (l == TILE_LAND);
      bool is_r_land = (r == TILE_LAND);
      bool is_bl_land = (bl == TILE_LAND);
      bool is_b_land = (b == TILE_LAND);
      bool is_br_land = (br == TILE_LAND);

      TileKey key = TILE_WATER;

      if (is_tl_land) {
        key = TILE_WATER_LAND_TL;
      }

      if (is_tr_land) {
        key = TILE_WATER_LAND_TR;
      }

      if (is_bl_land) {
        key = TILE_WATER_LAND_BL;
      }

      if (is_br_land) {
        key = TILE_WATER_LAND_BR;
      }

      if (is_t_land) {
        key = TILE_WATER_LAND_T;
      }

      if (is_b_land) {
        key = TILE_WATER_LAND_B;
      }

      if (is_l_land) {
        key = TILE_WATER_LAND_L;
      }

      if (is_r_land) {
        key = TILE_WATER_LAND_R;
      }

      if (is_t_land && is_tl_land && is_l_land) {
        key = TILE_WATER_LAND_TL_L_T;
      }

      if (is_t_land && is_tr_land && is_r_land) {
        key = TILE_WATER_LAND_TR_R_T;
      }

      if (is_b_land && is_bl_land && is_l_land) {
        key = TILE_WATER_LAND_BL_L_B;
      }

      if (is_b_land && is_br_land && is_r_land) {
        key = TILE_WATER_LAND_BR_R_B;
      }
      map->tiles[y * map->width + x].key = key;
      update_coordinates(&map->tiles[y * map->width + x]);
    }
  }
}
