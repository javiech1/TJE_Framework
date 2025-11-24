#include "game/world/world.h"
#include "framework/entities/entity.h"
#include "framework/entities/entity_skybox.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "game/entities/entity_player.h"
#include "game/entities/entity_platform.h"
#include "game/entities/entity_orb.h"
#include "game/entities/entity_reset_slab.h"
#include "framework/utils.h"
#include "framework/audio.h"
#include <iostream>

World::World()
{
    //create player entity
    EntityPlayer* player = new EntityPlayer();
    player->mesh = Mesh::Get("data/meshes/arachnoid.obj");
    player->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    player->texture = Texture::Get("data/textures/arachnoid.png");
    const float scale = 0.4f;  // Increased scale for better visibility with closer camera
    player->setScale(scale);
    // Start player at proper height on platform
    player->setPosition(Vector3(0.0f, 1.0f, 0.0f));
    player->setWorld(this);  // Connect player to world for gravity
    this->player = player;
    entities.push_back(player);

    // Create default skybox
    skybox = new EntitySkybox();
    skybox->mesh = Mesh::Get("data/meshes/cubemap.ASE");
    skybox->shader = Shader::Get("data/shaders/skybox.vs", "data/shaders/skybox.fs");

    // Load space skybox cubemap (standard OpenGL mapping)
    skybox->texture = new Texture();
    std::vector<std::string> faces = {
        "data/sky/px.png",   // right (+X)
        "data/sky/nx.png",   // left (-X)
        "data/sky/py.png",   // top (+Y)
        "data/sky/ny.png",   // bottom (-Y)
        "data/sky/pz.png",   // front (+Z)
        "data/sky/nz.png"    // back (-Z)
    };
    skybox->texture->loadCubemap("space_skybox", faces);

    // Debug output
    if (skybox->texture->texture_id == 0) {
        std::cout << "ERROR: Space skybox texture failed to load!" << std::endl;
    } else {
        std::cout << "Space skybox loaded successfully. ID: " << skybox->texture->texture_id << std::endl;
    }
}

World::~World()
{
    // Stop background music if playing
    if (music_channel) {
        Audio::Stop(music_channel);
        music_channel = 0;
    }

    // Clean up skybox
    if (skybox) {
        delete skybox;
        skybox = nullptr;
    }

    for( Entity* entity : entities )
    {
        delete entity;
    }
    entities.clear();
}

void World::render(Camera* camera)
{
    for (Entity* entity : entities)
    {
        entity->render(camera);
    }

    // Render reset slabs (semi-transparent boundaries)
    for (EntityResetSlab* slab : reset_slabs)
    {
        slab->render(camera);
    }

    // Render orbs (collectibles)
    for (EntityOrb* orb : orbs)
    {
        orb->render(camera);
    }

    // UI Text - orb counter (now 4 orbs in tutorial)
    drawText(10, 35, "Orbs collected: " + std::to_string(orbs_collected) + "/" + std::to_string(orbs.size()), Vector3(1,1,1), 2);

    // Tutorial instructions based on player position (Z-axis progression)
    if (player) {
        float playerZ = player->getPosition().z;
        Vector3 textColor(1.0f, 1.0f, 0.0f); // Yellow for instructions

        if (playerZ > -15.0f) {
            // Zone 1: Movement
            drawText(300, 200, "Use WASD to move around", textColor, 2);
            drawText(280, 230, "Walk to the edge and continue", Vector3(0.8f, 0.8f, 0.8f), 1.5f);
        }
        else if (playerZ > -37.0f) {
            // Zone 2: Basic jumping (ends at Z=-34)
            drawText(300, 200, "Press SPACE to jump", textColor, 2);
            drawText(260, 230, "Jump between the green platforms", Vector3(0.8f, 0.8f, 0.8f), 1.5f);
        }
        else if (playerZ > -58.0f) {
            // Zone 3: Vertical climbing (ends at Z=-56)
            drawText(280, 200, "Jump upward to climb", textColor, 2);
            drawText(260, 230, "Follow the yellow platforms up", Vector3(0.8f, 0.8f, 0.8f), 1.5f);
        }
        else if (playerZ > -79.0f) {
            // Zone 4: Precision (ends at Z=-77)
            drawText(300, 200, "Precision jumps!", textColor, 2);
            drawText(240, 230, "These platforms are smaller!", Vector3(1.0f, 0.5f, 0.2f), 1.5f);
        }
        else if (playerZ > -87.0f) {
            // Zone 5: Victory approach (ends at Z=-85)
            drawText(280, 200, "Final jump to victory!", Vector3(1.0f, 0.9f, 0.2f), 2);
            drawText(260, 230, "Jump to the gold platform!", Vector3(0.8f, 0.8f, 0.8f), 1.5f);
        }
        else {
            // Completed!
            drawText(280, 200, "Tutorial Complete!", Vector3(0.2f, 1.0f, 0.2f), 3);
            drawText(260, 240, "Press R to restart", Vector3(0.8f, 0.8f, 0.8f), 2);
        }
    }

    // Always show controls at bottom
    drawText(10, 560, "Controls: WASD=Move  SPACE=Jump  R=Reset", Vector3(0.7f, 0.7f, 0.7f), 1.5f);

}

void World::update(float delta_time)
{
    // SIMPLE UPDATE ORDER:

    // 1. Update all entities (handle input and movement)
    for (Entity* entity : entities)
    {
        entity->update(delta_time);
    }

    // Update reset slabs (for pulsing animation)
    for (EntityResetSlab* slab : reset_slabs)
    {
        slab->update(delta_time);
    }

    // 2. Check collisions FIRST to establish grounded state
    player->checkCollisions(entities);

    // 3. Apply physics AFTER collision (so is_grounded is correct)
    player->applyPhysics(delta_time);

    // Check if player has fallen below the world
    if (player->getPosition().y < 0.0f) {
        std::cout << "Player fell! Restarting level..." << std::endl;
        reset();
        return; // Skip rest of update this frame
    }

    // Check reset slab collisions
    for (EntityResetSlab* slab : reset_slabs) {
        if (slab->collidesWithPlayer(player->getPosition(), player->getScale() * 0.5f)) {
            std::cout << "Hit reset slab! Restarting level..." << std::endl;
            reset();
            return; // Skip rest of update this frame
        }
    }

    // 4. Check orb collection
    for (EntityOrb* orb : orbs) {
        if(!orb->getIsCollected()) {
            float distance = player->distance(orb);
            if (distance < 1.0f) {
                orb->collect();
                orbs_collected++;
                //TODO: add effect or smth
                std::cout << "Orb collected!" << std::endl;
            }
        }
    }
}

void World::onKeyDown(SDL_KeyboardEvent event)
{
    if (event.keysym.sym == SDLK_r) {
        reset();
    }
}

void World::onKeyUp(SDL_KeyboardEvent event)
{ 

}

void World::onMouseMove(SDL_MouseMotionEvent event)
{

}

Vector3 World::getPlayerPosition() const {
    return player ? player->getPosition() : Vector3();
}

float World::getPlayerScale() const {
    return player ? player->getScale() : 1.0f;
}

void World::initEmpty()
{
    // Empty level - no platforms or orbs
    // Just the player in empty space
    std::cout << "Loaded empty level - Gravity: " << gravity_value << std::endl;
}

void World::clearLevel()
{
    // Stop background music
    if (music_channel) {
        Audio::Stop(music_channel);
        music_channel = 0;
    }

    // Clear all entities except player
    for (auto it = entities.begin(); it != entities.end(); ) {
        if (*it != player) {
            delete *it;
            it = entities.erase(it);
        } else {
            ++it;
        }
    }

    // Clear orbs and reset slabs
    for (EntityOrb* orb : orbs) {
        delete orb;
    }
    orbs.clear();

    for (EntityResetSlab* slab : reset_slabs) {
        delete slab;
    }
    reset_slabs.clear();

    orbs_collected = 0;
}

void World::loadLevel(const LevelConfig& config)
{
    // Store config for reset functionality
    current_config = config;

    // Clear existing level
    clearLevel();

    // Apply level settings
    gravity_value = config.gravity;

    // Set player position
    if (player) {
        player->setPosition(config.player_start_position);
        player->resetVelocity();
    }

    // Handle different level types
    if (config.type == LevelConfig::EMPTY) {
        initEmpty();
    }
    else if (config.type == LevelConfig::DATA) {
        // Load platforms from config
        for (const auto& plat_def : config.platforms) {
            EntityPlatform* platform = new EntityPlatform();
            platform->mesh = Mesh::Get("data/meshes/box.ASE");
            platform->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
            platform->setPosition(plat_def.position);
            platform->setScale(plat_def.scale);
            platform->color = plat_def.color;
            entities.push_back(platform);
        }

        // Load orbs from config
        for (const auto& orb_def : config.orbs) {
            EntityOrb* orb = new EntityOrb();
            orb->setPosition(orb_def.position);
            // Note: EntityOrb handles its own mesh, shader, scale, and color internally
            orbs.push_back(orb);
        }

        // Load reset slabs from config
        for (const auto& slab_def : config.reset_slabs) {
            EntityResetSlab* slab = new EntityResetSlab();
            slab->mesh = Mesh::Get("data/meshes/box.ASE");
            slab->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
            slab->setPosition(slab_def.position);
            slab->setScale(slab_def.scale);
            slab->color = slab_def.color;
            reset_slabs.push_back(slab);
        }

        std::cout << "Loaded level from data file" << std::endl;
        std::cout << "  Platforms: " << config.platforms.size() << std::endl;
        std::cout << "  Orbs: " << config.orbs.size() << std::endl;
        std::cout << "  Reset Slabs: " << config.reset_slabs.size() << std::endl;
    }

    // Play background music if specified
    if (!config.background_music.empty()) {
        music_channel = Audio::Play(config.background_music.c_str(), config.music_volume, true);
    }

    std::cout << "Level '" << config.name << "' loaded. Gravity: " << gravity_value << std::endl;
}

void World::reset()
{
    // Reload the current level configuration
    loadLevel(current_config);
    std::cout << "Level reset!" << std::endl;
}
