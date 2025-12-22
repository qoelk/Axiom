#ifndef MAP_H
#define MAP_H

typedef enum { R_TILE_WATER, R_TILE_LAND, R_TILE_DIRT, R_TILE_ROCK } RawTileKey;

typedef enum {
  // Basic terrain tiles
  TILE_WATER,
  TILE_LAND,
  TILE_DIRT,
  TILE_ROCK,
  TILE_UNKNOWN,

  // Water-to-land transition tiles (water center with land edges)
  TILE_WATER_LAND_TL_L_T,
  TILE_WATER_LAND_TL,
  TILE_WATER_LAND_L,
  TILE_WATER_LAND_BL_L_B,
  TILE_WATER_LAND_BL,
  TILE_WATER_LAND_T,
  TILE_WATER_LAND_B,
  TILE_WATER_LAND_TR_R_T,
  TILE_WATER_LAND_TR,
  TILE_WATER_LAND_R,
  TILE_WATER_LAND_BR_R_B,
  TILE_WATER_LAND_BR,

} TileKey;
typedef struct {
  int width;
  int height;
  RawTileKey *tiles;
} RawTileMap;

typedef struct {
  RawTileKey raw_key;
  TileKey key;
  int variation;
  int atlas_coords[2];
} TileType;

typedef struct {
  RawTileKey raw_key;
  TileKey key;
  int elevation;
  int texture_index_x;
  int texture_index_y;
} Tile;

typedef struct {
  int width;
  int height;
  Tile *tiles;
} TileMap;

Tile raw_to_tile(RawTileKey raw_key);
void update_coordinates(Tile *tile);
TileKey get_neighbor_at_offset(TileMap *map, int x, int y, int dx, int dy);
void preprocess_map(TileMap *map);
#endif
