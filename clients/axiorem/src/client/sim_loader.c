#include "sim_loader.h"
#include <cjson/cJSON.h> // You'll need the cJSON library for this
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to parse TileMap from JSON
TileMap *ParseMapFromJSON(cJSON *mapJson) {
  if (!mapJson)
    return NULL;

  cJSON *widthJson = cJSON_GetObjectItem(mapJson, "width");
  cJSON *heightJson = cJSON_GetObjectItem(mapJson, "height");
  cJSON *tilesJson = cJSON_GetObjectItem(mapJson, "tiles");

  if (!widthJson || !heightJson || !tilesJson || !cJSON_IsArray(tilesJson)) {
    return NULL;
  }

  TileMap *map = (TileMap *)malloc(sizeof(TileMap));
  if (!map)
    return NULL;

  map->width = widthJson->valueint;
  map->height = heightJson->valueint;

  int totalTiles = map->width * map->height;
  map->tiles = (TileType *)malloc(totalTiles * sizeof(TileType));
  if (!map->tiles) {
    free(map);
    return NULL;
  }

  // Parse tiles array
  cJSON *tileItem;
  int i = 0;
  cJSON_ArrayForEach(tileItem, tilesJson) {
    if (i < totalTiles) {
      map->tiles[i] = (TileType)tileItem->valueint;
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

// Main function to load state from JSON file
SimulationState *LoadStateFromFile(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (!file) {
    printf("Error: Could not open file %s\n", filename);
    return NULL;
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Read file content
  char *file_content = (char *)malloc(file_size + 1);
  if (!file_content) {
    fclose(file);
    return NULL;
  }

  fread(file_content, 1, file_size, file);
  file_content[file_size] = '\0';
  fclose(file);

  // Parse JSON
  cJSON *json = cJSON_Parse(file_content);
  free(file_content);

  if (!json) {
    printf("Error: Failed to parse JSON\n");
    return NULL;
  }

  // Create simulation state
  SimulationState *state = (SimulationState *)malloc(sizeof(SimulationState));
  if (!state) {
    cJSON_Delete(json);
    return NULL;
  }

  // Parse map
  cJSON *mapJson = cJSON_GetObjectItem(json, "map");
  TileMap *map = ParseMapFromJSON(mapJson);
  if (!map) {
    printf("Error: Failed to parse map from JSON\n");
    cJSON_Delete(json);
    free(state);
    return NULL;
  }
  state->map = *map;
  free(map); // We've copied the data, so free the temporary map

  // Parse first state from the state array
  cJSON *stateArrayJson = cJSON_GetObjectItem(json, "state");
  if (!stateArrayJson || !cJSON_IsArray(stateArrayJson) ||
      cJSON_GetArraySize(stateArrayJson) == 0) {
    printf("Error: No state array found in JSON\n");
    cJSON_Delete(json);
    FreeState(state);
    return NULL;
  }

  cJSON *firstStateJson = cJSON_GetArrayItem(stateArrayJson, 0);

  // Parse paused flag
  cJSON *pausedJson = cJSON_GetObjectItem(firstStateJson, "paused");
  state->paused = pausedJson ? cJSON_IsTrue(pausedJson) : false;

  // Parse objects
  cJSON *objectsJson = cJSON_GetObjectItem(firstStateJson, "objects");
  state->objects = ParseObjectsFromJSON(objectsJson, &state->objectCount);

  // Parse units
  cJSON *unitsJson = cJSON_GetObjectItem(firstStateJson, "units");
  state->units = ParseUnitsFromJSON(unitsJson, &state->unitCount);

  cJSON_Delete(json);
  return state;
}

// Modified LoadState to use file loading
SimulationState *LoadState() {
  return LoadStateFromFile("../assets/test.sim.json");
}

// Keep the existing helper functions (FreeState, FreeMap, etc.) as they are
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

void FreeMap(TileMap *map) {
  if (map) {
    if (map->tiles)
      free(map->tiles);
    free(map);
  }
}

TileMap *LoadMap() {
  // For backward compatibility, you can keep this or modify it to load from
  // JSON
  SimulationState *state = LoadState();
  if (state) {
    TileMap *map = (TileMap *)malloc(sizeof(TileMap));
    if (map) {
      // Copy map data
      map->width = state->map.width;
      map->height = state->map.height;
      map->tiles =
          (TileType *)malloc(map->width * map->height * sizeof(TileType));
      if (map->tiles) {
        memcpy(map->tiles, state->map.tiles,
               map->width * map->height * sizeof(TileType));
      }
    }
    FreeState(state);
    return map;
  }
  return NULL;
}
