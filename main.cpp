#include "raylib.h"
#include <vector>
#include <string>

const int screenWidth = 1280;
const int screenHeight = 720;
const int numChannels = 10;
int currentChannel = 0;

// ---------- Base Class ----------
class IChannel {
public:
    virtual void Draw() = 0;
    virtual void Update() {} 
    virtual ~IChannel() {}\
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

// ---------- RickRollChannel ----------
class RickRollChannel : public IChannel {
private:
    std::vector<Texture2D> frames;
    int currentFrame = 0;
    float frameTime = 0.04f; // ~25 FPS
    float timer = 0.0f;

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
        // Move logo
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
    SetTargetFPS(60);

    std::vector<IChannel*> channels;
    channels.push_back(new StaticChannel());         // Channel 0 - Static
    channels.push_back(new GameChannel(1));         // Channel 1 - Pacman Game
    channels.push_back(new DVDChannel());          // Channel 2 - DVD Channel
    channels.push_back(new RickRollChannel());    // Channel 3 - RickRoll

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
        if (IsKeyPressed(KEY_RIGHT)) {
            currentChannel = (currentChannel + 1) % channels.size();
        }
        if (IsKeyPressed(KEY_LEFT)) {
            currentChannel = (currentChannel - 1 + channels.size()) % channels.size();
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
    for (auto c : channels) delete c;
    CloseWindow();
    return 0;
}
