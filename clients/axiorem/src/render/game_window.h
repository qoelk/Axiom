#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

#include "../client/sim_loader.h"
#include "camera.h"
#include "renderer.h"
#include "ui.h"

/**
 * @brief Main game window management
 *
 * Handles the primary game loop, window creation, and high-level
 * game state management for the Axiom Battlefield simulation.
 */

typedef struct {
  int screen_width;
  int screen_height;
  const char *window_title;
  int target_fps;
} GameWindowConfig;

int game_window_run(SimulationState *sim);
void game_window_handle_input(SimulationState *sim, Camera2D_RTS *camera,
                              int *current_tick, bool *paused);
void game_window_render_frame(const SimulationState *sim,
                              const Camera2D_RTS *camera, int current_tick,
                              bool paused);

#endif
