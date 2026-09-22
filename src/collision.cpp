#include "collision.h"

bool isOutOfBounds(Bird const& bird, sf::Vector2u windowSize,
                   float floorOffset) noexcept {
  if (windowSize.x == 0 || windowSize.y == 0) {
    return false;
  }

  float const winTop = static_cast<float>(windowSize.y) / 2.0f;
  float const winBottom =
      -static_cast<float>(windowSize.y) / 2.0f + floorOffset;
  float const winLeft = -static_cast<float>(windowSize.x) / 2.0f;
  float const winRight = static_cast<float>(windowSize.x) / 2.0f;

  return (bird.pos.y + BIRD_RADIUS >= winTop) ||
         (bird.pos.y - BIRD_RADIUS <= winBottom) ||
         (bird.pos.x + BIRD_RADIUS <= winLeft) ||
         (bird.pos.x - BIRD_RADIUS >= winRight);
}

bool checkPipeCollision(Bird const& bird, Pipe const& pipe) noexcept {
  float const pipeLeft = pipe.xPos - PIPE_HALF_WIDTH;
  float const pipeRight = pipe.xPos + PIPE_HALF_WIDTH;

  // leave early if we aren't near pipe
  if (bird.pos.x + BIRD_RADIUS < pipeLeft ||
      bird.pos.x - BIRD_RADIUS > pipeRight) {
    return false;
  }

  float const upperPipeBottom = pipe.gapPos + pipe.gapHeight / 2.0f;
  constexpr float upperPipeTop = 100000.0f;

  float const lowerPipeTop = pipe.gapPos - pipe.gapHeight / 2.0f;
  constexpr float lowerPipeBottom = -100000.0f;

  if (checkCircleAABBCollision(bird.pos, BIRD_RADIUS, pipeLeft, pipeRight,
                               upperPipeBottom, upperPipeTop)) {
    return true;
  }

  if (checkCircleAABBCollision(bird.pos, BIRD_RADIUS, pipeLeft, pipeRight,
                               lowerPipeBottom, lowerPipeTop)) {
    return true;
  }

  return false;
}

bool checkCollisions(GameState const& game, sf::Vector2u windowSize) noexcept {
  if (isOutOfBounds(game.bird, windowSize)) {
    return true;
  }

  for (auto const& pipe : game.pipes) {
    if (checkPipeCollision(game.bird, pipe)) {
      return true;
    }
  }

  return false;
}
