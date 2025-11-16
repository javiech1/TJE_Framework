#include "play_stage.h"
#include "world.h"
#include "framework/utils.h"

PlayStage::PlayStage()
{
    world = new World();
}

PlayStage::~PlayStage()
{
    delete world;
}

void PlayStage::render(Camera* camera)
{
    world->render(camera);
}

void PlayStage::update(float delta_time)
{
    world->update(delta_time);
}

void PlayStage::onKeyDown(SDL_KeyboardEvent event)
{
    world->onKeyDown(event);
}

void PlayStage::onKeyUp(SDL_KeyboardEvent event)
{
    world->onKeyUp(event);
}

void PlayStage::onMouseMove(SDL_MouseMotionEvent event)
{
    world->onMouseMove(event);
}