#include "game_window.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>

static GameWindowConfig default_config = {.screen_width = 800,
                                          .screen_height = 600,
                                          .window_title =
                                              "Axiom - AI Battlefield",
                                          .target_fps = 60,
                                          .fullscreen = true};

void game_window_toggle_fullscreen(void) {
  if (IsWindowFullscreen()) {
    ToggleFullscreen();
    SetWindowSize(default_config.screen_width, default_config.screen_height);
    int monitor = GetCurrentMonitor();
    int screen_width = GetMonitorWidth(monitor);
    int screen_height = GetMonitorHeight(monitor);
    SetWindowPosition((screen_width - default_config.screen_width) / 2,
                      (screen_height - default_config.screen_height) / 2);
  } else {
    int monitor = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
    ToggleFullscreen();
  }
}

void game_window_load_tick(GameState *game_state, int tick) {
  if (tick < 0)
    tick = 0;
  if (tick > game_state->max_tick)
    tick = game_state->max_tick;

  SimulationState *new_sim = LoadStateAtTick(game_state->filename, tick);
  if (new_sim) {
    FreeState(game_state->sim);
    game_state->sim = new_sim;
    game_state->current_tick = tick;
    TraceLog(LOG_INFO, "GameWindow: Loaded tick %d", tick);
  }
}

int game_window_run(SimulationState *sim) {
  if (sim == NULL) {
    TraceLog(LOG_ERROR, "GameWindow: NULL simulation state provided");
    return 1;
  }

  // Initialize game state
  GameState game_state = {
      .sim = sim,
      .current_tick = 0,
      .max_tick = GetMaxTickFromFile("../assets/test.sim.json"),
      .paused = true, // Start paused to allow tick navigation
      .filename = "../assets/test.sim.json"};

  // Set initial window state
  if (default_config.fullscreen) {
    InitWindow(0, 0, default_config.window_title);
    int monitor = GetCurrentMonitor();
    int screen_width = GetMonitorWidth(monitor);
    int screen_height = GetMonitorHeight(monitor);
    SetWindowSize(screen_width, screen_height);
    ToggleFullscreen();
  } else {
    InitWindow(default_config.screen_width, default_config.screen_height,
               default_config.window_title);
    int monitor = GetCurrentMonitor();
    int screen_width = GetMonitorWidth(monitor);
    int screen_height = GetMonitorHeight(monitor);
    SetWindowPosition((screen_width - default_config.screen_width) / 2,
                      (screen_height - default_config.screen_height) / 2);
  }

  SetTargetFPS(default_config.target_fps);
  renderer_init_tile_atlas("../assets/tiles.png", 16, 16, 1);

  if (!IsWindowReady()) {
    TraceLog(LOG_ERROR, "GameWindow: Failed to initialize window");
    return 1;
  }

  Camera2D_RTS camera;
  CameraConfig cam_config = {.screen_width = GetScreenWidth(),
                             .screen_height = GetScreenHeight(),
                             .camera_move_speed = DEFAULT_CAMERA_SPEED,
                             .camera_zoom_speed = 0.1f};

  camera_init(&camera, &cam_config, &game_state.sim->map);

  TraceLog(LOG_INFO, "GameWindow: Starting main game loop");
  TraceLog(LOG_INFO, "GameWindow: Total ticks available: %d",
           game_state.max_tick);
  TraceLog(LOG_INFO, "GameWindow: Controls - WASD: Move, Mouse Wheel: Zoom, R: "
                     "Reset, P: Pause, F: Fullscreen, Q: Quit");
  TraceLog(LOG_INFO, "GameWindow: Tick Controls - Left/Right: Navigate ticks, "
                     "Space: Play/Pause, Home/End: First/Last tick");

  while (!WindowShouldClose()) {
    camera_update(&camera, &game_state.sim->map);
    game_window_handle_input(&game_state, &camera);

    BeginDrawing();
    game_window_render_frame(&game_state, &camera);
    EndDrawing();
  }

  CloseWindow();
  renderer_cleanup_tile_atlas();

  TraceLog(LOG_INFO, "GameWindow: Shutdown complete");
  return 0;
}

void game_window_handle_input(GameState *game_state, Camera2D_RTS *camera) {
  // Space: Toggle play/pause
  if (IsKeyPressed(KEY_SPACE)) {
    game_state->paused = !game_state->paused;
    TraceLog(LOG_INFO, "GameWindow: Simulation %s",
             game_state->paused ? "paused" : "playing");
  }

  // Left Arrow: Previous tick
  if (IsKeyPressed(KEY_LEFT)) {
    game_window_load_tick(game_state, game_state->current_tick - 1);
  }

  // Right Arrow: Next tick
  if (IsKeyPressed(KEY_RIGHT)) {
    game_window_load_tick(game_state, game_state->current_tick + 1);
  }

  // Home: First tick
  if (IsKeyPressed(KEY_HOME)) {
    game_window_load_tick(game_state, 0);
  }

  // End: Last tick
  if (IsKeyPressed(KEY_END)) {
    game_window_load_tick(game_state, game_state->max_tick);
  }

  // R: Reset to tick 0
  if (IsKeyPressed(KEY_R)) {
    game_window_load_tick(game_state, 0);
    TraceLog(LOG_INFO, "GameWindow: Reset to tick 0");
  }

  // F: Toggle fullscreen
  if (IsKeyPressed(KEY_F)) {
    game_window_toggle_fullscreen();
    TraceLog(LOG_INFO, "GameWindow: Toggled fullscreen mode");
    CameraConfig config = {GetScreenWidth(), GetScreenHeight()};
    camera_init(camera, &config, &game_state->sim->map);
  }

  // Q: Quit game
  if (IsKeyPressed(KEY_Q)) {
    TraceLog(LOG_INFO, "GameWindow: Quit requested via Q key");
  }

  // Auto-advance if not paused
  if (!game_state->paused && game_state->current_tick < game_state->max_tick) {
    game_window_load_tick(game_state, game_state->current_tick + 1);
  }
}

void game_window_render_frame(const GameState *game_state,
                              const Camera2D_RTS *camera) {
  ClearBackground(RAYWHITE);

  // Render game world layer

  renderer_draw_map_textured(&game_state->sim->map, camera);
  renderer_draw_objects(game_state->sim->objects, game_state->sim->objectCount,
                        camera);
  renderer_draw_units(game_state->sim->units, game_state->sim->unitCount,
                      camera);

  // Render UI layers
  ui_draw_main_panel(game_state->sim, camera, game_state->current_tick,
                     game_state->max_tick, game_state->paused);
  ui_draw_top_bar(game_state->current_tick, game_state->max_tick,
                  game_state->paused);

  // Render quit hint in corner
  DrawText("Press Q to Quit", GetScreenWidth() - 120, 10, 14, LIGHTGRAY);
}
