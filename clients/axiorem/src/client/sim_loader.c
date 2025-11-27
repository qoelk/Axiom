#include "sim_loader.h"
#include <stdio.h>
#include <stdlib.h>

TileMap *LoadMap() {
  TileMap *map = (TileMap *)malloc(sizeof(TileMap));
  if (!map)
    return NULL;

  map->width = 10;
  map->height = 10;

  map->tiles = (TileType *)malloc(map->width * map->height * sizeof(TileType));
  if (!map->tiles) {
    free(map);
    return NULL;
  }

  TileType hardcodedTiles[] = {
      // Row 0
      TILE_WATER, TILE_WATER, TILE_LAND, TILE_LAND, TILE_LAND, TILE_LAND,
      TILE_LAND, TILE_LAND, TILE_DIRT, TILE_DIRT,
      // Row 1
      TILE_WATER, TILE_LAND, TILE_LAND, TILE_LAND, TILE_LAND, TILE_DIRT,
      TILE_DIRT, TILE_DIRT, TILE_DIRT, TILE_ROCK,
      // Row 2
      TILE_LAND, TILE_LAND, TILE_LAND, TILE_LAND, TILE_DIRT, TILE_DIRT,
      TILE_DIRT, TILE_DIRT, TILE_ROCK, TILE_ROCK,
      // Row 3
      TILE_LAND, TILE_LAND, TILE_LAND, TILE_DIRT, TILE_DIRT, TILE_DIRT,
      TILE_DIRT, TILE_ROCK, TILE_ROCK, TILE_ROCK,
      // Row 4
      TILE_LAND, TILE_LAND, TILE_DIRT, TILE_DIRT, TILE_DIRT, TILE_DIRT,
      TILE_ROCK, TILE_ROCK, TILE_ROCK, TILE_ROCK,
      // Row 5
      TILE_LAND, TILE_DIRT, TILE_DIRT, TILE_DIRT, TILE_DIRT, TILE_ROCK,
      TILE_ROCK, TILE_ROCK, TILE_ROCK, TILE_ROCK,
      // Row 6
      TILE_DIRT, TILE_DIRT, TILE_DIRT, TILE_DIRT, TILE_ROCK, TILE_ROCK,
      TILE_ROCK, TILE_ROCK, TILE_ROCK, TILE_ROCK,
      // Row 7
      TILE_DIRT, TILE_DIRT, TILE_DIRT, TILE_ROCK, TILE_ROCK, TILE_ROCK,
      TILE_ROCK, TILE_ROCK, TILE_ROCK, TILE_ROCK,
      // Row 8
      TILE_DIRT, TILE_DIRT, TILE_ROCK, TILE_ROCK, TILE_ROCK, TILE_ROCK,
      TILE_ROCK, TILE_ROCK, TILE_ROCK, TILE_ROCK,
      // Row 9
      TILE_DIRT, TILE_ROCK, TILE_ROCK, TILE_ROCK, TILE_ROCK, TILE_ROCK,
      TILE_ROCK, TILE_ROCK, TILE_ROCK, TILE_ROCK};

  for (int i = 0; i < map->width * map->height; i++) {
    map->tiles[i] = hardcodedTiles[i];
  }

  return map;
}
SimulationState *LoadState() {
  // Create and initialize a SimulationState with hardcoded data
  SimulationState *state = (SimulationState *)malloc(sizeof(SimulationState));
  if (!state)
    return NULL;

  // Load the map
  state->map = *LoadMap();

  // Create hardcoded objects
  state->objectCount = 3;
  state->objects = (Object *)malloc(state->objectCount * sizeof(Object));
  if (!state->objects) {
    free(state->map.tiles);
    free(state);
    return NULL;
  }

  // Hardcoded object data
  state->objects[0] =
      (Object){2.5f, 1.5f, 1.0f}; // Object at (2.5, 1.5) with size 1.0
  state->objects[1] =
      (Object){4.0f, 3.0f, 0.5f}; // Object at (4.0, 3.0) with size 0.5
  state->objects[2] =
      (Object){1.0f, 4.0f, 1.5f}; // Object at (1.0, 4.0) with size 1.5

  // Create hardcoded units
  state->unitCount = 2;
  state->units = (Unit *)malloc(state->unitCount * sizeof(Unit));
  if (!state->units) {
    free(state->objects);
    free(state->map.tiles);
    free(state);
    return NULL;
  }

  // Hardcoded unit data
  state->units[0] =
      (Unit){1.0f, 1.0f, 0.8f, 45.0f, 2.5f, 1}; // Unit for player 1
  state->units[1] =
      (Unit){3.0f, 3.0f, 0.8f, 270.0f, 1.5f, 2}; // Unit for player 2

  return state;
}

// Optional: Function to free the SimulationState
void FreeState(SimulationState *state) {
  if (state) {
    if (state->map.tiles)
      free(state->map.tiles);
    if (state->objects)
      free(state->objects);
    if (state->units)
      free(state->units);
    free(state);
  }
}

// Optional: Function to free just a TileMap
void FreeMap(TileMap *map) {
  if (map) {
    if (map->tiles)
      free(map->tiles);
    free(map);
  }
}
