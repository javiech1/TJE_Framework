#include "play_stage.h"
#include "menu_stage.h"
#include "game/world/world.h"
#include "game/levels/level_manager.h"
#include "game/game.h"
#include "framework/entities/entity_skybox.h"
#include "framework/utils.h"
#include <iostream>

PlayStage::PlayStage()
{
    world = new World();
    level_manager = new LevelManager();

    // Load the first level (tutorial)
    current_level_index = 0;
    switchLevel(0);

    // Lock mouse for gameplay
    if (Game::instance) {
        Game::instance->setMouseLocked(true);
    }
}

PlayStage::~PlayStage()
{
    delete world;
    delete level_manager;
}

void PlayStage::render(Camera* camera)
{
    // Render skybox first (behind everything)
    EntitySkybox* skybox = world->getSkybox();
    if (skybox && skybox->texture) {
        skybox->render(camera);
    }

    // Then render world entities
    world->render(camera);
}

void PlayStage::update(float delta_time)
{
    world->update(delta_time);
}

void PlayStage::onKeyDown(SDL_KeyboardEvent event)
{
    // Handle level switching
    switch(event.keysym.sym)
    {
        case SDLK_ESCAPE:
            // Return to menu
            std::cout << "Returning to main menu..." << std::endl;
            Game::instance->setStage(new MenuStage());
            return;  // Don't process any more input

        case SDLK_1: switchLevel(0); break;  // Level 1 (Tutorial)
        case SDLK_2: switchLevel(1); break;  // Level 2 (Low gravity)
        case SDLK_3: switchLevel(2); break;  // Level 3 (High gravity)
        case SDLK_n: nextLevel(); break;      // Next level
        case SDLK_p: previousLevel(); break;  // Previous level
    }

    // Pass to world for other input
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

EntityPlayer* PlayStage::getPlayer() const
{
    return world ? world->getPlayer() : nullptr;
}

void PlayStage::switchLevel(int index)
{
    // Validate index
    if (index < 0 || index >= level_manager->getLevelCount()) {
        std::cout << "Invalid level index: " << index << std::endl;
        return;
    }

    // Get the level config
    LevelConfig* config = level_manager->getLevel(index);
    if (!config) {
        std::cout << "Failed to get level config for index: " << index << std::endl;
        return;
    }

    // Load the level
    current_level_index = index;
    world->loadLevel(*config);

    // Display level info
    std::cout << "\n===========================================" << std::endl;
    std::cout << "Level " << (index + 1) << ": " << config->name << std::endl;
    std::cout << "Gravity: " << config->gravity << " m/s²" << std::endl;
    if (config->gravity < 9.8f) {
        std::cout << "Low gravity - Jump higher, fall slower!" << std::endl;
    } else if (config->gravity > 9.8f) {
        std::cout << "High gravity - Jump lower, fall faster!" << std::endl;
    }
    std::cout << "Controls: WASD to move, SPACE to jump" << std::endl;
    std::cout << "Level switching: 1-3 for direct, N/P for next/prev" << std::endl;
    std::cout << "==========================================\n" << std::endl;
}

void PlayStage::nextLevel()
{
    int next = current_level_index + 1;
    if (next >= level_manager->getLevelCount()) {
        next = 0; // Wrap around to first level
    }
    switchLevel(next);
}

void PlayStage::previousLevel()
{
    int prev = current_level_index - 1;
    if (prev < 0) {
        prev = level_manager->getLevelCount() - 1; // Wrap to last level
    }
    switchLevel(prev);
}
