#ifndef GAMECONFIG_H
#define GAMECONFIG_H

inline constexpr float GRAVITY_CONST = 400.0f;
inline constexpr float BIRD_JUMP_VEL = 200.0f;
inline constexpr float PIPE_BASE_SPEED = 200.0f;
inline constexpr float PIPE_BASE_SPAWN_COOLDOWN = 5.0f;
inline constexpr float PIPE_BASE_GAP_SIZE = 200.0f;

inline constexpr float PIPE_WIDTH = 64.0f;
inline constexpr float PIPE_HALF_WIDTH = PIPE_WIDTH / 2.0f;

inline constexpr float BIRD_WIDTH = 32.0f;
inline constexpr float BIRD_HEIGHT = 32.0f;
inline constexpr float BIRD_RADIUS = 12.0f;

#endif