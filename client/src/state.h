#ifndef STATE_H
#define STATE_H

#include <pthread.h>
#include <stddef.h>

typedef struct {
  int width;
  int height;
  int tiles_l;
  int *tiles;
} TileMap;

typedef struct {
  TileMap *map;
  pthread_mutex_t mutex;
  int map_ready;
  pthread_cond_t cond;
  char *server_address;
} Game;

typedef struct {
  char *data;
  size_t size;
} MemoryData;

Game *init_game(const char *server_address);
TileMap *init_map();
static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp);
int parse_map(const char *json_str, TileMap *m);
MemoryData *fetch_map(char *addr);
TileMap *get_map(Game *g);
void destroy_game(Game *g);
#endif
