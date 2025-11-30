#include "math_utils.h"
#include <math.h>

float math_utils_clamp(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

float math_utils_lerp(float start, float end, float amount) {
  return start + (end - start) * amount;
}

float math_utils_vector2_length(Vector2 v) {
  return sqrtf(v.x * v.x + v.y * v.y);
}

Vector2 math_utils_vector2_normalize(Vector2 v) {
  Vector2 result = {0};
  float length = math_utils_vector2_length(v);

  if (length > 0.0f) {
    float inv_length = 1.0f / length;
    result.x = v.x * inv_length;
    result.y = v.y * inv_length;
  }
  return result;
}

bool math_utils_rect_contains_point(Rectangle rect, Vector2 point) {
  return (point.x >= rect.x && point.x <= rect.x + rect.width &&
          point.y >= rect.y && point.y <= rect.y + rect.height);
}
