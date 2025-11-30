#include "client/sim_loader.h"
#include "render/game_window.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  SimulationState *sim = LoadState();
  if (sim == NULL) {
    return 1;
  }

  int result = game_window_run(sim);
  FreeState(sim);

  return result;
}
