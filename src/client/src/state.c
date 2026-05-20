#include "state.h"
#include "cJSON.h"
#include <curl/curl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

int parse_map(const char *json_str, TileMap *m) {
  cJSON *root = cJSON_Parse(json_str);
  if (!root) {
    return 1;
  }
  cJSON *width = cJSON_GetObjectItem(root, "width");
  cJSON *height = cJSON_GetObjectItem(root, "height");
  cJSON *tiles = cJSON_GetObjectItem(root, "tiles");
  m->tiles_l = cJSON_GetArraySize(tiles);

  m->width = width->valueint;
  m->height = height->valueint;

  if (m->tiles_l != m->width * m->height) {
    cJSON_Delete(root);
    return 1;
  }

  int *tiles_p = malloc(sizeof(int) * m->tiles_l);
  if (!tiles_p) {
    cJSON_Delete(root);
    return 1;
  }

  for (int i = 0; i < m->tiles_l; i++) {
    cJSON *tile = cJSON_GetArrayItem(tiles, i);
    tiles_p[i] = tile->valueint;
  }

  m->tiles = tiles_p;
  cJSON_Delete(root);
  return 0;
}

static size_t write_callback(void *contents, size_t size, size_t nmemb,
                             void *userp) {
  size_t r_size = size * nmemb;
  MemoryData *mem = (MemoryData *)userp;

  char *ptr = realloc(mem->data, mem->size + r_size + 1);
  if (!ptr) {
    return 0;
  }

  mem->data = ptr;
  memcpy(&(mem->data[mem->size]), contents, r_size);
  mem->size += r_size;
  mem->data[mem->size] = '\0';

  return r_size;
}

MemoryData *fetch_map(char *addr) {
  MemoryData *resp = malloc(sizeof(MemoryData));
  if (!resp) {
    return NULL;
  }

  resp->data = malloc(1);
  if (!resp->data) {
    free(resp);
    return NULL;
  }
  resp->size = 0;
  resp->data[0] = '\0';

  CURL *curl = curl_easy_init();
  if (!curl) {
    printf("Failed to initialise CURL\n");
    return NULL;
  }

  curl_easy_setopt(curl, CURLOPT_URL, strcat(addr, "/map"));
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)resp);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    free(resp->data);
    free(resp);
    curl_easy_cleanup(curl);
    return NULL;
  }
  curl_easy_cleanup(curl);
  return resp;
}

Game *init_game(const char *server_address) {
  Game *g = NULL;
  g = malloc(sizeof(Game));

  if (!g) {
    return NULL;
  }

  pthread_mutex_init(&g->mutex, NULL);
  pthread_cond_init(&g->cond, NULL);

  g->map = init_map();
  g->map_ready = 0;
  g->server_address = strdup(server_address);

  return g;
}

TileMap *init_map() {
  TileMap *m = NULL;
  m = malloc(sizeof(TileMap));
  return m;
}

void destroy_game(Game *g) {
  if (g) {
    pthread_mutex_destroy(&g->mutex);
    pthread_cond_destroy(&g->cond);
    free(&g->server_address);
    free(g->map->tiles);
    free(g->map);
    free(g);
  }
}

TileMap *get_map(Game *g) {
  pthread_mutex_lock(&g->mutex);
  while (!g->map_ready) {
    pthread_cond_wait(&g->cond, &g->mutex);
  }

  return g->map;
}
