#ifndef SIM_LOADER_H
#define SIM_LOADER_H

#include <stdbool.h>
typedef enum TileType {
  TILE_WATER = 0,
  TILE_LAND = 1,
  TILE_DIRT = 2,
  TILE_ROCK = 3
} TileType;

typedef struct {
  int width;
  int height;
  TileType *tiles;

} TileMap;

typedef struct Object {
  float x, y;
  float size;
} Object;

typedef struct Unit {
  float x, y;
  float size;
  float facing;
  float velocity;
  int owner;
} Unit;

typedef struct SimulationState {
  TileMap map;
  Object *objects;
  int objectCount;
  Unit *units;
  int unitCount;
  bool paused;
} SimulationState;

TileMap *LoadMap();

SimulationState *LoadState();
void FreeState(SimulationState *state);
#endif
