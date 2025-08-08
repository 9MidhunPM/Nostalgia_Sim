#include "raylib.h"

int main() {
    // Window size
    const int screenWidth = 800;
    const int screenHeight = 450;

    // Initialize Raylib
    InitWindow(screenWidth, screenHeight, "Raylib Sample Program");

    // Ball properties
    Vector2 ballPos = { screenWidth / 2.0f, screenHeight / 2.0f };
    float ballSpeed = 5.0f;

    SetTargetFPS(60);

    while (!WindowShouldClose()) { // Main loop
        // Movement controls
        if (IsKeyDown(KEY_RIGHT)) ballPos.x += ballSpeed;
        if (IsKeyDown(KEY_LEFT))  ballPos.x -= ballSpeed;
        if (IsKeyDown(KEY_DOWN))  ballPos.y += ballSpeed;
        if (IsKeyDown(KEY_UP))    ballPos.y -= ballSpeed;

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Use arrow keys to move the ball", 10, 10, 20, DARKGRAY);
        DrawCircleV(ballPos, 30, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
