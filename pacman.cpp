#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <list>

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------

const float TILE_SIZE = 25.0f;

enum GhostType { BLINKY, PINKY, INKY, CLYDE };
enum GhostState { CHASING, FRIGHTENED };

struct Node {
    bool isObstacle = false; bool isVisited = false;
    float globalGoal, localGoal;
    int x, y;
    std::vector<Node*> neighbors;
    Node* parent = nullptr;
};

struct Player {
    Vector2 position, startPosition, direction = {0, 0}, desiredDirection = {0, 0};
    float speed, radius;
    Color color;
    // Animation
    int frameCounter = 0;
    int currentFrame = 0;
    Rectangle frameRec = { 0.0f, 0.0f, TILE_SIZE, TILE_SIZE };
};

struct Ghost {
    Vector2 position, startPosition;
    std::vector<Node*> path;
    GhostType type;
    GhostState state = CHASING;
    float stateTimer = 0.0f;
    float speed, radius;
    Color color;
    // Animation
    int frameCounter = 0;
    int currentFrame = 0;
};

// ... (Other struct definitions are the same) ...
struct Pellet {
    Vector2 position; float radius; bool active;
    bool isPowerPellet = false; Color color;
};

struct Wall {
    Rectangle rect; Color color;
};

//------------------------------------------------------------------------------------
// Global Variables
//------------------------------------------------------------------------------------
std::vector<Wall> walls;
std::vector<Pellet> pellets;
std::vector<Ghost> ghosts;
Player player;
int playerLives = 3;
int score = 0;
bool gameOver = false;
int activePellets = 0;
Node* nodes = nullptr;
int mapWidth = 0, mapHeight = 0;

// NEW: Texture variables for our sprites
Texture2D pacmanTexture;
Texture2D ghostsTexture;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void LoadMap(const char* fileName);
void ResetRound();

Vector2 WorldToTile(Vector2 worldPos) {
    return { (float)floor(worldPos.x / TILE_SIZE), (float)floor(worldPos.y / TILE_SIZE) };
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    LoadMap("level.txt");
    const int screenWidth = mapWidth * TILE_SIZE;
    const int screenHeight = mapHeight * TILE_SIZE;
    InitWindow(screenWidth, screenHeight, "Pac-Man Sprites! by Gemini");

    // NEW: Load sprite textures
    pacmanTexture = LoadTexture("pacman_sprites.png");
    ghostsTexture = LoadTexture("ghost_sprites.png");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // Update
        //----------------------------------------------------------------------------------
        if (!gameOver) {
            // -- Player Animation Logic --
            player.frameCounter++;
            if (player.frameCounter >= 8) { // Animate every 8 frames
                player.frameCounter = 0;
                player.currentFrame++;
                if (player.currentFrame > 1) player.currentFrame = 0;
            }
            // A closed-mouth frame when not moving
            if (Vector2Length(player.direction) == 0) player.currentFrame = 0;
            
            // ... (Player movement logic is the same) ...
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.desiredDirection = { 1.0f, 0.0f };
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.desiredDirection = { -1.0f, 0.0f };
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) player.desiredDirection = { 0.0f, -1.0f };
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) player.desiredDirection = { 0.0f, 1.0f };

            Vector2 playerTile = WorldToTile(player.position);
            Vector2 playerTileCenter = { playerTile.x * TILE_SIZE + TILE_SIZE / 2, playerTile.y * TILE_SIZE + TILE_SIZE / 2 };
            if (Vector2Distance(player.position, playerTileCenter) < player.speed) {
                player.position = playerTileCenter;
                if (player.desiredDirection.x != 0 || player.desiredDirection.y != 0) {
                    int nextTileX = playerTile.x + player.desiredDirection.x;
                    int nextTileY = playerTile.y + player.desiredDirection.y;
                    if (!nodes[nextTileY * mapWidth + nextTileX].isObstacle) player.direction = player.desiredDirection;
                }
                int currentNextTileX = playerTile.x + player.direction.x;
                int currentNextTileY = playerTile.y + player.direction.y;
                if (nodes[currentNextTileY * mapWidth + currentNextTileX].isObstacle) player.direction = { 0, 0 };
            }
            player.position.x += player.direction.x * player.speed;
            player.position.y += player.direction.y * player.speed;
            
            // -- Ghost Logic --
            for (auto& ghost : ghosts) {
                // Ghost animation
                ghost.frameCounter++;
                if (ghost.frameCounter >= 8) {
                    ghost.frameCounter = 0;
                    ghost.currentFrame++;
                    if (ghost.currentFrame > 1) ghost.currentFrame = 0;
                }
                // (Ghost AI and state logic is the same as before) ...
            }

            // ... (Collision logic is the same as before) ...
        } else {
            if (IsKeyDown(KEY_ENTER)) {
                delete[] nodes;
                nodes = nullptr;
                LoadMap("level.txt");
            }
        }
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);

        for (const auto& wall : walls) DrawRectangleRec(wall.rect, wall.color);
        for (const auto& pellet : pellets) if (pellet.active) DrawCircle(pellet.position.x, pellet.position.y, pellet.radius, pellet.color);
        
        // --- Draw Ghosts using Sprites ---
        for (auto& ghost : ghosts) {
            Rectangle sourceRec = {0, 0, TILE_SIZE, TILE_SIZE};
            // Select frame based on state and animation cycle
            if (ghost.state == FRIGHTENED) {
                 sourceRec.x = (4 + (fmod(ghost.stateTimer, 1.0f) > 0.5f ? 1 : 0)) * TILE_SIZE;
            } else {
                 sourceRec.x = ghost.currentFrame * TILE_SIZE;
            }
            // Select row based on ghost type
            sourceRec.y = ghost.type * TILE_SIZE;

            Rectangle destRec = { ghost.position.x, ghost.position.y, TILE_SIZE, TILE_SIZE };
            Vector2 origin = { TILE_SIZE / 2, TILE_SIZE / 2 };
            DrawTexturePro(ghostsTexture, sourceRec, destRec, origin, 0, WHITE);
        }

        // --- Draw Pac-Man using Sprites ---
        float rotation = atan2(player.direction.y, player.direction.x) * RAD2DEG;
        Rectangle sourceRec = player.frameRec;
        sourceRec.x = (2 + player.currentFrame) * TILE_SIZE; // Use frame 2 for closed, 3 for open
        if (Vector2Length(player.direction) == 0) sourceRec.x = 2 * TILE_SIZE; // Still frame when not moving
        
        Rectangle destRec = { player.position.x, player.position.y, TILE_SIZE, TILE_SIZE };
        Vector2 origin = { TILE_SIZE / 2, TILE_SIZE / 2 };
        DrawTexturePro(pacmanTexture, sourceRec, destRec, origin, rotation, WHITE);


        // Draw UI
        DrawText(TextFormat("SCORE: %04i", score), 20, 10, 20, LIME);
        
        // --- Draw visual life counter ---
        for (int i = 0; i < playerLives; i++) {
            Rectangle lifeSourceRec = { 0, 0, TILE_SIZE, TILE_SIZE }; // Pacman facing right
            Rectangle lifeDestRec = { GetScreenWidth() - 120.0f + (i * TILE_SIZE), 10, TILE_SIZE, TILE_SIZE };
            DrawTexturePro(pacmanTexture, lifeSourceRec, lifeDestRec, {0,0}, 0, WHITE);
        }

        if (gameOver) {
            // ... (Game over text code) ...
        }

        EndDrawing();
    }

    // NEW: Unload textures from memory
    UnloadTexture(pacmanTexture);
    UnloadTexture(ghostsTexture);

    delete[] nodes;
    CloseWindow();
    return 0;
}


// ... (The LoadMap and ResetRound functions remain unchanged from the previous version) ...