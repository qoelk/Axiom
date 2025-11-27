#include "game_window.h"
#include "../utils/math_utils.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>

static GameWindowConfig default_config = {.screen_width = 800,
                                          .screen_height = 600,
                                          .window_title =
                                              "Axiom - AI Battlefield",
                                          .target_fps = 60};

int game_window_run(SimulationState *sim) {
  if (sim == NULL) {
    TraceLog(LOG_ERROR, "GameWindow: NULL simulation state provided");
    return 1;
  }

  InitWindow(default_config.screen_width, default_config.screen_height,
             default_config.window_title);
  SetTargetFPS(default_config.target_fps);

  if (!IsWindowReady()) {
    TraceLog(LOG_ERROR, "GameWindow: Failed to initialize window");
    return 1;
  }

  Camera2D_RTS camera;
  CameraConfig cam_config = {.screen_width = default_config.screen_width,
                             .screen_height = default_config.screen_height,
                             .camera_move_speed = DEFAULT_CAMERA_SPEED,
                             .camera_zoom_speed = 0.1f};

  camera_init(&camera, &cam_config, &sim->map);

  int current_tick = 0;
  bool paused = false;

  TraceLog(LOG_INFO, "GameWindow: Starting main game loop");

  while (!WindowShouldClose()) {
    camera_update(&camera, &sim->map);
    game_window_handle_input(sim, &camera, &current_tick, &paused);

    BeginDrawing();
    game_window_render_frame(sim, &camera, current_tick, paused);
    EndDrawing();
  }

  CloseWindow();
  TraceLog(LOG_INFO, "GameWindow: Shutdown complete");
  return 0;
}

void game_window_handle_input(SimulationState *sim, Camera2D_RTS *camera,
                              int *current_tick, bool *paused) {
  // Space: Advance simulation tick
  if (IsKeyPressed(KEY_SPACE)) {
    (*current_tick)++;
    TraceLog(LOG_DEBUG, "GameWindow: Advanced to tick %d", *current_tick);
  }

  // R: Reset simulation
  if (IsKeyPressed(KEY_R)) {
    TraceLog(LOG_INFO, "GameWindow: Resetting simulation");
    FreeState(sim);
    SimulationState *new_sim = LoadState();
    if (new_sim != NULL) {
      *sim = *new_sim;
      free(new_sim);
    }
    *current_tick = 0;
    CameraConfig config = {GetScreenWidth(), GetScreenHeight()};
    camera_init(camera, &config, &sim->map);
  }

  // P: Pause/unpause simulation
  if (IsKeyPressed(KEY_P)) {
    *paused = !(*paused);
    TraceLog(LOG_INFO, "GameWindow: Simulation %s",
             *paused ? "paused" : "resumed");
  }
}

void game_window_render_frame(const SimulationState *sim,
                              const Camera2D_RTS *camera, int current_tick,
                              bool paused) {
  ClearBackground(RAYWHITE);

  // Render game world layers
  renderer_draw_map(&sim->map, camera);
  renderer_draw_objects(sim->objects, sim->objectCount, camera);
  renderer_draw_units(sim->units, sim->unitCount, camera);

  // Render UI layers
  ui_draw_main_panel(sim, camera);
  ui_draw_top_bar();

  // Render simulation info
  char tick_text[64];
  snprintf(tick_text, sizeof(tick_text), "Tick: %d", current_tick);
  DrawText(tick_text, 20, GetScreenHeight() - 40, 20, DARKGRAY);

  if (paused) {
    DrawText("PAUSED", GetScreenWidth() / 2 - 40, 20, 30, RED);
  }
}
