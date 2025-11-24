#pragma once

#include "framework/camera.h"
#include "framework/input.h"
#include "framework/extra/bass.h"  // For HCHANNEL type
#include "../levels/level_config.h"
#include <vector>

class Entity;
class EntityPlayer;
class EntitySkybox;
class Camera;
class EntityOrb;
class EntityResetSlab;
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

        // Level management
        void loadLevel(const LevelConfig& config);
        void clearLevel();
        void initTutorial(); //nivel tutorial
        void initEmpty();    //empty level for placeholders
        void reset();  // Reset world to initial state

        // Gravity system
        float getGravity() const { return gravity_value; }
        void setGravity(float g) { gravity_value = g; }

        // Skybox access
        EntitySkybox* getSkybox() const { return skybox; }
        void setSkybox(EntitySkybox* sb) { skybox = sb; }

    private:
        std::vector<Entity*> entities;
        std::vector<EntityOrb*> orbs;
        std::vector<EntityResetSlab*> reset_slabs;
        EntityPlayer* player = nullptr;
        EntitySkybox* skybox = nullptr;

        float gravity_value = 9.8f;  // Default Earth gravity
        int orbs_collected = 0;
        HCHANNEL music_channel = 0;  // Background music channel

        LevelConfig current_config;  // Store current level configuration
};
