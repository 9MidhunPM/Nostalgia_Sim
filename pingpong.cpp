#include "raylib.h"
#include <cmath> // Required for fabs()

// Use an enum to manage the game's state
enum GameState {
    PLAYING,
    GAME_OVER
};

int main(void) {
    //==================================================
    // ## Window and Game Initialization
    //==================================================
    const int screenWidth = 840;
    const int screenHeight = 480;
    InitWindow(screenWidth, screenHeight, "Classic Pong - by Gemini");
    SetTargetFPS(60);

    // Player Paddle
    Rectangle player = { 30.0f, screenHeight / 2.0f - 50.0f, 10.0f, 100.0f };
    const float playerSpeed = 500.0f;

    // AI Paddle
    Rectangle ai = { screenWidth - 40.0f, screenHeight / 2.0f - 50.0f, 10.0f, 100.0f };
    const float aiSpeed = 350.0f; // AI is slightly slower to be beatable

    // Ball
    Vector2 ballPosition = { screenWidth / 2.0f, screenHeight / 2.0f };
    const float initialBallSpeed = 350.0f;
    Vector2 ballSpeed = { initialBallSpeed, initialBallSpeed };
    const float ballRadius = 8.0f;

    // Game State Variables
    int playerScore = 0;
    int aiScore = 0;
    const int winningScore = 3;
    GameState currentState = PLAYING;
    const char* winnerText = "";

    //==================================================
    // ## Main Game Loop
    //==================================================
    while (!WindowShouldClose()) {
        //----------------------------------------------------------------------------------
        // ## Update Logic
        //----------------------------------------------------------------------------------
        switch (currentState) {
            case PLAYING: {
                // Player paddle movement (W for up, S for down)
                if (IsKeyDown(KEY_W) && player.y > 0) {
                    player.y -= playerSpeed * GetFrameTime();
                }
                if (IsKeyDown(KEY_S) && player.y < screenHeight - player.height) {
                    player.y += playerSpeed * GetFrameTime();
                }

                // AI paddle movement (simple chase logic)
                if (ai.y + ai.height / 2 < ballPosition.y) ai.y += aiSpeed * GetFrameTime();
                if (ai.y + ai.height / 2 > ballPosition.y) ai.y -= aiSpeed * GetFrameTime();
                if (ai.y < 0) ai.y = 0; // Keep AI in bounds
                if (ai.y > screenHeight - ai.height) ai.y = screenHeight - ai.height;

                // Move the ball
                ballPosition.x += ballSpeed.x * GetFrameTime();
                ballPosition.y += ballSpeed.y * GetFrameTime();

                // Ball collision: Top and bottom walls
                if (ballPosition.y + ballRadius >= screenHeight || ballPosition.y - ballRadius <= 0) {
                    ballSpeed.y *= -1;
                }

                // Ball collision: Paddles
                if (CheckCollisionCircleRec(ballPosition, ballRadius, player) && ballSpeed.x < 0) {
                    ballSpeed.x *= -1.1f; // Reverse direction and increase speed by 10%
                    // Adjust y-speed based on where the ball hits the paddle for better control
                    ballSpeed.y = (ballPosition.y - (player.y + player.height / 2)) / (player.height / 2) * std::fabs(ballSpeed.x);
                }
                if (CheckCollisionCircleRec(ballPosition, ballRadius, ai) && ballSpeed.x > 0) {
                    ballSpeed.x *= -1.1f; // Reverse direction and increase speed by 10%
                    ballSpeed.y = (ballPosition.y - (ai.y + ai.height / 2)) / (ai.height / 2) * std::fabs(ballSpeed.x);
                }

                // Scoring Logic
                bool pointScored = false;
                if (ballPosition.x - ballRadius > screenWidth) { // Player scores
                    playerScore++;
                    ballSpeed.x = -initialBallSpeed; // Serve to the AI
                    pointScored = true;
                }
                if (ballPosition.x + ballRadius < 0) { // AI scores
                    aiScore++;
                    ballSpeed.x = initialBallSpeed; // Serve to the player
                    pointScored = true;
                }

                // If a point was scored, reset ball position and speed
                if (pointScored) {
                    ballPosition = { screenWidth / 2.0f, screenHeight / 2.0f };
                    // Reset vertical speed to a random-ish starting value
                    ballSpeed.y = initialBallSpeed * (GetRandomValue(0, 1) == 0 ? -1 : 1);
                }

                // Check for a winner
                if (playerScore >= winningScore) {
                    winnerText = "Player Wins!";
                    currentState = GAME_OVER;
                }
                if (aiScore >= winningScore) {
                    winnerText = "AI Wins!";
                    currentState = GAME_OVER;
                }
            } break;

            case GAME_OVER: {
                // Press Enter to restart the game
                if (IsKeyPressed(KEY_ENTER)) {
                    playerScore = 0;
                    aiScore = 0;
                    currentState = PLAYING;
                }
            } break;
        }

        //----------------------------------------------------------------------------------
        // ## Drawing Logic
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw the center dashed line
        for (int i = 0; i < screenHeight; i += 25) {
            DrawRectangle(screenWidth / 2 - 2, i, 4, 15, GREEN);
        }

        // Draw paddles and ball
        DrawRectangleRec(player, GREEN);
        DrawRectangleRec(ai, GREEN);
        DrawCircleV(ballPosition, ballRadius, GREEN);

        // Draw the scores
        DrawText(TextFormat("%i", playerScore), screenWidth / 4 - 20, 20, 80, GREEN);
        DrawText(TextFormat("%i", aiScore), 3 * screenWidth / 4 - 20, 20, 80, GREEN);
        
        // Draw the Game Over screen
        if (currentState == GAME_OVER) {
            DrawText(winnerText, GetScreenWidth() / 2 - MeasureText(winnerText, 40) / 2, GetScreenHeight() / 2 - 40, 40, GREEN);
            const char* restartMsg = "Press [ENTER] to Play Again";
            DrawText(restartMsg, GetScreenWidth() / 2 - MeasureText(restartMsg, 20) / 2, GetScreenHeight() / 2 + 20, 20, GREEN);
        }

        EndDrawing();
    }

    //==================================================
    // ## De-Initialization
    //==================================================
    CloseWindow();

    return 0;
}