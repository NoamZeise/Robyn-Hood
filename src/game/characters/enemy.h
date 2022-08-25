#ifndef GAME_ENEMY_H
#define GAME_ENEMY_H

#include "character.h"
#include "glm/geometric.hpp"

namespace
{    
    const float DISTRACTION_RANGE = 140.0f;
    const float INVESTIGATION_INITIATION_PROXIMITY = 10.0f;
    const float INVESTIGATION_DURATION = 2200.0f;
    const float PLAYER_CHASE_RADIUS = 100.0f;
    const float ENEMY_PATROL_SPEED = 0.025f;
    const float ENEMY_INVESTIGATE_SPEED = 0.03f;
    const float ENEMY_CHASE_SPEED = 0.12f;
} // namespace

class Enemy : public Character
{
 public:
  Enemy() {}
    Enemy(Sprite sprite, Sprite distracted) : Character(sprite)
  {
      speed = ENEMY_PATROL_SPEED;
      distracted.rect.z *= 0.1f;
      distracted.rect.w *= 0.1f;
      distracted.depth = 2.0f;
      this->distracted = distracted;
  }
    void setRectToPrev() override
    {
	this->dir *= -1;
	this->currentTargetIndex += this->dir;
	if(currentTargetIndex < 0)
	    currentTargetIndex = 0;
	if(currentTargetIndex >= this->path.size())
	    currentTargetIndex = (int)this->path.size() - 1;
	sprite.rect = prevRect;
    }

    void Update(glm::vec4 camRect, Timer &timer, glm::vec2 playerPos)
    {
	prevRect = sprite.rect;
	if(currentState == EnemyState::Patrol)
	{
	    speed = ENEMY_PATROL_SPEED;
	    Character::Update(camRect, timer);
	}
	else if(currentState == EnemyState::Investigate)
	{
	    speed = ENEMY_INVESTIGATE_SPEED;
	    glm::vec2 toTarget = investigationTarget - gh::centre(sprite.rect);
	    float length = glm::length(toTarget);
	    moveToTarget(toTarget, timer);
	    if(length < INVESTIGATION_INITIATION_PROXIMITY)
		investigationTimer += timer.FrameElapsed();
	    if(investigationTimer > investigationDuration)
	    {
		currentState = EnemyState::Patrol;
	    }
	    sprite.UpdateMatrix(camRect);
	}
	else if(currentState == EnemyState::Chase)
	{
	    speed = ENEMY_CHASE_SPEED;
	    glm::vec2 toTarget = playerPos - gh::centre(sprite.rect);
	    moveToTarget(toTarget, timer);
	    sprite.UpdateMatrix(camRect);
	}

	if(glm::distance(gh::centre(sprite.rect), playerPos) < PLAYER_CHASE_RADIUS)
	{
	    currentState = EnemyState::Chase;
	}
	distracted.rect.x = sprite.rect.x + (sprite.rect.z - distracted.rect.z)/2.0f;
	distracted.rect.y = sprite.rect.y - distracted.rect.w;
	distracted.UpdateMatrix(camRect);
    }

    void soundEvent(glm::vec2 sound)
    {
	if(currentState != EnemyState::Chase)
	{

	    if(glm::distance(gh::centre(sprite.rect), sound) < DISTRACTION_RANGE)
		{
		    currentState = EnemyState::Investigate;
		    investigationTarget = sound;
		    investigationTimer = 0.0f;
		}
	}

    }

    void Draw(Render *render) override
    {
	Character::Draw(render);
	if(currentState == EnemyState::Investigate)
	    distracted.Draw(render);
    }

private:

    enum EnemyState
	{
	    Patrol,
	    Investigate,
	    Chase,
	    Dead,
	};

    Sprite distracted;
    EnemyState currentState = EnemyState::Patrol;
    glm::vec2 investigationTarget = glm::vec2(0);
    float investigationTimer = 0.0f;
    float investigationDuration = INVESTIGATION_DURATION;
};

#endif
