#include "sim_loader.h"
#include "map.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to parse TileMap from JSON
RawTileMap *ParseMapFromJSON(cJSON *mapJson) {
  if (!mapJson)
    return NULL;

  cJSON *widthJson = cJSON_GetObjectItem(mapJson, "width");
  cJSON *heightJson = cJSON_GetObjectItem(mapJson, "height");
  cJSON *tilesJson = cJSON_GetObjectItem(mapJson, "tiles");

  if (!widthJson || !heightJson || !tilesJson || !cJSON_IsArray(tilesJson)) {
    return NULL;
  }

  RawTileMap *map = (RawTileMap *)malloc(sizeof(RawTileMap));
  if (!map)
    return NULL;

  map->width = widthJson->valueint;
  map->height = heightJson->valueint;

  int totalTiles = map->width * map->height;
  map->tiles = (RawTileKey *)malloc(totalTiles * sizeof(RawTileKey));
  if (!map->tiles) {
    free(map);
    return NULL;
  }

  // Parse tiles array
  cJSON *tileItem;
  int i = 0;
  cJSON_ArrayForEach(tileItem, tilesJson) {
    if (i < totalTiles) {
      map->tiles[i] = (RawTileKey)tileItem->valueint;
      i++;
    }
  }

  return map;
}

// Helper function to parse objects from JSON
Object *ParseObjectsFromJSON(cJSON *objectsJson, int *objectCount) {
  if (!objectsJson || !cJSON_IsArray(objectsJson)) {
    return NULL;
  }

  *objectCount = cJSON_GetArraySize(objectsJson);
  if (*objectCount == 0) {
    return NULL;
  }

  Object *objects = (Object *)malloc(*objectCount * sizeof(Object));
  if (!objects) {
    return NULL;
  }

  cJSON *objectItem;
  int i = 0;
  cJSON_ArrayForEach(objectItem, objectsJson) {
    cJSON *xJson = cJSON_GetObjectItem(objectItem, "x");
    cJSON *yJson = cJSON_GetObjectItem(objectItem, "y");
    cJSON *sizeJson = cJSON_GetObjectItem(objectItem, "size");

    if (xJson && yJson && sizeJson) {
      objects[i] = (Object){.x = (float)xJson->valuedouble,
                            .y = (float)yJson->valuedouble,
                            .size = (float)sizeJson->valuedouble};
      i++;
    }
  }

  return objects;
}

// Helper function to parse units from JSON
Unit *ParseUnitsFromJSON(cJSON *unitsJson, int *unitCount) {
  if (!unitsJson || !cJSON_IsArray(unitsJson)) {
    return NULL;
  }

  *unitCount = cJSON_GetArraySize(unitsJson);
  if (*unitCount == 0) {
    return NULL;
  }

  Unit *units = (Unit *)malloc(*unitCount * sizeof(Unit));
  if (!units) {
    return NULL;
  }

  cJSON *unitItem;
  int i = 0;
  cJSON_ArrayForEach(unitItem, unitsJson) {
    cJSON *xJson = cJSON_GetObjectItem(unitItem, "x");
    cJSON *yJson = cJSON_GetObjectItem(unitItem, "y");
    cJSON *sizeJson = cJSON_GetObjectItem(unitItem, "size");
    cJSON *facingJson = cJSON_GetObjectItem(unitItem, "facing");
    cJSON *velocityJson = cJSON_GetObjectItem(unitItem, "velocity");
    cJSON *ownerJson = cJSON_GetObjectItem(unitItem, "owner");

    if (xJson && yJson && sizeJson && facingJson && velocityJson && ownerJson) {
      units[i] = (Unit){.x = (float)xJson->valuedouble,
                        .y = (float)yJson->valuedouble,
                        .size = (float)sizeJson->valuedouble,
                        .facing = (float)facingJson->valuedouble,
                        .velocity = (float)velocityJson->valuedouble,
                        .owner = ownerJson->valueint};
      i++;
    }
  }

  return units;
}

// Get the maximum tick available in the simulation file
int GetMaxTickFromFile(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("Error: Could not open file %s\n", filename);
    return 0;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *file_content = (char *)malloc(file_size + 1);
  if (!file_content) {
    fclose(file);
    return 0;
  }

  fread(file_content, 1, file_size, file);
  file_content[file_size] = '\0';
  fclose(file);

  cJSON *json = cJSON_Parse(file_content);
  free(file_content);

  if (!json) {
    printf("Error: Failed to parse JSON\n");
    return 0;
  }

  cJSON *stateArrayJson = cJSON_GetObjectItem(json, "state");
  int maxTick = 0;
  if (stateArrayJson && cJSON_IsArray(stateArrayJson)) {
    maxTick = cJSON_GetArraySize(stateArrayJson) - 1; // 0-based indexing
  }

  cJSON_Delete(json);
  return maxTick;
}

// Load state at specific tick
SimulationState *LoadStateAtTick(const char *filename, int tick) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("Error: Could not open file %s\n", filename);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *file_content = (char *)malloc(file_size + 1);
  if (!file_content) {
    fclose(file);
    return NULL;
  }

  fread(file_content, 1, file_size, file);
  file_content[file_size] = '\0';
  fclose(file);

  cJSON *json = cJSON_Parse(file_content);
  free(file_content);

  if (!json) {
    printf("Error: Failed to parse JSON\n");
    return NULL;
  }

  SimulationState *state = (SimulationState *)malloc(sizeof(SimulationState));
  if (!state) {
    cJSON_Delete(json);
    return NULL;
  }

  // Parse map (static across all ticks)
  cJSON *mapJson = cJSON_GetObjectItem(json, "map");
  RawTileMap *map = ParseMapFromJSON(mapJson);
  if (!map) {
    printf("Error: Failed to parse map from JSON\n");
    cJSON_Delete(json);
    free(state);
    return NULL;
  }
  state->map = *TransformMap(map);
  free(map);

  // Parse state array and get specific tick
  cJSON *stateArrayJson = cJSON_GetObjectItem(json, "state");
  if (!stateArrayJson || !cJSON_IsArray(stateArrayJson)) {
    printf("Error: No state array found in JSON\n");
    cJSON_Delete(json);
    FreeState(state);
    return NULL;
  }

  int maxTick = cJSON_GetArraySize(stateArrayJson) - 1;
  state->totalTicks = maxTick + 1; // Store total ticks available

  // Clamp tick to valid range
  if (tick < 0)
    tick = 0;
  if (tick > maxTick)
    tick = maxTick;

  cJSON *tickStateJson = cJSON_GetArrayItem(stateArrayJson, tick);
  if (!tickStateJson) {
    printf("Error: Could not load tick %d\n", tick);
    cJSON_Delete(json);
    FreeState(state);
    return NULL;
  }

  // Parse paused flag
  cJSON *pausedJson = cJSON_GetObjectItem(tickStateJson, "paused");
  state->paused = pausedJson ? cJSON_IsTrue(pausedJson) : false;

  // Parse objects
  cJSON *objectsJson = cJSON_GetObjectItem(tickStateJson, "objects");
  state->objects = ParseObjectsFromJSON(objectsJson, &state->objectCount);

  // Parse units
  cJSON *unitsJson = cJSON_GetObjectItem(tickStateJson, "units");
  state->units = ParseUnitsFromJSON(unitsJson, &state->unitCount);

  cJSON_Delete(json);
  return state;
}

// Main function to load state from JSON file (loads tick 0 by default)
SimulationState *LoadStateFromFile(const char *filename) {
  return LoadStateAtTick(filename, 0);
}

// Modified LoadState to use file loading
SimulationState *LoadState(void) {
  return LoadStateFromFile("../assets/test.sim.json");
}

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

void FreeMap(RawTileMap *map) {
  if (map) {
    if (map->tiles)
      free(map->tiles);
    free(map);
  }
}

RawTileMap *LoadMap() {
  SimulationState *state = LoadState();
  if (state) {
    RawTileMap *map = (RawTileMap *)malloc(sizeof(RawTileMap));
    if (map) {
      map->width = state->map.width;
      map->height = state->map.height;
      map->tiles =
          (RawTileKey *)malloc(map->width * map->height * sizeof(RawTileKey));
      if (map->tiles) {
        memcpy(map->tiles, state->map.tiles,
               map->width * map->height * sizeof(RawTileKey));
      }
    }
    FreeState(state);
    return map;
  }
  return NULL;
}

TileMap *TransformMap(RawTileMap *rmap) {
  if (!rmap || !rmap->tiles) {
    return NULL;
  }

  TileMap *tmap = (TileMap *)malloc(sizeof(TileMap));
  if (!tmap) {
    return NULL;
  }

  tmap->width = rmap->width;
  tmap->height = rmap->height;
  int totalTiles = tmap->width * tmap->height;

  tmap->tiles = (Tile *)malloc(totalTiles * sizeof(Tile));
  if (!tmap->tiles) {
    free(tmap);
    return NULL;
  }

  for (int i = 0; i < totalTiles; i++) {
    tmap->tiles[i] = raw_to_tile(rmap->tiles[i]);
  }

  preprocess_map(tmap);

  return tmap;
}
