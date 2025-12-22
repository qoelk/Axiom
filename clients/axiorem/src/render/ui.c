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
      RawTileKey tile = sim->map.tiles[y_pos * sim->map.width + x_pos].raw_key;
      Color color = UI_BORDER_COLOR;
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

  // Draw units on mini-map (colored by owner, proportional rectangles)
  for (int i = 0; i < sim->unitCount; i++) {
    Unit unit = sim->units[i];

    // Calculate unit position and size in minimap pixels
    int unit_x = x + (int)(unit.x * scale_x);
    int unit_y = y + (int)(unit.y * scale_y);

    // Use unit's actual size scaled down for the minimap
    int unit_width = max(1, (int)(unit.size * scale_x));
    int unit_height = max(1, (int)(unit.size * scale_y));

    // Adjust position so unit is centered at its actual position
    unit_x -= unit_width / 2;
    unit_y -= unit_height / 2;

    Color unit_color = (unit.owner == 1) ? RED : YELLOW;

    // Draw filled rectangle for the unit
    DrawRectangle(unit_x, unit_y, unit_width, unit_height, unit_color);

    // Optional: Add a border to make units more visible
    DrawRectangleLines(unit_x, unit_y, unit_width, unit_height,
                       (Color){0, 0, 0, 255});
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
                           int current_tick, int max_tick, bool paused) {}

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
}

void ui_draw_top_bar(int current_tick, int max_tick, bool paused) {
  UIConfig config = ui_get_default_config();

  Color top_bar_color = {0, 0, 0, 204};
  DrawRectangle(0, 0, GetScreenWidth(), config.top_bar_height, top_bar_color);
  DrawRectangle(0, config.top_bar_height, GetScreenWidth(), 1, UI_BORDER_COLOR);
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

  // Mini-map on left edge
  int minimap_x = 10;
  int minimap_y = screen_height - config.panel_height + 10;

  // Status panel (moved right to make space for minimap)
  int status_panel_x = minimap_x + config.minimap_size + 10;
  ui_draw_status_panel(status_panel_x, screen_height - config.panel_height + 10,
                       config.status_panel_width, panel_inner_height, sim,
                       camera, current_tick, max_tick, paused);

  // Tick controls panel
  int tick_panel_x = status_panel_x + config.status_panel_width + 10;
  int tick_panel_width = screen_width - tick_panel_x - 10;

  if (tick_panel_width > 200) { // Only draw if there's reasonable space
    DrawRectangle(tick_panel_x, screen_height - config.panel_height + 10,
                  tick_panel_width, panel_inner_height, UI_COMMAND_PANEL_COLOR);
    DrawRectangleLines(tick_panel_x, screen_height - config.panel_height + 10,
                       tick_panel_width, panel_inner_height, UI_BORDER_COLOR);

    ui_draw_tick_controls(
        tick_panel_x, screen_height - config.panel_height + 10,
        tick_panel_width, panel_inner_height, current_tick, max_tick, paused);
  }

  // Draw minimap (always on left edge)
  ui_draw_minimap(sim, camera, minimap_x, minimap_y, config.minimap_size);
}
