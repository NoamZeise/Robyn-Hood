#ifndef GAME_SPELL_CARD
#define GAME_SPELL_CARD

#include "../button.h"
#include "config.h"
#include "glm/geometric.hpp"
#include <timer.h>
#include <input.h>

enum class Spells
{
  None,
  Stone,
  Wait,
  Wind,
};

namespace{
const float RAISE_CARD_DIST = 30.0f;
const float CARD_FLOATINESS = 100.0f;
const float CARD_TRANSPARENCY_RANGE = 300.0f;
const float CARD_ACTIATION_THREASHOLD = 0.5f;
} // namespace

class SpellCard : public Button
{
 public:
    SpellCard() {}
    SpellCard(Sprite card, Spells spell) : Button(card, true)
    {
	this->spell = spell;
	this->sprite.depth = 2.0f;
    }

  void Update(glm::vec4 camRect, Timer &timer, Input &input, glm::vec2 mousePos)
  {
    Button::Update(camRect, input, mousePos);
    cast = false;
    glm::vec2 toTarget = target - glm::vec2(initialRect.x, initialRect.y);
    toTarget /= CARD_FLOATINESS;
    toTarget *= timer.FrameElapsed();
    initialRect.x += toTarget.x;
    initialRect.y += toTarget.y;
    if(selected)
      {
	if(!clicked)
	  {
	  selected = false;
	  if(prevDist < CARD_ACTIATION_THREASHOLD)
	    {
	      cast = true;
	    }
	   
	  target.x = originTarget.x;
	  target.y = originTarget.y;
	  
	  }
	else
	  {
	    sprite.spriteColour = SELECT_COLOUR;
	    float dist = originTarget.y - target.y;
	    sprite.spriteColour.w = 1 - (dist / CARD_TRANSPARENCY_RANGE);
	    prevDist = sprite.spriteColour.w;
	    if(sprite.spriteColour.w < 0.1f) { sprite.spriteColour.w = 0.1f; }
	    spellTarget = glm::vec2(mousePos.x + camRect.x, mousePos.y + camRect.y);
	    float scale = settings::TARGET_WIDTH / camRect.z;
	    target.x = mousePos.x*scale - initialRect.z/2.0f;
	    target.y = mousePos.y*scale - initialRect.w/2.0f;
	  }
      }
    else
      {
	if(Clicked())
	  {
	  selected = true;
	  originTarget = target;
	  }
	else
	  {
	    if(onSprite && !wasOnSprite)
	      {
		wasOnSprite = true;
		target.y -= RAISE_CARD_DIST;
		sprite.depth += 0.1f;
	      }
	    else if(!onSprite && wasOnSprite)
	      {
		wasOnSprite = false;
		target.y += RAISE_CARD_DIST;
		sprite.depth -= 0.1f;
	      }
	}
      }
  }

  bool wasCast()
  {
    return cast;
  }
  
  glm::vec2 getTarget()
    {
      return spellTarget;
    }

    glm::vec2 getOriginTarget()
    {
	return originTarget;
    }

    bool isSelected()
    {
	return selected && prevDist < CARD_ACTIATION_THREASHOLD && spell != Spells::Wait;
    }

  Spells getSpell() { return spell; }

  void setInitialRect(glm::vec4 rect) override
  {
    initialRect = rect;
    target = glm::vec2(initialRect.x, initialRect.y);
  }

    void setInitialPos(glm::vec2 pos)
  {
    initialRect.x = pos.x;
    initialRect.y = pos.y;
    target = glm::vec2(initialRect.x, initialRect.y);
  }

  void setTarget(glm::vec2 target)
  {
      this->target = target;
  }

 private:
  bool selected = false;
  bool cast = false;
  bool wasOnSprite = false;
  glm::vec2 originTarget;
  Spells spell;
  float prevDist = 1.0f;
  glm::vec2 target;
  glm::vec2 spellTarget;
};

#endif
