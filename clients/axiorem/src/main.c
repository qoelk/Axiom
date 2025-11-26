#include "raylib.h"

int main(void) {
  const int screenWidth = 800;
  const int screenHeight = 600;

  InitWindow(screenWidth, screenHeight, "Axiorem - RTS Client");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Hello, Axiorem!", 200, 280, 40, MAROON);
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
