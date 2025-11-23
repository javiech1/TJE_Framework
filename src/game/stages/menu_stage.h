#pragma once
#include "stage.h"
#include <vector>
#include <string>

class EntitySkybox;

class MenuStage : public Stage
{
public:
    MenuStage();
    virtual ~MenuStage();

    // Implement Stage methods
    void update(float delta_time) override;
    void render(Camera* camera) override;

    void onKeyDown(SDL_KeyboardEvent event) override;
    void onKeyUp(SDL_KeyboardEvent event) override;
    void onMouseMove(SDL_MouseMotionEvent event) override;

    EntityPlayer* getPlayer() const override { return nullptr; }  // Menu has no player

private:
    EntitySkybox* skybox;

    // Menu state
    enum MenuOption {
        MENU_START_GAME,
        MENU_OPTIONS,
        MENU_QUIT,
        MENU_COUNT
    };

    int selected_option;
    float menu_animation_time;

    // Camera rotation for animated background
    float camera_rotation;

    void renderMenuUI(Camera* camera);
};