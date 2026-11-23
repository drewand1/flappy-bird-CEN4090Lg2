#ifndef GAMECONFIG_H
#define GAMECONFIG_H

inline constexpr float GRAVITY_CONST = 400.0f;
inline constexpr float BIRD_JUMP_VEL = 200.0f;
inline constexpr float PIPE_BASE_SPEED = 200.0f;
inline constexpr float PIPE_BASE_SPAWN_COOLDOWN = 5.0f;
inline constexpr float PIPE_BASE_GAP_SIZE = 150.0f;

inline constexpr float PIPE_WIDTH = 64.0f;
inline constexpr float PIPE_HALF_WIDTH = PIPE_WIDTH / 2.0f;

inline constexpr float BIRD_WIDTH = 32.0f;
inline constexpr float BIRD_HEIGHT = 32.0f;
inline constexpr float BIRD_RADIUS = 12.0f;

inline constexpr float VIEW_WIDTH = 800.0f;
inline constexpr float VIEW_HEIGHT = 400.0f;

// override at runtime with the
// FLAPPY_SCOREBOARD environment variable, e.g. FLAPPY_SCOREBOARD=example.com:8080
inline constexpr const char* SCOREBOARD_DEFAULT_HOST = "flappy.serwe.dev";
inline constexpr unsigned short SCOREBOARD_DEFAULT_PORT = 80;
inline constexpr unsigned int SCOREBOARD_NAME_MAX = 12; // keep in sync with server/main.cpp
inline constexpr unsigned int SCOREBOARD_TOP_COUNT = 5;

#endif
