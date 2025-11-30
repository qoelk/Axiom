#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

#include "../client/sim_loader.h"
#include "../utils/math_utils.h"
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
  bool fullscreen;
} GameWindowConfig;

// Game state management
typedef struct {
  SimulationState *sim;
  int current_tick;
  int max_tick;
  bool paused;
  char filename[256];
} GameState;

int game_window_run(SimulationState *sim);
void game_window_handle_input(GameState *game_state, Camera2D_RTS *camera);
void game_window_render_frame(const GameState *game_state,
                              const Camera2D_RTS *camera);
void game_window_toggle_fullscreen(void);
void game_window_load_tick(GameState *game_state, int tick);

#endif
