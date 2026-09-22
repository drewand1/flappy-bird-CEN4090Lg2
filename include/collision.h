#ifndef COLLISION_H
#define COLLISION_H

#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include "gamelogic.h"
#include "gameconfig.h"

[[nodiscard]] constexpr bool checkCircleAABBCollision(
    sf::Vector2f center,
    float radius,
    float left,
    float right,
    float bottom,
    float top) noexcept {
    const float closestX = std::clamp(center.x, left, right);
    const float closestY = std::clamp(center.y, bottom, top);
    const float dx = center.x - closestX;
    const float dy = center.y - closestY;
    return (dx * dx + dy * dy) < (radius * radius);
}

[[nodiscard]] bool isOutOfBounds(
    const Bird& bird,
    sf::Vector2u windowSize,
    float floorOffset = 0.0f) noexcept;

[[nodiscard]] bool checkPipeCollision(
    const Bird& bird,
    const Pipe& pipe) noexcept;

[[nodiscard]] bool checkCollisions(
    const GameState& game,
    sf::Vector2u windowSize) noexcept;

#endif
