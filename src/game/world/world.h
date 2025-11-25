#pragma once

#include "framework/camera.h"
#include "framework/input.h"
#include "../levels/level_config.h"
#include <vector>

class Entity;
class EntityPlayer;
class EntitySkybox;
class EntityPlatform;
class Camera;
class EntityOrb;
class EntityResetSlab;
class EntityObstacle;

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

        void loadLevel(const LevelConfig& config);
        void clearLevel();
        void initEmpty();
        void reset();
        void fullReset();

        float getGravity() const { return gravity_value; }
        void setGravity(float g) { gravity_value = g; }

        EntitySkybox* getSkybox() const { return skybox; }
        void setSkybox(EntitySkybox* sb) { skybox = sb; }

    private:
        std::vector<Entity*> entities;
        std::vector<EntityOrb*> orbs;
        std::vector<EntityResetSlab*> reset_slabs;
        std::vector<EntityObstacle*> obstacles;
        std::vector<EntityPlatform*> twin_platforms;
        EntityPlayer* player = nullptr;
        EntitySkybox* skybox = nullptr;

        float gravity_value = 9.8f;
        int orbs_collected = 0;

        LevelConfig current_config;
        Vector3 player_start;
        Vector3 last_checkpoint;

        void toggleTwinPlatforms();
};
