#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include "raylib.h"
#include <stdbool.h>

// Constants
#define TILE_SIZE_PIXELS 128.0f
#define DEFAULT_CAMERA_SPEED 100.0f

// Math utilities
float math_utils_clamp(float value, float min, float max);
float math_utils_lerp(float start, float end, float amount);
Vector2 math_utils_vector2_normalize(Vector2 v);
float math_utils_vector2_length(Vector2 v);
bool math_utils_rect_contains_point(Rectangle rect, Vector2 point);

#endif
