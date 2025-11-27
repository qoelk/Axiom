#ifndef SIM_LOADER_H
#define SIM_LOADER_H

#include <stdbool.h>

typedef enum { TILE_WATER, TILE_LAND, TILE_DIRT, TILE_ROCK } TileType;

typedef struct {
  int width;
  int height;
  TileType *tiles;
} TileMap;

typedef struct {
  float x;
  float y;
  float size;
} Object;

typedef struct {
  float x;
  float y;
  float size;
  float facing;
  float velocity;
  int owner;
} Unit;

typedef struct {
  TileMap map;
  Object *objects;
  int objectCount;
  Unit *units;
  int unitCount;
  bool paused;
  int totalTicks; // Added: total ticks available in simulation
} SimulationState;

// Function declarations
SimulationState *LoadState(void);
SimulationState *LoadStateFromFile(const char *filename);
SimulationState *LoadStateAtTick(const char *filename,
                                 int tick); // New: Load specific tick
void FreeState(SimulationState *state);
void FreeMap(TileMap *map);
TileMap *LoadMap(void);

// Tick management
int GetMaxTickFromFile(const char *filename); // New: Get total ticks available

#endif
