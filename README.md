<img width="3188" height="1202" alt="frame (3)" src="https://github.com/user-attachments/assets/517ad8e9-ad22-457d-9538-a9e62d137cd7" />

# Nostalgia Simulator 🎯

## Basic Details
### Team Name: RetroRewinders

### Team Members
- Team Lead: Midhun - Sahrdaya College of Engineering
- Member 2: [Name] - [College]
- Member 3: [Name] - [College]

### Project Description
Nostalgia Simulator is a C++ application that recreates the feel of an old CRT TV with multiple “channels,” each hosting a different game or visualizer. It features classic games, video playback, and authentic CRT effects powered by raylib and GLSL shaders.

### The Problem (that doesn't exist)
Who actually needs a modern, slick UI when you can have fuzzy static, bouncing DVD logos, and pixelated Pac-Man ghosts? Nobody asked for this, but everyone secretly wants it.

### The Solution (that nobody asked for)
We’re throwing a time machine into your screen—multiple games, a rickroll video, and more—wrapped in a CRT shader that makes your monitor feel like it’s from the 80s. Channel surf your boredom away with glitchy nostalgia and retro vibes.

## Technical Details
### Technologies/Components Used

**Software:**
- C++ (Core language)
- raylib (Graphics & audio)
- GLSL (CRT post-processing shader)
- Emscripten (For web deployment)

**Hardware:**
- Standard PC or laptop capable of running OpenGL apps
- Web browser (for WebAssembly version)

### Implementation

**Installation (Desktop):**
- bash
- git clone https://github.com/9MidhunPM/Nostalgia_Sim
- cd NostalgiaSimulator
- mkdir build && cd build
- cmake ..
- make
- ./NostalgiaSimulator


**Installation (Web)**
# Ensure you have Emscripten and raylib for web configured

- em++ main.cpp -o index.js -Os -s USE_GLFW=3 -s ASYNCIFY --preload-file assets -s MODULARIZE=1 -s EXPORT_ES6 -s ALLOW_MEMORY_GROWTH=1 -I "path/to/raylib/src" -L - - "path/to/raylib/build_web/raylib" -lraylib ```

**Run (Web)**
# You'll need a simple local server to run the web build
- python -m http.server
# Then open your browser to http://localhost:8000

**Project Documentation**
# For Software:

# Screenshots
- The Pac-Man channel in action, complete with the CRT shader effect.

- A heated match of Pong against the surprisingly competent AI.

- The legendary bouncing DVD logo, a sight that has brought joy to millions.

# Diagrams
- Our workflow consists of a main loop that draws the active "Channel" to a render texture, which is then drawn to the screen with the CRT shader applied.

# For Hardware:
- N/A

### Project Demo
- [Demo Link](https://9midhunpm.github.io/Nostalgia_Sim/)

## Team Contributions
- Midhun: Project Lead, core application architecture, implemented the channel system, developed Pac-Man and Pong game logic, integrated the CRT shader, and handled the Emscripten web build.

- Sharon: Building the core logics and debugging of pacman and pong game logic


Made with ❤️ at TinkerHub Useless Projects

![Static Badge](https://img.shields.io/badge/TinkerHub-24?color=%23000000&link=https%3A%2F%2Fwww.tinkerhub.org%2F)
![Static Badge](https://img.shields.io/badge/UselessProjects--25-25?link=https%3A%2F%2Fwww.tinkerhub.org%2Fevents%2FQ2Q1TQKX6Q%2FUseless%2520Projects)

