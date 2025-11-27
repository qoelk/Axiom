#include "ui.h"
#include "../utils/math_utils.h"
#include "renderer.h"
#include <math.h>
#include <stdio.h>

// Internal constants
#define UI_PANEL_HEIGHT 150
#define UI_MINIMAP_SIZE 120
#define UI_STATUS_PANEL_WIDTH 200

// Color constants
static const Color UI_PANEL_COLOR = {0, 0, 0, 230};
static const Color UI_BORDER_COLOR = {255, 215, 0, 255};
static const Color UI_STATUS_PANEL_COLOR = {169, 169, 169, 178};
static const Color UI_COMMAND_PANEL_COLOR = {139, 69, 19, 153};

UIConfig ui_get_default_config(void) {
  return (UIConfig){.panel_height = UI_PANEL_HEIGHT,
                    .minimap_size = UI_MINIMAP_SIZE,
                    .status_panel_width = UI_STATUS_PANEL_WIDTH};
}

void ui_draw_minimap(const SimulationState *sim, const Camera2D_RTS *camera,
                     int x, int y, int size) {
  // Mini-map background
  DrawRectangle(x, y, size, size, DARKBLUE);
  DrawRectangleLines(x, y, size, size, UI_BORDER_COLOR);

  // Calculate scaling factors
  float scale_x = (float)size / sim->map.width;
  float scale_y = (float)size / sim->map.height;

  // Draw map tiles
  for (int y_pos = 0; y_pos < sim->map.height; y_pos++) {
    for (int x_pos = 0; x_pos < sim->map.width; x_pos++) {
      TileType tile = sim->map.tiles[y_pos * sim->map.width + x_pos];
      Color color = renderer_get_tile_color(tile);
      // Apply fade manually using alpha
      color.a = (unsigned char)(255 * 0.7f);

      int pixel_x = x + (int)(x_pos * scale_x);
      int pixel_y = y + (int)(y_pos * scale_y);
      int pixel_width = (int)ceil(scale_x);
      int pixel_height = (int)ceil(scale_y);

      DrawRectangle(pixel_x, pixel_y, pixel_width, pixel_height, color);
    }
  }

  // Draw objects on mini-map (as small dots)
  for (int i = 0; i < sim->objectCount; i++) {
    Object obj = sim->objects[i];
    int pixel_x = x + (int)(obj.x * scale_x);
    int pixel_y = y + (int)(obj.y * scale_y);
    DrawCircle(pixel_x, pixel_y, 2, PURPLE);
  }

  // Draw units on mini-map (colored by owner)
  for (int i = 0; i < sim->unitCount; i++) {
    Unit unit = sim->units[i];
    int pixel_x = x + (int)(unit.x * scale_x);
    int pixel_y = y + (int)(unit.y * scale_y);
    Color unit_color = (unit.owner == 1) ? RED : YELLOW;
    DrawCircle(pixel_x, pixel_y, 2, unit_color);
  }

  // Draw viewport rectangle
  Rectangle viewport = camera->viewport;
  int viewport_x = x + (int)(viewport.x * scale_x);
  int viewport_y = y + (int)(viewport.y * scale_y);
  int viewport_width = (int)(viewport.width * scale_x);
  int viewport_height = (int)(viewport.height * scale_y);

  DrawRectangleLines(viewport_x, viewport_y, viewport_width, viewport_height,
                     YELLOW);

  // Mini-map interaction
  Rectangle minimap_rect = {x, y, size, size};
  if (math_utils_rect_contains_point(minimap_rect, GetMousePosition())) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      Vector2 mouse_pos = GetMousePosition();
      float world_x = (mouse_pos.x - x) / scale_x;
      float world_y = (mouse_pos.y - y) / scale_y;
      camera_center_on_world_position(camera, (Vector2){world_x, world_y});
    }
    DrawRectangleLines(x, y, size, size, WHITE);
  }
}

void ui_draw_tick_controls(int x, int y, int width, int height,
                           int current_tick, int max_tick, bool paused) {
  int button_width = 80;
  int button_height = 30;
  int button_spacing = 10;

  // Tick navigation buttons
  DrawRectangle(x + 10, y + 20, button_width, button_height, DARKBLUE);
  DrawText("PREV", x + 25, y + 28, 12, WHITE);

  DrawRectangle(x + 10 + button_width + button_spacing, y + 20, button_width,
                button_height, DARKBLUE);
  DrawText("NEXT", x + 25 + button_width + button_spacing, y + 28, 12, WHITE);

  DrawRectangle(x + 10 + 2 * (button_width + button_spacing), y + 20,
                button_width, button_height, MAROON);
  DrawText(paused ? "PLAY" : "PAUSE",
           x + 22 + 2 * (button_width + button_spacing), y + 28, 12, WHITE);

  // First/Last tick buttons
  DrawRectangle(x + 10, y + 60, button_width, button_height, DARKPURPLE);
  DrawText("FIRST", x + 22, y + 68, 12, WHITE);

  DrawRectangle(x + 10 + button_width + button_spacing, y + 60, button_width,
                button_height, DARKPURPLE);
  DrawText("LAST", x + 30 + button_width + button_spacing, y + 68, 12, WHITE);

  DrawRectangle(x + 10 + 2 * (button_width + button_spacing), y + 60,
                button_width, button_height, DARKGRAY);
  DrawText("RESET", x + 22 + 2 * (button_width + button_spacing), y + 68, 12,
           WHITE);

  // Tick progress bar
  int bar_width = width - 20;
  int bar_height = 15;
  int bar_x = x + 10;
  int bar_y = y + 100;

  // Background
  DrawRectangle(bar_x, bar_y, bar_width, bar_height, DARKGRAY);

  // Progress
  if (max_tick > 0) {
    int progress_width = (int)((float)current_tick / max_tick * bar_width);
    DrawRectangle(bar_x, bar_y, progress_width, bar_height, GREEN);
  }

  // Border
  DrawRectangleLines(bar_x, bar_y, bar_width, bar_height, BLACK);

  // Tick text
  char tick_text[64];
  snprintf(tick_text, sizeof(tick_text), "Tick: %d / %d", current_tick,
           max_tick);
  DrawText(tick_text, bar_x + 5, bar_y - 2, 12, WHITE);
}

void ui_draw_control_buttons(int x, int y, int width, int height, bool paused) {
  int button_width = 80;
  int button_height = 30;
  int button_spacing = 10;

  // Unit commands (simplified for space)
  DrawRectangle(x + 10, y + 20, button_width, button_height, DARKGREEN);
  DrawText("MOVE", x + 25, y + 28, 12, WHITE);

  DrawRectangle(x + 10 + button_width + button_spacing, y + 20, button_width,
                button_height, DARKBLUE);
  DrawText("ATTACK", x + 20 + button_width + button_spacing, y + 28, 12, WHITE);
}

void ui_draw_status_panel(int x, int y, int width, int height,
                          const SimulationState *sim,
                          const Camera2D_RTS *camera, int current_tick,
                          int max_tick, bool paused) {
  DrawRectangle(x, y, width, height, UI_STATUS_PANEL_COLOR);

  DrawText("AXIOM BATTLEFIELD", x + 10, y + 10, 18, GOLD);

  char info_text[256];
  snprintf(info_text, sizeof(info_text), "Tick: %d / %d", current_tick,
           max_tick);
  DrawText(info_text, x + 10, y + 35, 16, paused ? YELLOW : LIME);

  snprintf(info_text, sizeof(info_text), "Units: %d", sim->unitCount);
  DrawText(info_text, x + 10, y + 55, 14, LIME);

  snprintf(info_text, sizeof(info_text), "Objects: %d", sim->objectCount);
  DrawText(info_text, x + 10, y + 75, 14, SKYBLUE);

  snprintf(info_text, sizeof(info_text), "Map: %dx%d", sim->map.width,
           sim->map.height);
  DrawText(info_text, x + 10, y + 95, 12, LIGHTGRAY);

  snprintf(info_text, sizeof(info_text), "Zoom: %.1fx", camera->zoom);
  DrawText(info_text, x + 10, y + 110, 12, YELLOW);

  // Status indicator
  const char *status = paused ? "PAUSED" : "PLAYING";
  Color status_color = paused ? YELLOW : GREEN;
  DrawText(status, x + width - 60, y + 35, 14, status_color);
}

void ui_draw_top_bar(int current_tick, int max_tick, bool paused) {
  int top_bar_height = 30; // Slightly taller for more info
  Color top_bar_color = {0, 0, 0, 204};
  DrawRectangle(0, 0, GetScreenWidth(), top_bar_height, top_bar_color);

  // Simulation info
  char info_text[256];
  snprintf(info_text, sizeof(info_text), "Time: %.1fs | FPS: %d | %s",
           GetTime(), GetFPS(), paused ? "PAUSED" : "PLAYING");
  DrawText(info_text, GetScreenWidth() / 2 - MeasureText(info_text, 14) / 2, 8,
           14, paused ? YELLOW : GREEN);

  // Tick info
  snprintf(info_text, sizeof(info_text), "Tick: %d/%d", current_tick, max_tick);
  DrawText(info_text, 10, 8, 14, LIME);

  // Updated control hints
  DrawText("WASD:Move  Wheel:Zoom  Arrows:Ticks  Space:Play/Pause  "
           "Home/End:First/Last  F:Fullscreen  Q:Quit",
           200, 8, 12, LIGHTGRAY);
}

void ui_draw_main_panel(const SimulationState *sim, const Camera2D_RTS *camera,
                        int current_tick, int max_tick, bool paused) {
  int screen_width = GetScreenWidth();
  int screen_height = GetScreenHeight();
  UIConfig config = ui_get_default_config();

  // Main panel background
  DrawRectangle(0, screen_height - config.panel_height, screen_width,
                config.panel_height, UI_PANEL_COLOR);
  DrawRectangle(0, screen_height - config.panel_height, screen_width, 2,
                UI_BORDER_COLOR);

  // Mini-map
  int minimap_x = screen_width - config.minimap_size - 10;
  int minimap_y = screen_height - config.panel_height + 10;
  ui_draw_minimap(sim, camera, minimap_x, minimap_y, config.minimap_size);

  // Status panel
  ui_draw_status_panel(10, screen_height - config.panel_height + 10,
                       config.status_panel_width, config.panel_height - 20, sim,
                       camera, current_tick, max_tick, paused);

  // Tick controls panel (replaces command buttons)
  int tick_panel_x = config.status_panel_width + 30;
  int tick_panel_width =
      screen_width - config.status_panel_width - config.minimap_size - 50;
  DrawRectangle(tick_panel_x, screen_height - config.panel_height + 10,
                tick_panel_width, config.panel_height - 20,
                UI_COMMAND_PANEL_COLOR);

  ui_draw_tick_controls(tick_panel_x, screen_height - config.panel_height,
                        tick_panel_width, config.panel_height, current_tick,
                        max_tick, paused);

  // Mini-map labels
  DrawText("MINI-MAP", minimap_x + 30, minimap_y + config.minimap_size + 5, 12,
           GOLD);
  DrawText("Click to move", minimap_x + 20,
           minimap_y + config.minimap_size + 20, 10, LIGHTGRAY);
}
