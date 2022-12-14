#include "player.h"
#include "gamehelper.h"


Player::Player(Sprite sprite)
{
  this->sprite = sprite;
  this->sprite.rect.z *= 0.25f;
  this->sprite.rect.w *= 0.25f;
  this->sprite.depth = 0.05f;
  this->hitbox = this->sprite.rect;
}

void Player::Update(glm::vec4 cameraRect, Timer &timer, Input &input)
{
  glm::vec2 moveVector = this->movementUpdate(input);
  moveVector *= timer.FrameElapsed();
  gh::addVec2ToRect(moveVector, &this->hitbox);
  gh::addVec2ToRect(moveVector, &this->sprite.rect);
  this->sprite.UpdateMatrix(cameraRect);
}

void Player::Draw(Render *render)
{
  sprite.Draw(render);
}

glm::vec2 Player::movementUpdate(Input &input)
{
  glm::vec2 move = glm::vec2(0.0f);

  if(input.Keys[GLFW_KEY_W])
    move.y -= speed;
  if(input.Keys[GLFW_KEY_A])
    move.x -= speed;
  if(input.Keys[GLFW_KEY_S])
    move.y += speed;
  if(input.Keys[GLFW_KEY_D])
    move.x += speed;

  return move;
}
