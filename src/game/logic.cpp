#include "logic.h"
#include "config.h"
#include "gamehelper.h"
#include "glm/geometric.hpp"

GameLogic::GameLogic(Render *render, Camera::RoomFollow2D *cam2D, Audio::Manager* audioManager)
{
     this->audio = audioManager;

     defaultCursor = Sprite(render->LoadTexture("textures/UI/cursor/default.png"));
     defaultCursor.depth = 5.1f;
     defaultCursor.rect.z *= 0.3f;
     defaultCursor.rect.w *= 0.3f;
     targetCursor = Sprite(render->LoadTexture("textures/UI/cursor/target.png"));
     targetCursor.depth = 5.1f;
     targetCursor.rect.z *= 0.3f;
     targetCursor.rect.w *= 0.3f;
     
     Resource::Font mapFont = render->LoadFont("textures/MedievalSharp-Regular.ttf");
          levels.push_back(
		      Level(render, "maps/intro", mapFont)
			   );
     levels.push_back(
		      Level(render, "maps/tut1", mapFont)
		      );
     levels.push_back(
		      Level(render, "maps/tut2", mapFont)
		      );
     levels.push_back(
		      Level(render, "maps/level0", mapFont)
		      );
     levels.push_back(
		      Level(render, "maps/level1", mapFont)
		      );
     levels.push_back(
		      Level(render, "maps/level2", mapFont)
		      );
     currentLevel = levels[currentLevelIndex];
     player = Player(
		     
		     );
     hero = Hero(
		 Sprite(render->LoadTexture("textures/characters/Robyn-anim.png")),
		 Sprite(render->LoadTexture("textures/UI/circle.png")),
		 audio
		   );
     enemy = Enemy(
		   Sprite(render->LoadTexture("textures/characters/enemy-anim.png")),
		   Sprite(render->LoadTexture("textures/UI/distracted.png")),
		   Sprite(render->LoadTexture("textures/UI/circle.png")),
		   Sprite(render->LoadTexture("textures/UI/search.png")),
		   audio);
     obstacle = Obstacle(Sprite(render->LoadTexture("textures/obstacle.png")), audio);
     stone = god::Stone(Sprite(render->LoadTexture("textures/spells/stone.png")));
     smoke = god::Smoke(render->LoadTexture("textures/spells/smoke.png"));
     gust = god::Gust(render->LoadTexture("textures/spells/wind.png"));
     checkpoint = Sprite(render->LoadTexture("textures/checkpoint_off.png"));
     checkpoint.depth = CHARACTER_DEPTH;
     spellControls = SpellControls(render);
     checkpointActive = render->LoadTexture("textures/checkpoint_on.png");

     pickupSprite = Sprite(render->LoadTexture("textures/spells/pickup.png"));
     pickupSprite.depth = CHARACTER_DEPTH - 0.005f;

     Sprite restartBtnSprite = Sprite(glm::vec2(10.0f, 10.0f),render->LoadTexture("textures/UI/restart.png"));
     restartBtnSprite.depth = 5.0f;
     restartBtnSprite.rect.z *= 0.3f;
     restartBtnSprite.rect.w *= 0.3f;
     Sprite restartBtnActiveSprite = Sprite(glm::vec2(10.0f, 10.0f),render->LoadTexture("textures/UI/restart_pressed.png"));
     restartBtnActiveSprite.depth = 5.0f;
     restartBtnActiveSprite.rect.z *= 0.3f;
     restartBtnActiveSprite.rect.w *= 0.3f;
     restartBtn = Button(restartBtnSprite, restartBtnActiveSprite, true);
     restartBtn.setInitialRect(restartBtnSprite.rect);

     gold = Sprite(render->LoadTexture("textures/gold.png"));
     gold.depth = CHARACTER_DEPTH - 0.01f;

     tree = Sprite(render->LoadTexture("textures/tree.png"));
				
     LoadMap(cam2D);
     currentAudio = game_music::EndOfLevel;
     audioManager->LoadAudioFile(game_music::EndOfLevel);
     audioManager->LoadAudioFile(game_music::Voiced);
     audioManager->LoadAudioFile(game_music::Voiceless);
     audioManager->LoadAudioFile(game_music::Victory);
     audioManager->LoadAudioFile(game_music::Hip);
     audioManager->LoadAudioFile(game_music::Menu);
     audioManager->LoadAudioFile(game_music::Lose);
     for( int i = 1; i < 4; i++)
	 audio->LoadAudioFile("audio/SFX/Box/Box" + std::to_string(i) + ".wav");
     for( int i = 1; i < 4; i++)
	 audio->LoadAudioFile("audio/SFX/Rock/Rock" + std::to_string(i) + ".wav");
     audio->LoadAudioFile("audio/SFX/Smoke Bomb.wav");
     audio->LoadAudioFile("audio/SFX/Thunder.wav");
     audio->LoadAudioFile("audio/SFX/Wind1.wav");
     audio->LoadAudioFile("audio/SFX/Wind2.wav");
}

void GameLogic::Update(glm::vec4 camRect, Timer &timer, Input &input, Camera::RoomFollow2D *cam2D, glm::vec2 mousePos)
{
    if(currentAudio == game_music::EndOfLevel)
    {
	if(!audio->Playing(currentAudio))
	{
	    switch(currentLevelIndex) {
	    case 0:
		currentAudio = game_music::Menu;
		break;
	    case 1:
	    case 2:
		currentAudio = game_music::Voiceless;
		break;
	    case 3:
	    case 4:
		currentAudio = game_music::Hip;
		break;
	    case 5:
		currentAudio = game_music::Voiced;
		break;
	    }
	    audio->Play(currentAudio, true, GAME_MUSIC_VOLUME);
	}
    }
    lastScale = camRect.z / settings::TARGET_WIDTH;
    std::vector<glm::vec4> frameColliders;
    if(currentLevelIndex != 0) {
	spellControls.Update(camRect, timer, input, mousePos);
	spellCast(spellControls.spellCast().first, spellControls.spellCast().second, cam2D);
    }
    currentLevel.Update(camRect, timer, &frameColliders);
    hero.Update(camRect, timer);
    if(hero.isFinished())
	levelComplete(cam2D);
    for(auto& t: trees)
	t.UpdateMatrix(camRect);
    for(auto& e: enemies)
    {
	e.Update(camRect, timer, gh::centre(hero.getHitBox()));
	if(gh::colliding(e.getHitBox(), hero.getHitBox()))
	    playerDeath(cam2D);
    }
    spellUpdate(camRect, timer);
    for(auto& o: obstacles)
    {
      o.Update(camRect, timer);
      if(gh::colliding(o.getHitBox(), hero.getHitBox()))
	  hero.setRectToPrev();
      for(auto& e: enemies)
	  if(!e.chasing() && gh::colliding(o.getHitBox(), e.getHitBox()))
            e.setRectToPrev();
    }
    for(auto& s: staticColliders)
    {
      if(gh::colliding(s, hero.getHitBox()))
	  hero.setRectToPrev();
      for(auto& e: enemies)
	  if(!e.chasing() && gh::colliding(s, e.getHitBox()))
	      e.setRectToPrev();
    }
    for(int i = 0; i < pickups.size(); i++)
    {
	pickups[i].second.UpdateMatrix(camRect);        ;
	if(gh::colliding(pickups[i].first.rect, hero.getHitBox()))
	{
	    std::vector<Spells> toAdd;

	    for(int s = 0; s < pickups[i].first.spells.size(); s++)
	    {
		for(int k = 0; k < pickups[i].first.spells[s].second; k++)
		    toAdd.push_back(pickups[i].first.spells[s].first);
	    }
	    spellControls.addCards(toAdd);
	    pickups.erase(pickups.begin() + i--);
	}
    }
    if(!gotGold)
    {
        gold.UpdateMatrix(camRect);
	if(gh::colliding(hero.getHitBox(), gold.rect))
	    gotGold = true;
    }
    for(int i = 0; i < checkpoints.size(); i++)
    {
	checkpoints[i].UpdateMatrix(camRect);
	if(lastCheckpoint != &checkpoints[i])
	    if (gh::colliding(checkpoints[i].rect, hero.getHitBox()))
	    {
		lastCheckpoint = &checkpoints[i];
		lastCheckpoint->texture = checkpointActive;
		checkpointTargetIndex = hero.getTargetIndex();
		checkpointPos = hero.getPos();
		checkpointSpells = spellControls.getSpells();
		checkpointObstacles.clear();
		for(int o = 0; o < obstacles.size(); o++)
		    checkpointObstacles.push_back(obstacles[o]);
		checkpointEnemies.clear();
		for(int c = 0; c < enemies.size(); c++)
		    checkpointEnemies.push_back(enemies[c]);
		checkpointPickups.clear();
		for(int p = 0; p < pickups.size(); p++)
		    checkpointPickups.push_back(pickups[p]);
		checkpointGotGold = gotGold;
	    }
    }
    if(spellControls.isTargeting())
    {
	targetCursor.rect.x = mousePos.x + camRect.x - targetCursor.rect.z / 2.0f;
	targetCursor.rect.y = mousePos.y + camRect.y - targetCursor.rect.w / 2.0f;
	targetCursor.UpdateMatrix(camRect);
	currentCursor = &targetCursor;
  }
    else
  {

      defaultCursor.rect.x = mousePos.x + camRect.x;
      defaultCursor.rect.y = mousePos.y + camRect.y;
      defaultCursor.UpdateMatrix(camRect);
      currentCursor = &defaultCursor;
  }
    if(currentLevelIndex !=0)
	restartBtn.Update(camRect, input, mousePos);
    if(restartBtn.Clicked())
	LoadMap(cam2D);
    prevInput = input;

    removeAudioTimer += timer.FrameElapsed();
    if(removeAudioTimer > removeAudioDelay)
    {
	removeAudioTimer = 0.0f;
	audio->RemovePlayed();
    }
}

void GameLogic::Draw(Render *render)
{
  currentLevel.Draw(render);
  for(auto& t: trees)
      t.Draw(render);
  hero.Draw(render);
  for(auto& s: stones)
    s.Draw(render);
  for(auto& o: obstacles)
      o.Draw(render);
  for(auto& pu: pickups)
      pu.second.Draw(render);
  if(!gotGold)
      gold.Draw(render);
  for(auto& c: checkpoints)
      c.Draw(render);
  for(auto& e: enemies)
      e.Draw(render);
  for(auto& s: smokes)
      s.Draw(render);
  for(auto& g: gusts)
      g.Draw(render);
  for(auto& e: enemies)
      e.DrawTransparent(render);
  if(currentLevelIndex != 0)
  {
      spellControls.Draw(render);
      if(cursorActive) currentCursor->Draw(render);
      restartBtn.Draw(render);
  }
}

glm::vec2 GameLogic::getTarget()
{
    auto target = hero.getPos();
    const float CARD_OFFSET = 200.0f;
    target.y += CARD_OFFSET * lastScale;
    return target;
}

void GameLogic::LoadMap(Camera::RoomFollow2D *cam2D)
{
  cam2D->setCameraMapRect(currentLevel.getMapRect());
  
  Level::MapGameplayObjects mapObjs = currentLevel.getObjLists();
  cam2D->setCameraRects(mapObjs.roomRects);
  hero.setPath(mapObjs.heroPath);
  stones.clear();
  smokes.clear();
  gusts.clear();
  enemies.clear();
  for(auto& p: mapObjs.enemyPaths)
  {
    Enemy e = enemy;
    e.setPath(p);
    enemies.push_back(e);
    }
  obstacles.clear();
  for(auto& o: mapObjs.obstacles)
  {
      Obstacle obs = obstacle;
      obs.setRect(o);
      obstacles.push_back(obs);
  }
  checkpoints.clear();
  lastCheckpoint = nullptr;
  const float CHECKPOINT_WIDTH = 200.0f;
  for(auto &c: mapObjs.checkpoints)
  {
      auto cp = checkpoint;
      float ratio = cp.rect.w / cp.rect.z;
      cp.rect.z = CHECKPOINT_WIDTH;
      cp.rect.w = CHECKPOINT_WIDTH * ratio;
      cp.rect.x = c.x + c.z/2.0 - cp.rect.z/2.0;
      cp.rect.y = c.y + c.w/2.0 - cp.rect.w/2.0;
      checkpoints.push_back(cp);
  }
  
  spellControls.setCards({Spells::Restart, Spells::Wait});
  pickups.clear();
  for(auto &pu: mapObjs.pickups)
  {
      Sprite puS = pickupSprite;
      float ratio = puS.rect.w / puS.rect.z;
      puS.rect.x = pu.rect.x;
      puS.rect.y = pu.rect.y;
      puS.rect.z = pu.rect.z;
      puS.rect.w = pu.rect.z * ratio;
      pickups.push_back(std::pair<Pickup, Sprite>(pu, puS));
  }
  float ratio = gold.rect.w / gold.rect.z;
  gold.rect = mapObjs.gold;
  gold.rect.w = gold.rect.z * ratio;
  gotGold = false;
  staticColliders = mapObjs.staticColliders;
  trees.clear();
  for(auto& t: mapObjs.trees)
  {
      Sprite tSprite = tree;
      tSprite.rect = glm::vec4(t.x - TREE_DIM.x/2.0, t.y - TREE_DIM.y*0.8f, TREE_DIM.x, TREE_DIM.y);
      tSprite.depth = CHARACTER_DEPTH - 0.001f;
      trees.push_back(tSprite);
  }
}

void GameLogic::playerDeath(Camera::RoomFollow2D *cam2D)
{
    glm::vec4 cp = lastCheckpoint == nullptr ? glm::vec4(0) : lastCheckpoint->rect;
    LoadMap(cam2D);
    if(cp != glm::vec4(0))
    {
	hero.Wait();
	hero.setCheckpoint(checkpointPos, checkpointTargetIndex);
	for(int i = 0; i < checkpointSpells.size(); i++)
	{
	    if(checkpointSpells[i] == Spells::Wait)
		checkpointSpells[i] = Spells::Go;
	}
	spellControls.setCards(checkpointSpells);
	obstacles.clear();
	for(int o = 0; o < checkpointObstacles.size(); o++)
	    obstacles.push_back(checkpointObstacles[o]);
	enemies.clear();
	for(int c = 0; c < checkpointEnemies.size(); c++)
	{
	    if(!checkpointEnemies[c].isChasable(hero.getPos()))
	       enemies.push_back(checkpointEnemies[c]);
	}
	pickups.clear();
	for(int p = 0; p < checkpointPickups.size(); p++)
	    pickups.push_back(checkpointPickups[p]);
	gotGold = checkpointGotGold;
    }
}

void GameLogic::levelComplete(Camera::RoomFollow2D *cam2D)
{
    currentLevelIndex++;
    if(currentLevelIndex<levels.size())
    {
	currentLevel = levels[currentLevelIndex];
	LoadMap(cam2D);
    }
    audio->Stop(currentAudio);
    currentAudio = game_music::EndOfLevel;
    if(currentLevelIndex != 1)
	audio->Play(currentAudio, false, GAME_MUSIC_VOLUME);
    cam2D->Target(hero.getPos());
}

void pushCharacter(glm::vec2 pos, Character* character, god::Gust *gust, Timer &timer)
{
    auto otherPos = character->getPos();
    float dist = glm::distance(pos, otherPos);
    if(dist < gust->getAOE())
    {
	dist = dist < 70.0f ? 70.0f : dist;
	character->push(glm::normalize((otherPos - pos))/(dist*dist*0.0001f), timer);
    }
}

void GameLogic::spellUpdate(glm::vec4 camRect, Timer &timer)
{
    for(int stoneI = 0; stoneI < stones.size(); stoneI++)
  {
      stones[stoneI].Update(camRect, timer);
      auto hit = stones[stoneI].hit();
      if(hit != glm::vec4(0))
      {
	  //stone makes sound for enemies
	  for(int i = 0; i < enemies.size(); i++)
	      enemies[i].soundEvent(gh::centre(hit));

	  bool wasHit = false;
	  //stone destroys box
	  for(int i = 0; i < obstacles.size(); i++)
	      if(gh::colliding(hit, obstacles[i].getHitBox()))
	      {
		  audio->Play("audio/SFX/Box/Box" + rand.stringNum(3) + ".wav", false, GAME_ROCK_BOX_VOLUME);		  obstacles.erase(obstacles.begin() + i--);
	      }
	  audio->Play("audio/SFX/Rock/Rock" + rand.stringNum(3) + ".wav", false, GAME_ROCK_BOX_VOLUME);
	  stones.erase(stones.begin() + stoneI--);
      }
  }
    for(int i = 0; i < smokes.size(); i++)
    {
	smokes[i].Update(camRect, timer);
	for(int j = 0; j < enemies.size(); j++)
	    enemies[j].smokeEvent(smokes[i].getHitBox());
	if(smokes[i].isFinished())
	{
	    smokes.erase(smokes.begin() + i--);
	}
    }
    for(int i = 0; i < gusts.size(); i++)
    {
	gusts[i].Update(camRect, timer);
	auto pos = gusts[i].getPos();
	pushCharacter(pos, &hero, &gusts[i], timer);
	for(int e = 0 ; e < enemies.size(); e++)
	{
	    pushCharacter(pos, &enemies[e], &gusts[i], timer);
	}

	if(gusts[i].isFinished())
	    gusts.erase(gusts.begin() + i--);
    }
}

void GameLogic::spellCast(Spells spell, glm::vec2 pos, Camera::RoomFollow2D* cam2D)
{
    god::Stone s = stone;
    god::Smoke smk = smoke;
    god::Gust gst = gust;
    switch (spell)
    {
    case Spells::Stone:
	s.setPos(pos);
	stones.push_back(s);
	break;
    case Spells::Wait:
	hero.Wait();
	break;
    case Spells::Go:
	hero.Go();
	break;
    case Spells::Restart:
	playerDeath(cam2D);
	break;
    case Spells::Smoke:
	smk.setPos(pos);
	smokes.push_back(smk);
	audio->Play("audio/SFX/Smoke Bomb.wav", false, GAME_SFX_VOLUME);
	break;
    case Spells::Wind:
	audio->Play("audio/SFX/Wind" + rand.stringNum(2)  + ".wav", false, GAME_SFX_VOLUME);
	gst.setPos(pos);
	gusts.push_back(gst);
  }

}
