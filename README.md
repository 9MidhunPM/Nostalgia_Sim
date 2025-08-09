<img width="3188" height="1202" alt="frame (3)" src="https://github.com/user-attachments/assets/517ad8e9-ad22-457d-9538-a9e62d137cd7" />

# Nostalgia Simulator 🎯

## Basic Details
### Team Name: Professional Failures

### Team Members
- Team Lead: Midhun P M - Sahrdaya College of Engineering
- Member 2: Sharon Kariyattil - Sahrdaya College of Engineering

### Project Description
In today's fast-paced world of 4K streaming and instant gratification, we've lost the simple joy of flipping through a limited number of channels, hoping to find something, *anything*, to watch. The youth of today will never know the suspense of a bouncing DVD logo or the hypnotic trance of screen static.

### The Problem (that doesn't exist)
In today's fast-paced world of 4K streaming and instant gratification, we've lost the simple joy of flipping through a limited number of channels, hoping to find something, *anything*, to watch. The youth of today will never know the suspense of a bouncing DVD logo or the hypnotic trance of screen static.

### The Solution (that nobody asked for)
We've bottled that feeling. The Nostalgia Simulator provides a curated set of channels, including playable classics like Pac-Man and Pong, the legendary DVD screensaver, and even a surprise or two. It's a digital time capsule designed to solve a problem you didn't know you had: a distinct lack of fuzzy, low-resolution entertainment.

## Technical Details
### Technologies/Components Used

**Software:**
* **Languages:** C++, GLSL
* **Libraries:** raylib, raymath
* **Tools:** MinGW, Emscripten, Python

**Hardware:**
* This is a software-only project.

### Implementation

**Installation (Desktop):**
```bash
g++ main.cpp -o NostalgiaSimulator.exe -lraylib -lopengl32 -lgdi32 -lwinmm
./NostalgiaSimulator.exe
```

**Installation (Web)**
# Ensure you have Emscripten and raylib for web configured
```bash
em++ main.cpp -o index.js -Os -s USE_GLFW=3 -s ASYNCIFY --preload-file assets -s MODULARIZE=1 -s EXPORT_ES6 -s ALLOW_MEMORY_GROWTH=1 -I "path/to/raylib/src" -L "path/to/raylib/build_web/raylib" -lraylib
```

**Run (Web)**
# You'll need a simple local server to run the web build
```bash
python -m http.server
# Then open your browser to http://localhost:8000
```

**Project Documentation**
# For Software:

# Screenshots
- The Pac-Man channel in action, complete with the CRT shader effect.

- A heated match of Pong against the surprisingly competent AI.

- The legendary bouncing DVD logo, a sight that has brought joy to millions.

# Diagrams
- Our workflow consists of a main loop that draws the active "Channel" to a render texture, which is then drawn to the screen with the CRT shader applied.

# For Hardware:
 N/A

### Project Demo
- [Demo Link](https://9midhunpm.github.io/Nostalgia_Sim/)

## Team Contributions
- Midhun: Project Lead, core application architecture, implemented the channel system, developed Pac-Man and Pong game logic, integrated the CRT shader, and handled the Emscripten web build.

- Sharon: Building the core logics and debugging of pacman and pong game logic


Made with ❤️ at TinkerHub Useless Projects

![Static Badge](https://img.shields.io/badge/TinkerHub-24?color=%23000000&link=https%3A%2F%2Fwww.tinkerhub.org%2F)
![Static Badge](https://img.shields.io/badge/UselessProjects--25-25?link=https%3A%2F%2Fwww.tinkerhub.org%2Fevents%2FQ2Q1TQKX6Q%2FUseless%2520Projects)

