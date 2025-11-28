#include "ui.h"
#include "../utils/math_utils.h"
#include "renderer.h"
#include <math.h>
#include <stdio.h>

// Internal constants - now relative to screen size
#define UI_PANEL_HEIGHT_RATIO 0.15f       // 15% of screen height
#define UI_MINIMAP_SIZE_RATIO 0.12f       // 12% of screen width
#define UI_STATUS_PANEL_WIDTH_RATIO 0.18f // 18% of screen width
#define UI_TOP_BAR_HEIGHT_RATIO 0.03f     // 3% of screen height

// Minimum sizes to ensure readability
#define UI_MIN_PANEL_HEIGHT 120
#define UI_MIN_MINIMAP_SIZE 100
#define UI_MIN_STATUS_WIDTH 180
#define UI_MIN_TOP_BAR_HEIGHT 25

// Color constants
static const Color UI_PANEL_COLOR = {0, 0, 0, 230};
static const Color UI_BORDER_COLOR = {255, 215, 0, 255};
static const Color UI_STATUS_PANEL_COLOR = {169, 169, 169, 178};
static const Color UI_COMMAND_PANEL_COLOR = {139, 69, 19, 153};

// Helper macros for min/max if not available
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

UIConfig ui_get_default_config(void) {
  int screen_width = GetScreenWidth();
  int screen_height = GetScreenHeight();

  return (UIConfig){
      .panel_height = max((int)(screen_height * UI_PANEL_HEIGHT_RATIO),
                          UI_MIN_PANEL_HEIGHT),
      .minimap_size =
          max((int)(screen_width * UI_MINIMAP_SIZE_RATIO), UI_MIN_MINIMAP_SIZE),
      .status_panel_width =
          max((int)(screen_width * UI_STATUS_PANEL_WIDTH_RATIO),
              UI_MIN_STATUS_WIDTH),
      .top_bar_height = max((int)(screen_height * UI_TOP_BAR_HEIGHT_RATIO),
                            UI_MIN_TOP_BAR_HEIGHT)};
}

void ui_draw_minimap(const SimulationState *sim, const Camera2D_RTS *camera,
                     int x, int y, int size) {
  // Mini-map background with border
  DrawRectangle(x, y, size, size, (Color){0, 0, 50, 255});
  DrawRectangleLines(x, y, size, size, UI_BORDER_COLOR);
  DrawRectangleLines(x - 1, y - 1, size + 2, size + 2, (Color){0, 0, 0, 255});

  // Calculate scaling factors
  float scale_x = (float)size / sim->map.width;
  float scale_y = (float)size / sim->map.height;

  // Draw map tiles with better scaling for small maps
  for (int y_pos = 0; y_pos < sim->map.height; y_pos++) {
    for (int x_pos = 0; x_pos < sim->map.width; x_pos++) {
      TileType tile = sim->map.tiles[y_pos * sim->map.width + x_pos];
      Color color = renderer_get_tile_color(tile);
      color.a = (unsigned char)(255 * 0.7f);

      int pixel_x = x + (int)(x_pos * scale_x);
      int pixel_y = y + (int)(y_pos * scale_y);
      int pixel_width = max(1, (int)ceil(scale_x));
      int pixel_height = max(1, (int)ceil(scale_y));

      DrawRectangle(pixel_x, pixel_y, pixel_width, pixel_height, color);
    }
  }

  // Draw objects on mini-map (as small dots)
  for (int i = 0; i < sim->objectCount; i++) {
    Object obj = sim->objects[i];
    int pixel_x = x + (int)(obj.x * scale_x);
    int pixel_y = y + (int)(obj.y * scale_y);
    DrawCircle(pixel_x, pixel_y, max(1, (int)(2 * scale_x)), PURPLE);
  }

  // Draw units on mini-map (colored by owner)
  for (int i = 0; i < sim->unitCount; i++) {
    Unit unit = sim->units[i];
    int pixel_x = x + (int)(unit.x * scale_x);
    int pixel_y = y + (int)(unit.y * scale_y);
    Color unit_color = (unit.owner == 1) ? RED : YELLOW;
    DrawCircle(pixel_x, pixel_y, max(1, (int)(2 * scale_x)), unit_color);
  }

  // Draw viewport rectangle
  Rectangle viewport = camera->viewport;
  int viewport_x = x + (int)(viewport.x * scale_x);
  int viewport_y = y + (int)(viewport.y * scale_y);
  int viewport_width = max(1, (int)(viewport.width * scale_x));
  int viewport_height = max(1, (int)(viewport.height * scale_y));

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
  // Calculate button sizes based on available space
  int button_width = min(80, width / 6);
  int button_height = min(30, height / 4);
  int button_spacing = min(10, width / 50);

  // Calculate font size based on button height
  int font_size = max(10, button_height / 3);

  // Tick navigation buttons
  Rectangle prev_btn = {x + 10, y + 20, button_width, button_height};
  DrawRectangleRec(prev_btn, DARKBLUE);
  DrawText("PREV", prev_btn.x + 5, prev_btn.y + (button_height - font_size) / 2,
           font_size, WHITE);

  Rectangle next_btn = {prev_btn.x + button_width + button_spacing, y + 20,
                        button_width, button_height};
  DrawRectangleRec(next_btn, DARKBLUE);
  DrawText("NEXT", next_btn.x + 5, next_btn.y + (button_height - font_size) / 2,
           font_size, WHITE);

  Rectangle play_btn = {next_btn.x + button_width + button_spacing, y + 20,
                        button_width, button_height};
  DrawRectangleRec(play_btn, paused ? GREEN : MAROON);
  DrawText(paused ? "PLAY" : "PAUSE", play_btn.x + 5,
           play_btn.y + (button_height - font_size) / 2, font_size, WHITE);

  // First/Last tick buttons
  Rectangle first_btn = {x + 10, y + 20 + button_height + 10, button_width,
                         button_height};
  DrawRectangleRec(first_btn, DARKPURPLE);
  DrawText("FIRST", first_btn.x + 5,
           first_btn.y + (button_height - font_size) / 2, font_size, WHITE);

  Rectangle last_btn = {first_btn.x + button_width + button_spacing,
                        first_btn.y, button_width, button_height};
  DrawRectangleRec(last_btn, DARKPURPLE);
  DrawText("LAST", last_btn.x + 5, last_btn.y + (button_height - font_size) / 2,
           font_size, WHITE);

  Rectangle reset_btn = {last_btn.x + button_width + button_spacing,
                         first_btn.y, button_width, button_height};
  DrawRectangleRec(reset_btn, DARKGRAY);
  DrawText("RESET", reset_btn.x + 5,
           reset_btn.y + (button_height - font_size) / 2, font_size, WHITE);

  // Tick progress bar
  int bar_width = width - 20;
  int bar_height = max(12, height / 12);
  int bar_x = x + 10;
  int bar_y = y + height - bar_height - 20;

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
  int text_width = MeasureText(tick_text, font_size);
  DrawText(tick_text, bar_x + (bar_width - text_width) / 2,
           bar_y - font_size - 2, font_size, WHITE);
}

void ui_draw_status_panel(int x, int y, int width, int height,
                          const SimulationState *sim,
                          const Camera2D_RTS *camera, int current_tick,
                          int max_tick, bool paused) {
  DrawRectangle(x, y, width, height, UI_STATUS_PANEL_COLOR);
  DrawRectangleLines(x, y, width, height, UI_BORDER_COLOR);

  // Calculate font sizes based on panel height
  int title_size = max(14, height / 10);
  int header_size = max(12, height / 12);
  int text_size = max(10, height / 14);

  DrawText("AXIOM BATTLEFIELD", x + 10, y + 10, title_size, GOLD);

  char info_text[256];
  int text_y = y + 15 + title_size;

  snprintf(info_text, sizeof(info_text), "Tick: %d / %d", current_tick,
           max_tick);
  DrawText(info_text, x + 10, text_y, header_size, paused ? YELLOW : LIME);
  text_y += header_size + 5;

  snprintf(info_text, sizeof(info_text), "Units: %d", sim->unitCount);
  DrawText(info_text, x + 10, text_y, text_size, LIME);
  text_y += text_size + 5;

  snprintf(info_text, sizeof(info_text), "Objects: %d", sim->objectCount);
  DrawText(info_text, x + 10, text_y, text_size, SKYBLUE);
  text_y += text_size + 5;

  snprintf(info_text, sizeof(info_text), "Map: %dx%d", sim->map.width,
           sim->map.height);
  DrawText(info_text, x + 10, text_y, text_size, LIGHTGRAY);
  text_y += text_size + 5;

  snprintf(info_text, sizeof(info_text), "Zoom: %.1fx", camera->zoom);
  DrawText(info_text, x + 10, text_y, text_size, YELLOW);

  // Status indicator
  const char *status = paused ? "PAUSED" : "PLAYING";
  Color status_color = paused ? YELLOW : GREEN;
  int status_width = MeasureText(status, header_size);
  DrawText(status, x + width - status_width - 10, y + 15, header_size,
           status_color);
}

void ui_draw_top_bar(int current_tick, int max_tick, bool paused) {
  UIConfig config = ui_get_default_config();

  Color top_bar_color = {0, 0, 0, 204};
  DrawRectangle(0, 0, GetScreenWidth(), config.top_bar_height, top_bar_color);
  DrawRectangle(0, config.top_bar_height, GetScreenWidth(), 1, UI_BORDER_COLOR);

  // Calculate font sizes
  int main_font_size = max(12, config.top_bar_height / 2);
  int hint_font_size = max(10, config.top_bar_height / 3);

  // Simulation info
  char info_text[256];
  snprintf(info_text, sizeof(info_text), "Time: %.1fs | FPS: %d | %s",
           GetTime(), GetFPS(), paused ? "PAUSED" : "PLAYING");
  DrawText(info_text,
           GetScreenWidth() / 2 - MeasureText(info_text, main_font_size) / 2,
           (config.top_bar_height - main_font_size) / 2, main_font_size,
           paused ? YELLOW : GREEN);

  // Tick info
  snprintf(info_text, sizeof(info_text), "Tick: %d/%d", current_tick, max_tick);
  DrawText(info_text, 10, (config.top_bar_height - main_font_size) / 2,
           main_font_size, LIME);

  // Control hints (only show if there's enough space)
  int hints_width =
      MeasureText("WASD:Move Wheel:Zoom Space:Play/Pause", hint_font_size);
  if (hints_width < GetScreenWidth() - 250) {
    DrawText("WASD:Move Wheel:Zoom Space:Play/Pause",
             GetScreenWidth() - hints_width - 10,
             (config.top_bar_height - hint_font_size) / 2, hint_font_size,
             LIGHTGRAY);
  } else {
    // Simplified hints for smaller screens
    DrawText("WASD:Move Space:Play/Pause",
             GetScreenWidth() -
                 MeasureText("WASD:Move Space:Play/Pause", hint_font_size) - 10,
             (config.top_bar_height - hint_font_size) / 2, hint_font_size,
             LIGHTGRAY);
  }
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

  // Calculate layout with proper spacing
  int panel_inner_height = config.panel_height - 20; // 10px top/bottom margin
  int minimap_x = screen_width - config.minimap_size - 10;
  int minimap_y = screen_height - config.panel_height + 10;

  // Status panel
  ui_draw_status_panel(10, screen_height - config.panel_height + 10,
                       config.status_panel_width, panel_inner_height, sim,
                       camera, current_tick, max_tick, paused);

  // Tick controls panel
  int tick_panel_x = config.status_panel_width + 20;
  int tick_panel_width = minimap_x - tick_panel_x - 10;

  if (tick_panel_width > 200) { // Only draw if there's reasonable space
    DrawRectangle(tick_panel_x, screen_height - config.panel_height + 10,
                  tick_panel_width, panel_inner_height, UI_COMMAND_PANEL_COLOR);
    DrawRectangleLines(tick_panel_x, screen_height - config.panel_height + 10,
                       tick_panel_width, panel_inner_height, UI_BORDER_COLOR);

    ui_draw_tick_controls(
        tick_panel_x, screen_height - config.panel_height + 10,
        tick_panel_width, panel_inner_height, current_tick, max_tick, paused);
  }

  // Mini-map (always draw, but might be smaller if space is tight)
  int actual_minimap_size = config.minimap_size;
  if (minimap_x < tick_panel_x + 150) { // If overlapping, reduce minimap
    actual_minimap_size =
        max(UI_MIN_MINIMAP_SIZE, screen_width - minimap_x - 20);
  }

  ui_draw_minimap(sim, camera, minimap_x, minimap_y, actual_minimap_size);

  // Mini-map labels
  int label_font_size = max(10, actual_minimap_size / 12);
  DrawText(
      "MINI-MAP",
      minimap_x +
          (actual_minimap_size - MeasureText("MINI-MAP", label_font_size)) / 2,
      minimap_y + actual_minimap_size + 5, label_font_size, GOLD);

  if (actual_minimap_size > UI_MIN_MINIMAP_SIZE + 20) {
    DrawText("Click to move",
             minimap_x + (actual_minimap_size -
                          MeasureText("Click to move", label_font_size - 2)) /
                             2,
             minimap_y + actual_minimap_size + 5 + label_font_size + 2,
             label_font_size - 2, LIGHTGRAY);
  }
}
