#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <algorithm>

const int screenWidth = 1280;
const int screenHeight = 720;
const int numChannels = 10;
int currentChannel = 0;

// ---------- Base Class ----------
class IChannel {
public:
    virtual void Draw() = 0;
    virtual void Update() {}
    virtual void OnEnter() {}
    virtual void OnExit() {} 
    virtual ~IChannel() {}
};

// ---------- GameChannel ----------
class GameChannel : public IChannel {
private:
    int channelNumber;
public:
    GameChannel(int channel) {
        channelNumber = channel % numChannels;
    }
    void Draw() override {
        ClearBackground(RAYWHITE);
        DrawText(TextFormat("Game %d Running!", channelNumber),
                 screenWidth / 2 - 80, screenHeight / 2 - 10, 20, DARKGREEN);
    }
};

class PacmanChannel : public IChannel {
private:
    //----------------------------------------------------------------------------------
    // Types and Structures Definition
    //----------------------------------------------------------------------------------
    static constexpr float TILE_SIZE = 24.0f;

    enum GhostType { BLINKY, PINKY, INKY, CLYDE };
    enum GhostState { CHASING, FRIGHTENED, EATEN };

    struct Player {
        Vector2 position, startPosition, direction = {0, 0}, desiredDirection = {0, 0};
        float speed = 2.5f;
        float radius = TILE_SIZE / 2.0f - 2.0f;
        int frameCounter = 0;
        int currentFrame = 0;
    };

    struct Ghost {
        Vector2 position, startPosition;
        Vector2 direction = { -1, 0 };
        GhostType type;
        GhostState state = CHASING;
        float stateTimer = 0.0f;
        float speed = 2.0f;
        float radius = TILE_SIZE / 2.0f - 2.0f;
        int frameCounter = 0;
        int currentFrame = 0;
    };

    struct Pellet {
        Vector2 position;
        float radius;
        bool active;
        bool isPowerPellet = false;
        int points;
    };

    struct Wall {
        Rectangle rect;
    };

    //----------------------------------------------------------------------------------
    // Game State and Asset Variables
    //----------------------------------------------------------------------------------
    std::vector<Wall> walls;
    std::vector<Pellet> pellets;
    std::vector<Ghost> ghosts;
    Player player;

    int playerLives = 3;
    int score = 0;
    int activePellets = 0;
    int mapWidth = 0, mapHeight = 0;
    bool gameOver = false;
    bool victory = false;

    Texture2D pacmanTexture;
    Texture2D ghostsTexture;

    //----------------------------------------------------------------------------------
    // Private Module Functions
    //----------------------------------------------------------------------------------

    Vector2 WorldToTile(Vector2 worldPos) {
        return { (float)floor(worldPos.x / TILE_SIZE), (float)floor(worldPos.y / TILE_SIZE) };
    }

    bool IsWall(int x, int y) {
        if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) return true;
        for (const auto& wall : walls) {
            if (CheckCollisionPointRec({(float)x * TILE_SIZE, (float)y * TILE_SIZE}, wall.rect)) {
                return true;
            }
        }
        return false;
    }

    void LoadMap(const char* fileName) {
        walls.clear();
        pellets.clear();
        ghosts.clear();
        activePellets = 0;
        mapWidth = 0;
        mapHeight = 0;

        std::ifstream file(fileName);
        std::string line;
        int y = 0;
        while (std::getline(file, line)) {
            mapWidth = std::max(mapWidth, (int)line.length());
            for (int x = 0; x < line.length(); ++x) {
                Vector2 pos = { x * TILE_SIZE + TILE_SIZE / 2, y * TILE_SIZE + TILE_SIZE / 2 };
                switch (line[x]) {
                    case '#': walls.push_back({ { pos.x - TILE_SIZE/2, pos.y - TILE_SIZE/2, TILE_SIZE, TILE_SIZE } }); break;
                    case '.': pellets.push_back({ pos, 2.0f, true, false, 10 }); activePellets++; break;
                    case 'O': pellets.push_back({ pos, 6.0f, true, true, 50 }); activePellets++; break;
                    case 'P': player.startPosition = pos; break;
                    case 'G': ghosts.push_back({ pos, pos, {-1, 0}, (GhostType)(ghosts.size() % 4) }); break;
                }
            }
            y++;
        }
        mapHeight = y;
    }

    void ResetGame() {
        player.position = player.startPosition;
        player.direction = {0, 0};
        player.desiredDirection = {0, 0};

        for (auto& ghost : ghosts) {
            ghost.position = ghost.startPosition;
            ghost.state = CHASING;
            ghost.direction = {-1, 0};
        }

        activePellets = 0;
        for (auto& p : pellets) {
            if (!p.active) {
                p.active = true;
            }
            activePellets++;
        }
        
        playerLives = 3;
        score = 0;
        gameOver = false;
        victory = false;
    }

    void ResetAfterLifeLost() {
        player.position = player.startPosition;
        player.direction = {0, 0};
        player.desiredDirection = {0, 0};
        for (auto& ghost : ghosts) {
            ghost.position = ghost.startPosition;
            ghost.state = CHASING;
            ghost.direction = {-1, 0};
        }
    }

public:
    PacmanChannel() {
        LoadMap("level.txt");
        pacmanTexture = LoadTexture("pacman_sprites.png");
        ghostsTexture = LoadTexture("ghost_sprites.png");
        ResetGame();
    }

    ~PacmanChannel() {
        UnloadTexture(pacmanTexture);
        UnloadTexture(ghostsTexture);
    }

    void OnEnter() override {
        ResetGame();
    }

    void Update() override {
        if (gameOver || victory) {
            if (IsKeyPressed(KEY_ENTER)) {
                ResetGame();
            }
            return;
        }

        // --- Player Update ---
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.desiredDirection = { 1.0f, 0.0f };
        else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.desiredDirection = { -1.0f, 0.0f };
        else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) player.desiredDirection = { 0.0f, -1.0f };
        else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) player.desiredDirection = { 0.0f, 1.0f };

        Vector2 playerTile = WorldToTile(player.position);
        Vector2 playerTileCenter = { playerTile.x * TILE_SIZE + TILE_SIZE / 2, playerTile.y * TILE_SIZE + TILE_SIZE / 2 };

        if (Vector2Distance(player.position, playerTileCenter) < player.speed * 1.1f) {
            player.position = playerTileCenter;
            Vector2 nextTileDesired = Vector2Add(playerTile, player.desiredDirection);
            if (!IsWall(nextTileDesired.x, nextTileDesired.y)) {
                player.direction = player.desiredDirection;
            }
            Vector2 nextTileCurrent = Vector2Add(playerTile, player.direction);
            if (IsWall(nextTileCurrent.x, nextTileCurrent.y)) {
                player.direction = { 0, 0 };
            }
        }
        player.position = Vector2Add(player.position, Vector2Scale(player.direction, player.speed));

        player.frameCounter++;
        if (player.frameCounter >= 8) {
            player.frameCounter = 0;
            player.currentFrame = (player.currentFrame + 1) % 2;
        }
        if (Vector2Length(player.direction) == 0) player.currentFrame = 0;

        // --- Ghost Update ---
        for (auto& ghost : ghosts) {
            if (ghost.state != CHASING) {
                ghost.stateTimer -= GetFrameTime();
                if (ghost.stateTimer <= 0) {
                    ghost.state = CHASING;
                    ghost.speed = 2.0f;
                }
            }

            Vector2 ghostTile = WorldToTile(ghost.position);
            Vector2 ghostTileCenter = { ghostTile.x * TILE_SIZE + TILE_SIZE / 2, ghostTile.y * TILE_SIZE + TILE_SIZE / 2 };
            
            if (Vector2Distance(ghost.position, ghostTileCenter) < ghost.speed * 1.1f) {
                 ghost.position = ghostTileCenter;
                float min_dist = 10000.0f;
                Vector2 best_dir = {0,0};
                std::vector<Vector2> possible_dirs = {{0,-1}, {0,1}, {-1,0}, {1,0}};
                Vector2 oppositeDir = Vector2Scale(ghost.direction, -1);

                for(const auto& dir : possible_dirs) {
                    if (dir.x == oppositeDir.x && dir.y == oppositeDir.y) continue;
                    Vector2 nextTile = Vector2Add(ghostTile, dir);
                    if (!IsWall(nextTile.x, nextTile.y)) {
                        float dist = Vector2Distance(nextTile, playerTile);
                        if (dist < min_dist) {
                            min_dist = dist;
                            best_dir = dir;
                        }
                    }
                }
                if (best_dir.x != 0 || best_dir.y != 0) {
                    ghost.direction = best_dir;
                }
            }
            ghost.position = Vector2Add(ghost.position, Vector2Scale(ghost.direction, ghost.speed));

            ghost.frameCounter++;
            if (ghost.frameCounter >= 8) {
                ghost.frameCounter = 0;
                ghost.currentFrame = (ghost.currentFrame + 1) % 2;
            }
        }

        // --- Collision Detection ---
        for (auto& p : pellets) {
            if (p.active && CheckCollisionCircles(player.position, player.radius, p.position, p.radius)) {
                p.active = false;
                score += p.points;
                activePellets--;
                if (p.isPowerPellet) {
                    for (auto& ghost : ghosts) {
                        if (ghost.state != EATEN) {
                            ghost.state = FRIGHTENED;
                            ghost.stateTimer = 7.0f;
                            ghost.speed = 1.5f;
                        }
                    }
                }
            }
        }

        for (auto& ghost : ghosts) {
            if (CheckCollisionCircles(player.position, player.radius, ghost.position, ghost.radius)) {
                if (ghost.state == CHASING) {
                    playerLives--;
                    if (playerLives <= 0) gameOver = true;
                    else ResetAfterLifeLost();
                } else if (ghost.state == FRIGHTENED) {
                    score += 200;
                    ghost.state = EATEN;
                    ghost.position = ghost.startPosition;
                    ghost.stateTimer = 3.0f;
                }
            }
        }

        if (activePellets <= 0) {
            victory = true;
        }
    }

    void Draw() override {
        // Get the logical game dimensions
        float gameWidth = mapWidth * TILE_SIZE;
        float gameHeight = mapHeight * TILE_SIZE;

        ClearBackground(BLACK);

        // Draw Map centered on the screen
        Vector2 offset = { (GetScreenWidth() - gameWidth) / 2, (GetScreenHeight() - gameHeight) / 2 };
        
        for (const auto& wall : walls) {
             DrawRectangleRec({wall.rect.x + offset.x, wall.rect.y + offset.y, wall.rect.width, wall.rect.height}, BLUE);
        }
        for (const auto& pellet : pellets) {
            if (pellet.active) DrawCircleV(Vector2Add(pellet.position, offset), pellet.radius, YELLOW);
        }

        // Draw Ghosts
        for (const auto& ghost : ghosts) {
            Rectangle sourceRec = {0, 0, 16, 16};
            if (ghost.state == FRIGHTENED) {
                sourceRec.x = (4 + (ghost.stateTimer < 3.0f && (int)(ghost.stateTimer*4)%2==0 ? 1 : 0)) * 16;
            } else if (ghost.state == EATEN) {
                sourceRec.x = 6 * 16;
            } else {
                sourceRec.x = ghost.currentFrame * 16;
            }
            sourceRec.y = ghost.type * 16;
            Rectangle destRec = { ghost.position.x + offset.x, ghost.position.y + offset.y, TILE_SIZE, TILE_SIZE };
            DrawTexturePro(ghostsTexture, sourceRec, destRec, {TILE_SIZE/2, TILE_SIZE/2}, 0, WHITE);
        }

        // Draw Player
        float rotation = atan2f(player.direction.y, player.direction.x) * RAD2DEG;
        if (Vector2LengthSqr(player.direction) == 0) rotation = 0;
        Rectangle sourceRec = { (float)player.currentFrame * 16, 0, 16, 16 };
        if (Vector2LengthSqr(player.direction) == 0) sourceRec.x = 2 * 16;
        Rectangle destRec = { player.position.x + offset.x, player.position.y + offset.y, TILE_SIZE, TILE_SIZE };
        DrawTexturePro(pacmanTexture, sourceRec, destRec, {TILE_SIZE/2, TILE_SIZE/2}, rotation, WHITE);

        // --- UI DRAWING (FIXED) ---
        // Draw UI relative to the main screen, not the game world
        DrawText(TextFormat("SCORE: %04i", score), 20, 10, 20, LIME);
        for (int i = 0; i < playerLives; i++) {
            // Use GetScreenWidth() to position UI correctly on the render texture
            DrawTexturePro(pacmanTexture, {0, 0, 16, 16}, {GetScreenWidth() - 120.0f + (i * TILE_SIZE), 10, TILE_SIZE, TILE_SIZE}, {0,0}, 0, WHITE);
        }

        // Draw Game Over/Victory Text centered on the screen
        if (gameOver) {
            DrawText("GAME OVER", GetScreenWidth() / 2 - MeasureText("GAME OVER", 40) / 2, GetScreenHeight() / 2 - 40, 40, RED);
            DrawText("Press [ENTER] to Restart", GetScreenWidth() / 2 - MeasureText("Press [ENTER] to Restart", 20) / 2, GetScreenHeight() / 2 + 10, 20, GRAY);
        }
        if (victory) {
            DrawText("VICTORY!", GetScreenWidth() / 2 - MeasureText("VICTORY!", 40) / 2, GetScreenHeight() / 2 - 40, 40, GOLD);
            DrawText("Press [ENTER] to Restart", GetScreenWidth() / 2 - MeasureText("Press [ENTER] to Restart", 20) / 2, GetScreenHeight() / 2 + 10, 20, GRAY);
        }
    }
};

class PongChannel : public IChannel {
private:
    // Use an enum to manage the game's state, scoped to this class
    enum GameState {
        PLAYING,
        GAME_OVER
    };

    // Game Objects
    Rectangle player;
    Rectangle ai;
    Vector2 ballPosition;
    Vector2 ballSpeed;

    // Game Properties
    const float playerSpeed = 500.0f;
    const float aiSpeed = 350.0f;
    const float initialBallSpeed = 350.0f;
    const float ballRadius = 8.0f;

    // Game State Variables
    int playerScore;
    int aiScore;
    const int winningScore = 3;
    GameState currentState;
    const char* winnerText;

    // Helper function to reset the game to its initial state
    void ResetGame() {
        player = { 30.0f, screenHeight / 2.0f - 50.0f, 10.0f, 100.0f };
        ai = { screenWidth - 40.0f, screenHeight / 2.0f - 50.0f, 10.0f, 100.0f };
        ballPosition = { screenWidth / 2.0f, screenHeight / 2.0f };
        ballSpeed = { initialBallSpeed, initialBallSpeed };

        playerScore = 0;
        aiScore = 0;
        winnerText = "";
        currentState = PLAYING;
    }

public:
    // Constructor: Initializes the game
    PongChannel() {
        ResetGame();
    }

    // OnEnter is called when the channel becomes active
    void OnEnter() override {
        ResetGame(); // Restart the game every time you switch to this channel
    }

    // Update contains all the game logic
    void Update() override {
        switch (currentState) {
            case PLAYING: {
                // Player paddle movement
                if (IsKeyDown(KEY_W) && player.y > 0) {
                    player.y -= playerSpeed * GetFrameTime();
                }
                if (IsKeyDown(KEY_S) && player.y < screenHeight - player.height) {
                    player.y += playerSpeed * GetFrameTime();
                }

                // AI paddle movement
                if (ai.y + ai.height / 2 < ballPosition.y) ai.y += aiSpeed * GetFrameTime();
                if (ai.y + ai.height / 2 > ballPosition.y) ai.y -= aiSpeed * GetFrameTime();
                if (ai.y < 0) ai.y = 0;
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
                    ballSpeed.x *= -1.1f;
                    ballSpeed.y = (ballPosition.y - (player.y + player.height / 2)) / (player.height / 2) * std::fabs(ballSpeed.x);
                }
                if (CheckCollisionCircleRec(ballPosition, ballRadius, ai) && ballSpeed.x > 0) {
                    ballSpeed.x *= -1.1f;
                    ballSpeed.y = (ballPosition.y - (ai.y + ai.height / 2)) / (ai.height / 2) * std::fabs(ballSpeed.x);
                }

                // Scoring Logic
                bool pointScored = false;
                if (ballPosition.x - ballRadius > screenWidth) { // Player scores
                    playerScore++;
                    ballSpeed.x = -initialBallSpeed;
                    pointScored = true;
                }
                if (ballPosition.x + ballRadius < 0) { // AI scores
                    aiScore++;
                    ballSpeed.x = initialBallSpeed;
                    pointScored = true;
                }

                if (pointScored) {
                    ballPosition = { screenWidth / 2.0f, screenHeight / 2.0f };
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
                if (IsKeyPressed(KEY_ENTER)) {
                    ResetGame(); // Use the helper function to restart
                }
            } break;
        }
    }

    // Draw contains all the rendering logic
    void Draw() override {
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
    }
};

// ---------- RickRollChannel ----------
class RickRollChannel : public IChannel {
private:
    std::vector<Texture2D> frames;
    int currentFrame = 0;
    float frameTime = 0.04f; // ~25 FPS
    float timer = 0.0f;
    Sound rickrollSound;
    bool isPlaying = false; 

public:
    RickRollChannel() {
        // Load all frames into memory
        for (int i = 0; ; i++) {
            std::string path = TextFormat("E:/Codaing/Useless_Project/assets/rickroll/frame_%03d.png", i);
            if (!FileExists(path.c_str())) break; // stop if no more frames

            Image img = LoadImage(path.c_str());
            Texture2D tex = LoadTextureFromImage(img);
            UnloadImage(img);

            frames.push_back(tex);
        }

        if (frames.empty()) {
            TraceLog(LOG_ERROR, "No RickRoll frames found!");
        } else {
            TraceLog(LOG_INFO, "Loaded %d RickRoll frames", (int)frames.size());
        }

        rickrollSound = LoadSound("assets/rickroll.wav");
    }

    void OnEnter() override {
        if (!IsSoundPlaying(rickrollSound)) {
            PlaySound(rickrollSound);
        }
        // Reset the animation to the beginning
        currentFrame = 0;
        timer = 0.0f;
    }

    void OnExit() override {
        if (IsSoundPlaying(rickrollSound)) {
            StopSound(rickrollSound);
        }
    }    

    void Update() override {
        if (frames.empty()) return;

        timer += GetFrameTime();
        if (timer >= frameTime) {
            timer -= frameTime;
            currentFrame = (currentFrame + 1) % frames.size();
        }
    }

    void Draw() override {
        ClearBackground(BLACK);
        if (!frames.empty()) {
            Texture2D &tex = frames[currentFrame];
            // Draw centered
            DrawTexture(tex, 640 - tex.width / 2, 360 - tex.height / 2, WHITE);
        }
        DrawText("RickRoll Channel", 10, 10, 20, YELLOW);
    }

    ~RickRollChannel() {
        for (auto &tex : frames) {
            UnloadTexture(tex);
        }
        UnloadSound(rickrollSound);
    }
};


// ---------- DVDChannel ----------
class DVDChannel : public IChannel {
private:
    Texture2D dvdLogo;
    Vector2 pos;
    Vector2 speed;
    int logoWidth, logoHeight;
    int bounceCounter = 0;
    Color currentColor = WHITE;
    const float scale = 0.8f; 

public:
    DVDChannel() {
        dvdLogo = LoadTexture("assets/dvd.png");

        logoWidth = dvdLogo.width * scale;
        logoHeight = dvdLogo.height * scale;

        pos = { 640 - logoWidth / 2.0f, 360 - logoHeight / 2.0f };
        speed = { 4, 3 }; // pixels per frame
    }

    void Update() {
        pos.x += speed.x;
        pos.y += speed.y;

        bool hitX = false, hitY = false;

        auto randomColor = []() -> Color {
            return Color{ 
                (unsigned char)GetRandomValue(50, 255), // Red   (bright)
                (unsigned char)GetRandomValue(50, 255), // Green (bright)
                (unsigned char)GetRandomValue(50, 255), // Blue  (bright)
                255                                      // Alpha (fully visible)
            };
        };



        // Bounce horizontally
        if (pos.x <= 0) {
            pos.x = 0;
            speed.x *= -1;
            hitX = true;
            if (speed.y != 0) bounceCounter++;
            currentColor = randomColor();
        }
        else if (pos.x + logoWidth >= 1280) {
            pos.x = 1280 - logoWidth;
            speed.x *= -1;
            hitX = true;
            if (speed.y != 0) bounceCounter++;
            currentColor = randomColor();
        }

        // Bounce vertically
        if (pos.y <= 0) {
            pos.y = 0;
            speed.y *= -1;
            hitY = true;
            if (speed.y != 0) bounceCounter++;
            currentColor = randomColor();
        }
        else if (pos.y + logoHeight >= 720) {
            pos.y = 720 - logoHeight;
            speed.y *= -1;
            hitY = true;
            if (speed.y != 0) bounceCounter++;
            currentColor = randomColor();
        }

        // If both X and Y hit → perfect corner hit
        if (hitX && hitY) {
            speed.x = 0;
            speed.y = 0;
        }
    }

    void Draw() override {
        ClearBackground(BLACK);
        DrawTextureEx(dvdLogo, pos, 0.0f, scale, currentColor);
        DrawText(TextFormat("Bounce Counter: %d", bounceCounter), 540, 680, 20, LIGHTGRAY);
    }

    ~DVDChannel() {
        UnloadTexture(dvdLogo);
    }
};

// ---------- StaticChannel ----------
class StaticChannel : public IChannel {
private:
    Image noiseImage;
    Texture2D noiseTexture;

public:
    StaticChannel() {
        // Create a blank image initially
        noiseImage = GenImageColor(screenWidth, screenHeight, BLACK);
        noiseTexture = LoadTextureFromImage(noiseImage);
    }

    void Update() override {
        // Fill image pixels with random black or white
        for (int y = 0; y < screenHeight; y++) {
            for (int x = 0; x < screenWidth; x++) {
                unsigned char val = (GetRandomValue(0, 1) * 255);
                ImageDrawPixel(&noiseImage, x, y, { val, val, val, 255 });
            }
        }

        // Update texture from modified image
        UpdateTexture(noiseTexture, noiseImage.data);
    }

    void Draw() override {
        DrawTexture(noiseTexture, 0, 0, WHITE);
    }

    ~StaticChannel() {
        UnloadTexture(noiseTexture);
        UnloadImage(noiseImage);
    }
};


// ---------- CRT Shader Source ----------
const char* crtShaderCode = R"(
#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform float time;
uniform vec2 resolution;

out vec4 finalColor;

void main()
{
    // UV coords from 0 to 1 and Barrel Distortion
    vec2 uv = vec2(fragTexCoord.x, 1.0 - fragTexCoord.y);
    float distortion = 0.1;
    vec2 centeredUV = uv * 2.0 - 1.0;
    float r2 = dot(centeredUV, centeredUV);
    centeredUV *= 1.0 + distortion * r2;
    uv = centeredUV * 0.5 + 0.5;

    // Border check
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        finalColor = vec4(vec3(0.2), 1.0);
        return;
    }

    // --- CORRECTED ORDER OF EFFECTS ---

    // 1. Chromatic Aberration (Applied FIRST)
    // Check if the pixel is grayscale to protect the static channel from color fringing.
    vec3 col;
    vec3 center_color = texture(texture0, uv).rgb;

    // A small tolerance to check if R, G, and B values are nearly equal.
    float gray_threshold = 0.05;

    if (abs(center_color.r - center_color.g) < gray_threshold && abs(center_color.g - center_color.b) < gray_threshold)
    {
        // PIXEL IS GRAYSCALE: Don't apply chromatic aberration.
        col = center_color;
    }
    else
    {
        // PIXEL IS COLORFUL: Apply the standard chromatic aberration.
        float offset = 1.5 / resolution.x;
        col.r = texture(texture0, uv + vec2(offset, 0.0)).r;
        col.g = center_color.g; // Reuse the green channel from our first sample
        col.b = texture(texture0, uv - vec2(offset, 0.0)).b;
    }

    // 2. Rolling Wave Flicker (Applied SECOND)
    // Now, add the flicker effect to our base color.
    float flicker_period = 5.0;
    float flicker_duration = 0.4;
    if (mod(time, flicker_period) < flicker_duration) {
        float wave = sin(uv.y * 30.0 - time * 60.0);
        wave = smoothstep(0.9, 1.0, wave);
        col += vec3(wave * 0.8);
    }

    // 3. Scanline Effect (Applied THIRD)
    float scanline = sin(uv.y * resolution.y * 1.5) * 0.04;
    col -= scanline;

    // 4. Vignette (Applied LAST)
    float vignette = smoothstep(0.8, 0.2, length(uv - 0.5));
    col *= vignette;

    // Final color output
    finalColor = vec4(col, 1.0);
}

)";

int main(void) {
    InitWindow(screenWidth, screenHeight, "Nostalgia Simulator");
    InitAudioDevice();
    SetTargetFPS(60);

    std::vector<IChannel*> channels;
    channels.push_back(new StaticChannel());         // Channel 0 - Static
    channels.push_back(new PacmanChannel());         // Channel 1 - Pacman Game
    channels.push_back(new DVDChannel());          // Channel 2 - DVD Channel
    channels.push_back(new RickRollChannel());    // Channel 3 - RickRoll
    channels.push_back(new PongChannel());       // Channel 4 - Pong Game

    Shader crtShader = LoadShaderFromMemory(0, crtShaderCode);

    int timeLoc = GetShaderLocation(crtShader, "time");
    if (timeLoc == -1) TraceLog(LOG_WARNING, "'time' uniform not found");

    int resolutionLoc = GetShaderLocation(crtShader, "resolution");
    if (resolutionLoc == -1) TraceLog(LOG_WARNING, "'resolution' uniform not found");

    
    float resolution[2] = { (float)screenWidth, (float)screenHeight };
    SetShaderValue(crtShader, resolutionLoc, resolution, SHADER_UNIFORM_VEC2);

    RenderTexture2D screenTarget = LoadRenderTexture(screenWidth, screenHeight);

    // ---------- Game Loop ----------
    while (!WindowShouldClose()) {
        // Switch channels
        bool channelChanged = false;
        int previousChannel = currentChannel;

        // --- Switch channels ---
        if (IsKeyPressed(KEY_RIGHT)) {
            currentChannel = (currentChannel + 1) % channels.size();
            channelChanged = true;
        }
        if (IsKeyPressed(KEY_LEFT)) {
            currentChannel = (currentChannel - 1 + channels.size()) % channels.size();
            channelChanged = true;
        }

        // --- Handle activation/deactivation on change ---
        if (channelChanged) {
            channels[previousChannel]->OnExit();  // Deactivate the old channel
            channels[currentChannel]->OnEnter(); // Activate the new one
        }

        float timeValue = GetTime();
        SetShaderValue(crtShader, timeLoc, &timeValue, SHADER_UNIFORM_FLOAT);

        // Draw to render texture first
        BeginTextureMode(screenTarget);
        ClearBackground(BLACK);
        channels[currentChannel]->Update();
        channels[currentChannel]->Draw();
        DrawText(TextFormat("Channel %d", currentChannel), 1150, 10, 20, DARKGRAY);
        EndTextureMode();

        // Draw the texture with CRT shader
        BeginDrawing();
        ClearBackground(BLACK);

        BeginShaderMode(crtShader);
        DrawTexture(screenTarget.texture, 0, 0, WHITE);

        EndShaderMode();

        EndDrawing();
    }

    // Cleanup
    for (auto c : channels){ c->OnExit(); c;}
    CloseAudioDevice();
    UnloadRenderTexture(screenTarget);
    UnloadShader(crtShader);
    CloseWindow();
    return 0;
}
