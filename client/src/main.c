#include "state.h"
#include "window.h"
#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *network_thread(void *arg) {
  Game *game = (Game *)arg;
  printf("Network thread initialised\n");
  char *address = (char *)arg;
  MemoryData *data = fetch_map(address);
  if (!data) {
    printf("Failed to fetch map from the server\n");
    return NULL;
  }
  TileMap parsed_map;
  int result = parse_map(data->data, &parsed_map);

  if (result != 0) {
    printf("Failed to parse map JSON\n");
    return NULL;
  }

  pthread_mutex_lock(&game->mutex);

  game->map = &parsed_map;
  game->map_ready = 1;

  pthread_cond_signal(&game->cond);
  pthread_mutex_unlock(&game->mutex);

  printf("Map is ready JSON\n");

  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Please, provide valid server connection string, provided: %d\n",
           argc);
    return -1;
  }

  Game *g = init_game(argv[1]);

  if (!g) {
    return -1;
  }

  pthread_t n_tid;
  pthread_create(&n_tid, NULL, network_thread, (void *)argv[1]);
  printf("Main thread initialised\n");
  create_window(640, 480);
  TileMap *map = get_map(g);
  pthread_join(n_tid, NULL);
  destroy_game(g);
  return EXIT_SUCCESS;
}
