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
 *
 * Features responsive design that adapts to different screen sizes
 * and resolutions.
 */

// UI layout configuration with responsive sizing
typedef struct {
  int panel_height;       // Main control panel height
  int minimap_size;       // Mini-map display size
  int status_panel_width; // Status panel width
  int top_bar_height;     // Top information bar height
} UIConfig;

// UI component rendering functions

/**
 * @brief Draws the mini-map representation of the simulation
 *
 * @param sim Simulation state to visualize
 * @param camera Current camera view for viewport rectangle
 * @param x X position of mini-map
 * @param y Y position of mini-map
 * @param size Size of the mini-map (width and height)
 */
void ui_draw_minimap(const SimulationState *sim, const Camera2D_RTS *camera,
                     int x, int y, int size);

/**
 * @brief Draws the main control panel at the bottom of the screen
 *
 * @param sim Current simulation state
 * @param camera Active camera
 * @param current_tick Current simulation tick
 * @param max_tick Maximum available tick
 * @param paused Whether simulation is paused
 */
void ui_draw_main_panel(const SimulationState *sim, const Camera2D_RTS *camera,
                        int current_tick, int max_tick, bool paused);

/**
 * @brief Draws the top information bar with simulation stats and controls
 *
 * @param current_tick Current simulation tick
 * @param max_tick Maximum available tick
 * @param paused Whether simulation is paused
 */
void ui_draw_top_bar(int current_tick, int max_tick, bool paused);

/**
 * @brief Draws control buttons for unit commands (legacy - may be deprecated)
 *
 * @param x X position
 * @param y Y position
 * @param width Width of control area
 * @param height Height of control area
 * @param paused Whether simulation is paused
 */
void ui_draw_control_buttons(int x, int y, int width, int height, bool paused);

/**
 * @brief Draws the status panel with simulation information
 *
 * @param x X position
 * @param y Y position
 * @param width Width of panel
 * @param height Height of panel
 * @param sim Simulation state
 * @param camera Active camera
 * @param current_tick Current simulation tick
 * @param max_tick Maximum available tick
 * @param paused Whether simulation is paused
 */
void ui_draw_status_panel(int x, int y, int width, int height,
                          const SimulationState *sim,
                          const Camera2D_RTS *camera, int current_tick,
                          int max_tick, bool paused);

/**
 * @brief Draws tick control buttons and progress bar
 *
 * @param x X position
 * @param y Y position
 * @param width Width of control area
 * @param height Height of control area
 * @param current_tick Current simulation tick
 * @param max_tick Maximum available tick
 * @param paused Whether simulation is paused
 */
void ui_draw_tick_controls(int x, int y, int width, int height,
                           int current_tick, int max_tick, bool paused);

/**
 * @brief Gets the default UI configuration based on current screen size
 *
 * @return UIConfig Configuration with responsive sizing based on screen
 * dimensions
 */
UIConfig ui_get_default_config(void);

#endif
