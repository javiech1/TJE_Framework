#pragma once

#include "framework/camera.h"
#include "framework/input.h"
#include <vector>

class Entity;
class EntityPlayer;
class Camera;
class EntityOrb;
class World
{
    public:
        World();
        ~World();

        void render(Camera* camera);
        void update(float delta_time);

        void onKeyDown(SDL_KeyboardEvent event);
        void onKeyUp(SDL_KeyboardEvent event);
        void onMouseMove(SDL_MouseMotionEvent event);
        Vector3 getPlayerPosition() const;
        float getPlayerScale() const;
        EntityPlayer* getPlayer() const { return player; }

    private:
        std::vector<Entity*> entities;
        std::vector<EntityOrb*> orbs;
        EntityPlayer* player = nullptr;
        int orbs_collected = 0;
};
