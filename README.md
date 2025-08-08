<img width="3188" height="1202" alt="Nostalgia Simulator Banner" src="https://github.com/user-attachments/assets/517ad8e9-ad22-457d-9538-a9e62d137cd7" />

Nostalgia Simulator 🎯
Basic Details
Team Name: The Channel Surfers
Team Members
Team Lead: Midhun - [Your College]

Member 2: [Name] - [College]

Member 3: [Name] - [College]

Project Description
The Nostalgia Simulator is a high-fidelity recreation of staring at a CRT television in the late 90s. It's a multi-channel application featuring several classic games and screensavers, all rendered through an authentic retro shader that simulates screen curvature, scanlines, and static interference.

The Problem (that doesn't exist)
In today's fast-paced world of 4K streaming and instant gratification, we've lost the simple joy of flipping through a limited number of channels, hoping to find something, anything, to watch. The youth of today will never know the suspense of a bouncing DVD logo or the hypnotic trance of screen static.

The Solution (that nobody asked for)
We've bottled that feeling. The Nostalgia Simulator provides a curated set of channels, including playable classics like Pac-Man and Pong, the legendary DVD screensaver, and even a surprise or two. It's a digital time capsule designed to solve a problem you didn't know you had: a distinct lack of fuzzy, low-resolution entertainment.

Technical Details
Technologies/Components Used
For Software:

Languages: C++, GLSL (for the CRT shader)

Libraries: raylib, raymath

Tools: MinGW (for Windows compilation), Emscripten (for web compilation), Python (for local web server)

For Hardware:

This is a software-only project. The only required hardware is a computer and a longing for the past.

Implementation
For Software:

Installation (Desktop - Windows)
# Ensure you have raylib configured with your g++ compiler
g++ main.cpp -o NostalgiaSimulator.exe -lraylib -lopengl32 -lgdi32 -lwinmm

Run (Desktop)
./NostalgiaSimulator.exe

Installation (Web)
# Ensure you have Emscripten and raylib for web configured
em++ main.cpp -o index.js -Os -s USE_GLFW=3 -s ASYNCIFY --preload-file assets -s MODULARIZE=1 -s EXPORT_ES6 -s ALLOW_MEMORY_GROWTH=1 -I "path/to/raylib/src" -L "path/to/raylib/build_web/raylib" -lraylib

Run (Web)
# You'll need a simple local server to run the web build
python -m http.server
# Then open your browser to http://localhost:8000

Project Documentation
For Software:

Screenshots (Add at least 3)
The Pac-Man channel in action, complete with the CRT shader effect.

A heated match of Pong against the surprisingly competent AI.

The legendary bouncing DVD logo, a sight that has brought joy to millions.

Diagrams
Our workflow consists of a main loop that draws the active "Channel" to a render texture, which is then drawn to the screen with the CRT shader applied.

For Hardware:
N/A

Project Demo
Video
[Add your demo video link here - e.g., a YouTube or Loom link]
This video demonstrates the seamless channel surfing, gameplay from Pac-Man and Pong, and the various visual effects of the CRT shader.

Additional Demos
[Link to the live web build if you host it somewhere like GitHub Pages]

Team Contributions
Midhun: Project Lead, core application architecture, implemented the channel system, developed Pac-Man and Pong game logic, integrated the CRT shader, and handled the Emscripten web build.

[Name 2]: [Specific contributions]

[Name 3]: [Specific contributions]

Made with ❤️ at TinkerHub Useless Projects