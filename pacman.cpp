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

// Enum for different ghost personalities
enum GhostType { BLINKY, PINKY, INKY, CLYDE };

// A* Pathfinding Structure
struct Node {
    bool isObstacle = false;
    bool isVisited = false;
    float globalGoal;
    float localGoal;
    int x;
    int y;
    std::vector<Node*> neighbors;
    Node* parent = nullptr;
};

struct Player {
    Vector2 position;
    Vector2 startPosition; // To reset after losing a life
    Vector2 direction = {0, 0};
    Vector2 desiredDirection = {0, 0};
    float speed;
    float radius;
    Color color;
};

struct Ghost {
    Vector2 position;
    Vector2 startPosition; // To reset after player loses a life
    std::vector<Node*> path;
    GhostType type;
    float speed;
    float radius;
    Color color;
};

struct Pellet {
    Vector2 position;
    float radius;
    bool active;
    Color color;
};

struct Wall {
    Rectangle rect;
    Color color;
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
int mapWidth = 0;
int mapHeight = 0;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void LoadMap(const char* fileName);
void ResetRound();

// Helper function
Vector2 WorldToTile(Vector2 worldPos) {
    return { (float)floor(worldPos.x / TILE_SIZE), (float)floor(worldPos.y / TILE_SIZE) };
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    // Initialization
    //--------------------------------------------------------------------------------------
    LoadMap("level.txt");

    const int screenWidth = mapWidth * TILE_SIZE;
    const int screenHeight = mapHeight * TILE_SIZE;

    InitWindow(screenWidth, screenHeight, "Pac-Man Legends by Gemini");
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose()) {
        // Update
        //----------------------------------------------------------------------------------
        if (!gameOver) {
            // -- Player Controls --
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
                    if (!nodes[nextTileY * mapWidth + nextTileX].isObstacle) {
                        player.direction = player.desiredDirection;
                    }
                }
                int currentNextTileX = playerTile.x + player.direction.x;
                int currentNextTileY = playerTile.y + player.direction.y;
                if (nodes[currentNextTileY * mapWidth + currentNextTileX].isObstacle) {
                    player.direction = { 0, 0 };
                }
            }
            player.position.x += player.direction.x * player.speed;
            player.position.y += player.direction.y * player.speed;

            // -- Ghost AI --
            Vector2 playerTilePos = WorldToTile(player.position);

            for (auto& ghost : ghosts) {
                Vector2 ghostTilePos = WorldToTile(ghost.position);
                Vector2 ghostTileCenter = { ghostTilePos.x * TILE_SIZE + TILE_SIZE / 2, ghostTilePos.y * TILE_SIZE + TILE_SIZE / 2 };

                if (Vector2Distance(ghost.position, ghostTileCenter) < ghost.speed || ghost.path.empty()) {
                    ghost.position = ghostTileCenter;

                    // Determine target tile based on ghost type
                    Vector2 targetTile = playerTilePos;
                    switch (ghost.type) {
                        case PINKY:
                            targetTile = { playerTilePos.x + (player.direction.x * 4), playerTilePos.y + (player.direction.y * 4) };
                            break;
                        case INKY: {
                            Vector2 blinkyPos = {0, 0};
                            for(const auto& g : ghosts) { if(g.type == BLINKY) { blinkyPos = WorldToTile(g.position); break; }}
                            Vector2 pivot = { playerTilePos.x + (player.direction.x * 2), playerTilePos.y + (player.direction.y * 2) };
                            targetTile.x = pivot.x + (pivot.x - blinkyPos.x);
                            targetTile.y = pivot.y + (pivot.y - blinkyPos.y);
                            break;
                        }
                        case CLYDE: {
                            float distance = Vector2Distance(ghostTilePos, playerTilePos);
                            if (distance < 8.0f) {
                                targetTile = { 1, (float)mapHeight - 2 }; // Retreat to bottom-left corner
                            } else {
                                targetTile = playerTilePos;
                            }
                            break;
                        }
                        case BLINKY: // Default case
                        default:
                            targetTile = playerTilePos;
                            break;
                    }

                    // Clamp target to map bounds
                    targetTile.x = Clamp(targetTile.x, 0, mapWidth - 1);
                    targetTile.y = Clamp(targetTile.y, 0, mapHeight - 1);
                    if (nodes[(int)targetTile.y * mapWidth + (int)targetTile.x].isObstacle) {
                        targetTile = playerTilePos; // Fallback to player pos if target is a wall
                    }

                    // A* Pathfinding Logic
                    for (int i = 0; i < mapWidth * mapHeight; i++) {
                        nodes[i].isVisited = false; nodes[i].globalGoal = INFINITY;
                        nodes[i].localGoal = INFINITY; nodes[i].parent = nullptr;
                    }
                    Node* nodeStart = &nodes[(int)ghostTilePos.y * mapWidth + (int)ghostTilePos.x];
                    Node* nodeEnd = &nodes[(int)targetTile.y * mapWidth + (int)targetTile.x];
                    nodeStart->localGoal = 0.0f;
                    nodeStart->globalGoal = Vector2Distance(ghost.position, player.position);

                    std::list<Node*> listNotTestedNodes;
                    listNotTestedNodes.push_back(nodeStart);
                    
                    while (!listNotTestedNodes.empty() && listNotTestedNodes.front() != nodeEnd) {
                        listNotTestedNodes.sort([](const Node* a, const Node* b) { return a->globalGoal < b->globalGoal; });
                        while (!listNotTestedNodes.empty() && listNotTestedNodes.front()->isVisited) listNotTestedNodes.pop_front();
                        if (listNotTestedNodes.empty()) break;

                        Node* currentNode = listNotTestedNodes.front();
                        currentNode->isVisited = true;
                        for (auto nodeNeighbor : currentNode->neighbors) {
                            if (!nodeNeighbor->isVisited && !nodeNeighbor->isObstacle) listNotTestedNodes.push_back(nodeNeighbor);
                            float fPossiblyLowerGoal = currentNode->localGoal + 1.0f;
                            if (fPossiblyLowerGoal < nodeNeighbor->localGoal) {
                                nodeNeighbor->parent = currentNode;
                                nodeNeighbor->localGoal = fPossiblyLowerGoal;
                                nodeNeighbor->globalGoal = nodeNeighbor->localGoal + Vector2Distance({(float)nodeNeighbor->x, (float)nodeNeighbor->y}, {(float)nodeEnd->x, (float)nodeEnd->y});
                            }
                        }
                    }
                    ghost.path.clear();
                    Node* p = nodeEnd;
                    while (p != nullptr) { ghost.path.push_back(p); p = p->parent; }
                    std::reverse(ghost.path.begin(), ghost.path.end());
                }

                // Move ghost along its calculated path
                if (!ghost.path.empty() && ghost.path.size() > 1) {
                    Node* nextNode = ghost.path[1];
                    Vector2 targetPos = { (float)nextNode->x * TILE_SIZE + TILE_SIZE / 2, (float)nextNode->y * TILE_SIZE + TILE_SIZE / 2 };
                    Vector2 direction = Vector2Normalize({targetPos.x - ghost.position.x, targetPos.y - ghost.position.y});
                    ghost.position.x += direction.x * ghost.speed;
                    ghost.position.y += direction.y * ghost.speed;
                }
            }

            // --- Collision Detection ---
            // Player vs Pellets
            for (auto& pellet : pellets) {
                if (pellet.active && CheckCollisionCircles(player.position, player.radius, pellet.position, pellet.radius)) {
                    pellet.active = false; score += 10; activePellets--;
                }
            }

            // Player vs Ghosts
            for (const auto& ghost : ghosts) {
                if (CheckCollisionCircles(player.position, player.radius, ghost.position, ghost.radius)) {
                    playerLives--;
                    if (playerLives > 0) {
                        ResetRound();
                    } else {
                        gameOver = true;
                    }
                    break; // Only process one collision per frame
                }
            }
            if (activePellets == 0) gameOver = true; // Win condition

        } else { // Game is over
            if (IsKeyDown(KEY_ENTER)) {
                delete[] nodes;
                nodes = nullptr;
                LoadMap("level.txt");
            }
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        ClearBackground(BLACK);

        for (const auto& wall : walls) DrawRectangleRec(wall.rect, wall.color);
        for (const auto& pellet : pellets) if (pellet.active) DrawCircleV(pellet.position, pellet.radius, pellet.color);
        for (const auto& ghost : ghosts) DrawCircleV(ghost.position, ghost.radius, ghost.color);
        DrawCircleV(player.position, player.radius, player.color);

        // Draw UI
        DrawText(TextFormat("SCORE: %04i", score), 20, 10, 20, LIME);
        DrawText(TextFormat("LIVES: %i", playerLives), GetScreenWidth() - 100, 10, 20, LIME);

        if (gameOver) {
            const char* message = (activePellets == 0) ? "YOU WIN!" : "GAME OVER";
            Vector2 textSize = MeasureTextEx(GetFontDefault(), message, 40, 4);
            DrawText(message, (GetScreenWidth() - textSize.x) / 2, GetScreenHeight() / 2 - 40, 40, YELLOW);
            
            const char* restartMessage = "PRESS [ENTER] TO RESTART";
            Vector2 restartTextSize = MeasureTextEx(GetFontDefault(), restartMessage, 20, 2);
            DrawText(restartMessage, (GetScreenWidth() - restartTextSize.x) / 2, GetScreenHeight() / 2 + 20, 20, GREEN);
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    delete[] nodes;
    CloseWindow();
    return 0;
}


// Function to load map and initialize game state
void LoadMap(const char* fileName) {
    walls.clear(); pellets.clear(); ghosts.clear();
    std::vector<std::string> mapData;
    std::ifstream file(fileName);
    std::string line;
    
    while (std::getline(file, line)) mapData.push_back(line);
    file.close();

    mapHeight = mapData.size();
    mapWidth = mapData[0].size();
    
    nodes = new Node[mapWidth * mapHeight];
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            nodes[y * mapWidth + x].x = x;
            nodes[y * mapWidth + x].y = y;
            nodes[y * mapWidth + x].isObstacle = (mapData[y][x] == '#');

            char tile = mapData[y][x];
            Vector2 pos = { (float)x * TILE_SIZE, (float)y * TILE_SIZE };

            switch (tile) {
                case '#': walls.push_back({ { pos.x, pos.y, TILE_SIZE, TILE_SIZE }, GREEN }); break;
                case '.': pellets.push_back({ { pos.x + TILE_SIZE / 2, pos.y + TILE_SIZE / 2 }, 4.0f, true, WHITE }); break;
                case 'P': {
                    player.position = { pos.x + TILE_SIZE / 2, pos.y + TILE_SIZE / 2 };
                    player.startPosition = player.position;
                    player.speed = 2.0f;
                    player.radius = TILE_SIZE / 2 - 2;
                    player.color = YELLOW;
                } break;
                case 'G': {
                    Ghost newGhost;
                    newGhost.position = { pos.x + TILE_SIZE / 2, pos.y + TILE_SIZE / 2 };
                    newGhost.startPosition = newGhost.position;
                    newGhost.speed = 1.5f;
                    newGhost.radius = TILE_SIZE / 2 - 2;
                    
                    int ghostCount = ghosts.size();
                    switch(ghostCount) {
                        case 0: newGhost.color = RED; newGhost.type = BLINKY; break;
                        case 1: newGhost.color = PINK; newGhost.type = PINKY; break;
                        case 2: newGhost.color = SKYBLUE; newGhost.type = INKY; break;
                        case 3: newGhost.color = ORANGE; newGhost.type = CLYDE; break;
                        default: newGhost.color = GRAY; newGhost.type = BLINKY; break;
                    }
                    ghosts.push_back(newGhost);
                } break;
            }
        }
    }
    
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            if (y > 0) nodes[y * mapWidth + x].neighbors.push_back(&nodes[(y - 1) * mapWidth + x]);
            if (y < mapHeight - 1) nodes[y * mapWidth + x].neighbors.push_back(&nodes[(y + 1) * mapWidth + x]);
            if (x > 0) nodes[y * mapWidth + x].neighbors.push_back(&nodes[y * mapWidth + (x - 1)]);
            if (x < mapWidth - 1) nodes[y * mapWidth + x].neighbors.push_back(&nodes[y * mapWidth + (x + 1)]);
        }
    }

    playerLives = 3;
    score = 0;
    activePellets = pellets.size();
    gameOver = false;
}

// Function to reset player and ghost positions after losing a life
void ResetRound() {
    player.position = player.startPosition;
    player.direction = { 0, 0 };
    player.desiredDirection = { 0, 0 };

    for (auto& ghost : ghosts) {
        ghost.position = ghost.startPosition;
        ghost.path.clear();
    }
}