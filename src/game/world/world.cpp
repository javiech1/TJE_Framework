#include "game/world/world.h"
#include "framework/entities/entity.h"
#include "framework/entities/entity_skybox.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "game/entities/entity_player.h"
#include "game/entities/entity_platform.h"
#include "game/entities/entity_orb.h"
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
    // Start player above the platform so it falls down
    player->setPosition(Vector3(0.0f, 2.0f, 0.0f));
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

    drawText(10, 35, "Orbs collected: " + std::to_string(orbs_collected) + "/3", Vector3(1,1,1), 2);

}

void World::update(float delta_time)
{
    // SIMPLE UPDATE ORDER:

    // 1. Update all entities (handle input and movement)
    for (Entity* entity : entities)
    {
        entity->update(delta_time);
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

void World::initTutorial() {
    //create platform
    EntityPlatform* platform_ground = new EntityPlatform();
    platform_ground->mesh = Mesh::Get("data/meshes/box.ASE");
    platform_ground->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    platform_ground->texture = nullptr;  // No texture, using solid color
    platform_ground->color = Vector4(0.2f, 0.4f, 0.8f, 1.0f);  // Blue color
    platform_ground->setScale(Vector3(0.2f, 0.02f, 0.2f));  // 20×2×20 units platform (2x thicker)
    platform_ground->setPosition(Vector3(0.0f, -1.0f, 0.0f));  // Adjusted for thicker platform
    entities.push_back(platform_ground);


    EntityPlatform* platform_stair1 = new EntityPlatform();
    platform_stair1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform_stair1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    platform_stair1->texture = nullptr;  // No texture, using solid color
    platform_stair1->color = Vector4(0.8f, 0.4f, 0.2f, 1.0f);
    platform_stair1->setScale(Vector3(0.1f, 0.02f, 0.1f)); //10x2x10 units (2x thicker)
    platform_stair1->setPosition(Vector3(0.0f, 1.0f, -20.0f));  // Adjusted for thicker platform
    entities.push_back(platform_stair1);

    EntityPlatform* platform_stair2 = new EntityPlatform();
    platform_stair2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform_stair2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    platform_stair2->texture = nullptr;  // No texture, using solid color
    platform_stair2->color = Vector4(0.8f, 0.4f, 0.2f, 1.0f);
    platform_stair2->setScale(Vector3(0.1f, 0.02f, 0.1f)); //10x2x10 units (2x thicker)
    platform_stair2->setPosition(Vector3(0.0f, 3.0f, -35.0f));  // Adjusted for thicker platform
    entities.push_back(platform_stair2);
    
    
    //create orbs
    // Orb 1: On main platform (platform top at y=0)
    EntityOrb* orb1 = new EntityOrb();
    orb1->setPosition(Vector3(0.0f, 0.7f, 0.0f));  // Well above ground platform
    entities.push_back(orb1);
    orbs.push_back(orb1);

    // Orb 2: On first stair platform (platform top at y=2.0)
    EntityOrb* orb2 = new EntityOrb();
    orb2->setPosition(Vector3(0.0f, 2.7f, -20.0f));  // 0.7 units above platform_stair1
    entities.push_back(orb2);
    orbs.push_back(orb2);

    // Orb 3: On second stair platform (platform top at y=4.0)
    EntityOrb* orb3 = new EntityOrb();
    orb3->setPosition(Vector3(0.0f, 4.7f, -35.0f));  // 0.7 units above platform_stair2
    entities.push_back(orb3);
    orbs.push_back(orb3);

    // Play background music in loop
    music_channel = Audio::Play("data/audio/stellar_drift.mp3", 0.5f, BASS_SAMPLE_LOOP);
    if (music_channel) {
        std::cout << "Tutorial music started!" << std::endl;
    } else {
        std::cout << "Warning: Could not play background music" << std::endl;
    }
}

void World::reset()
{
    // Reset player position to current level's starting position
    player->setPosition(current_config.player_start_position);

    // Reset player velocity to prevent momentum carryover
    player->resetVelocity();

    // Reset orbs collected counter
    orbs_collected = 0;

    // Reset orbs to not collected
    for (EntityOrb* orb : orbs) {
        orb->reset();
    }

    std::cout << "Level restarted!" << std::endl;
}

// TODO(human): Implement the loadLevel method here
void World::loadLevel(const LevelConfig& config)
{

    //store the config in current_config
    current_config = config;

    //set gravity_value from config.gravity
    gravity_value = config.gravity;
    
    //clear current level (call clearLevel())
    clearLevel();
    
    //based on config.type, call initTutorial() or initEmpty()
    if(config.type == LevelConfig::TUTORIAL) {
        initTutorial();
    } else if(config.type == LevelConfig::EMPTY) {
        initEmpty();
    }
    //Set player position from config.player_start_position
    if(player) {
        player->setPosition(config.player_start_position);
    }
    
    //handle background music
    if (music_channel) {
        Audio::Stop(music_channel);
        music_channel = 0;
    }
    if (!config.background_music.empty()) {
        music_channel = Audio::Play(config.background_music.c_str(), 0.5f, BASS_SAMPLE_LOOP);
        if (music_channel) {
            std::cout << "Background music started: " << config.background_music << std::endl;
        } else {
            std::cout << "Warning: Could not play background music: " << config.background_music << std::endl;
        }
    }
}

void World::clearLevel()
{
    // Clear all entities except the player
    std::vector<Entity*> new_entities;
    for (Entity* entity : entities) {
        if (entity == player) {
            new_entities.push_back(player);  // Keep player
        } else {
            delete entity;  // Delete other entities
        }
    }
    entities = new_entities;

    // Clear orbs
    orbs.clear();
    orbs_collected = 0;

    // Stop music if playing
    if (music_channel) {
        Audio::Stop(music_channel);
        music_channel = 0;
    }
}

void World::initEmpty()
{
    // Empty level - no platforms or orbs
    // Just the player in empty space
    std::cout << "Loaded empty level - Gravity: " << gravity_value << std::endl;
}
