#pragma once
#include "stage.h"

class World;
class LevelManager;

class PlayStage : public Stage
{
    public:
        PlayStage();
        virtual ~PlayStage();

        //implement Stage methods
        void update(float delta_time) override;
        void render(Camera* camera) override;

        void onKeyDown(SDL_KeyboardEvent event) override;
        void onKeyUp(SDL_KeyboardEvent event) override;
        void onMouseMove(SDL_MouseMotionEvent event) override;

        EntityPlayer* getPlayer() const override;

        // Level management
        void switchLevel(int index);
        void nextLevel();
        void previousLevel();
        int getCurrentLevelIndex() const { return current_level_index; }

    private:
        World* world = nullptr;
        LevelManager* level_manager = nullptr;
        int current_level_index = 0;
};
