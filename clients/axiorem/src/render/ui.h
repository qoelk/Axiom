#ifndef UI_H
#define UI_H

#include "../client/sim_loader.h"
#include "camera.h"
#include "raylib.h"

/**
 * @brief User Interface rendering system
 *
 * Handles all UI elements including minimap, control panels,
 * status displays, and interactive elements.
 */

// UI layout constants
typedef struct {
  int panel_height;
  int minimap_size;
  int status_panel_width;
} UIConfig;

// UI component rendering
void ui_draw_minimap(const SimulationState *sim, const Camera2D_RTS *camera,
                     int x, int y, int size);
void ui_draw_main_panel(const SimulationState *sim, const Camera2D_RTS *camera);
void ui_draw_top_bar(void);
void ui_draw_control_buttons(int x, int y, int width, int height, bool paused);
void ui_draw_status_panel(int x, int y, int width, int height,
                          const SimulationState *sim,
                          const Camera2D_RTS *camera);

// Get default UI configuration
UIConfig ui_get_default_config(void);

#endif
