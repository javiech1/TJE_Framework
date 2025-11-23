#include "menu_stage.h"
#include "play_stage.h"
#include "game/game.h"
#include "framework/entities/entity_skybox.h"
#include "framework/utils.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/texture.h"
#include "framework/audio.h"
#include <cmath>
#include <iostream>

MenuStage::MenuStage()
{
    selected_option = MENU_START_GAME;
    menu_animation_time = 0.0f;
    camera_rotation = 0.0f;

    // Create skybox for menu background
    skybox = new EntitySkybox();
    skybox->mesh = Mesh::Get("data/meshes/cubemap.ASE");
    skybox->shader = Shader::Get("data/shaders/skybox.vs", "data/shaders/skybox.fs");

    // Try different skybox textures - using the same one as in-game for consistency
    const char* skyboxTextures[] = {
        "data/textures/skybox1.png",
        "data/textures/skybox2.png",
        "data/textures/skybox3.png",
        "data/textures/skybox.png"
    };

    for (const char* texPath : skyboxTextures) {
        skybox->texture = Texture::Get(texPath, false);
        if (skybox->texture) {
            std::cout << "Menu skybox texture loaded: " << texPath << std::endl;
            break;
        }
    }

    // Play menu music if audio system is available
    // The Audio system handles its own initialization check internally
    // Could play menu music here
    // Audio::Play("data/audio/menu_music.mp3", 0.3f, BASS_SAMPLE_LOOP);

    // Unlock mouse for menu navigation
    if (Game::instance) {
        Game::instance->setMouseLocked(false);
    }
}

MenuStage::~MenuStage()
{
    delete skybox;
}

void MenuStage::update(float delta_time)
{
    menu_animation_time += delta_time;

    // Slowly rotate camera around skybox for animated background
    camera_rotation += delta_time * 0.1f;  // Slow rotation

    // Update camera position for rotating view
    Camera* camera = Game::instance->camera;
    if (camera) {
        float radius = 5.0f;
        float height = 2.0f;
        camera->lookAt(
            Vector3(sin(camera_rotation) * radius, height, cos(camera_rotation) * radius),  // Eye position
            Vector3(0.0f, height, 0.0f),  // Look at center
            Vector3(0.0f, 1.0f, 0.0f)   // Up vector
        );
    }
}

void MenuStage::render(Camera* camera)
{
    // Render skybox background
    if (skybox) {
        skybox->render(camera);
    }

    // Render menu UI on top
    renderMenuUI(camera);
}

void MenuStage::renderMenuUI(Camera* camera)
{
    int window_width = Game::instance->window_width;
    int window_height = Game::instance->window_height;

    // Title
    float titleScale = 4.0f;
    Vector3 titleColor(1.0f, 1.0f, 1.0f);
    std::string title = "PLATFORM JUMPER";
    int titleX = window_width / 2 - (title.length() * 8 * titleScale) / 2;  // Center text
    int titleY = window_height / 4;
    drawText(titleX, titleY, title, titleColor, titleScale);

    // Menu options
    std::vector<std::string> options = {
        "START GAME",
        "OPTIONS",
        "QUIT"
    };

    float optionScale = 2.0f;
    int optionStartY = window_height / 2;
    int optionSpacing = 50;

    for (int i = 0; i < options.size(); i++) {
        Vector3 color;
        float scale = optionScale;

        if (i == selected_option) {
            // Highlight selected option with animation
            float pulse = sin(menu_animation_time * 5.0f) * 0.1f + 1.0f;
            scale *= pulse;
            color = Vector3(1.0f, 1.0f, 0.0f);  // Yellow for selected

            // Add arrow indicator
            std::string arrow = "> ";
            int arrowX = window_width / 2 - (options[i].length() * 8 * scale) / 2 - 30;
            drawText(arrowX, optionStartY + i * optionSpacing, arrow, color, scale);
        } else {
            color = Vector3(0.7f, 0.7f, 0.7f);  // Gray for unselected
        }

        int optionX = window_width / 2 - (options[i].length() * 8 * scale) / 2;
        drawText(optionX, optionStartY + i * optionSpacing, options[i], color, scale);
    }

    // Instructions
    Vector3 instructionColor(0.5f, 0.5f, 0.5f);
    drawText(window_width / 2 - 200, window_height - 100, "Use UP/DOWN arrows to navigate", instructionColor, 1.5f);
    drawText(window_width / 2 - 150, window_height - 60, "Press ENTER to select", instructionColor, 1.5f);
}

void MenuStage::onKeyDown(SDL_KeyboardEvent event)
{
    switch(event.keysym.sym) {
        case SDLK_UP:
        case SDLK_w:
            selected_option--;
            if (selected_option < 0) {
                selected_option = MENU_COUNT - 1;
            }
            // Could play menu sound effect here
            break;

        case SDLK_DOWN:
        case SDLK_s:
            selected_option++;
            if (selected_option >= MENU_COUNT) {
                selected_option = 0;
            }
            // Could play menu sound effect here
            break;

        case SDLK_RETURN:
        case SDLK_SPACE:
            switch(selected_option) {
                case MENU_START_GAME:
                    std::cout << "Starting game..." << std::endl;
                    // Transition to play stage
                    Game::instance->setStage(new PlayStage());
                    break;

                case MENU_OPTIONS:
                    std::cout << "Options not implemented yet" << std::endl;
                    // TODO: Could create an OptionsStage
                    break;

                case MENU_QUIT:
                    std::cout << "Quitting game..." << std::endl;
                    Game::instance->must_exit = true;
                    break;
            }
            break;

        case SDLK_ESCAPE:
            // ESC returns to menu from any submenu, or quits from main menu
            if (selected_option == MENU_QUIT) {
                Game::instance->must_exit = true;
            }
            break;
    }
}

void MenuStage::onKeyUp(SDL_KeyboardEvent event)
{
    // No key up handling needed for menu
}

void MenuStage::onMouseMove(SDL_MouseMotionEvent event)
{
    // Could add mouse hover detection for menu options here
}