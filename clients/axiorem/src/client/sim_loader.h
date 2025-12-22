#ifndef SIM_LOADER_H
#define SIM_LOADER_H

#include "../map/map.h"
#include <stdbool.h>

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
void FreeMap(RawTileMap *map);
RawTileMap *LoadMap(void);

// Tick management
int GetMaxTickFromFile(const char *filename); // New: Get total ticks available
TileMap *TransformMap(RawTileMap *rmap);

#endif
