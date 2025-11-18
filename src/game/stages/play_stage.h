#pragma once
#include "stage.h"

class World;

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
    
    private:
        World* world = nullptr;
};
