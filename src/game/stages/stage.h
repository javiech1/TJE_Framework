#pragma once

#include "framework/camera.h"
#include "framework/input.h"
#include "game/entities/entity_player.h"

class Stage
{
public:
    Stage() {}
    virtual ~Stage() {}
    virtual void update(float delta_time) = 0;
    virtual void render(Camera* camera) = 0;

    virtual void onKeyDown(SDL_KeyboardEvent event) = 0;
    virtual void onKeyUp(SDL_KeyboardEvent event) = 0;
    virtual void onMouseMove(SDL_MouseMotionEvent event) = 0;
    virtual EntityPlayer* getPlayer() const = 0;
};
