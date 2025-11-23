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

    // UI Text - orb counter (now 4 orbs in tutorial)
    drawText(10, 35, "Orbs collected: " + std::to_string(orbs_collected) + "/" + std::to_string(orbs.size()), Vector3(1,1,1), 2);

    // Tutorial instructions based on player position
    if (player) {
        float playerZ = player->getPosition().z;
        Vector3 textColor(1.0f, 1.0f, 0.0f); // Yellow for instructions

        if (playerZ > -15.0f) {
            // Zone 1: Movement
            drawText(300, 200, "Use WASD to move around", textColor, 2);
            drawText(280, 230, "Walk to the edge and continue", Vector3(0.8f, 0.8f, 0.8f), 1.5f);
        }
        else if (playerZ > -44.0f) {
            // Zone 2: Simple jumps
            drawText(300, 200, "Press SPACE to jump", textColor, 2);
            drawText(260, 230, "Jump between the green platforms", Vector3(0.8f, 0.8f, 0.8f), 1.5f);
        }
        else if (playerZ > -70.0f) {
            // Zone 3: Vertical jumps
            drawText(250, 200, "Jump to reach higher platforms", textColor, 2);
            drawText(280, 230, "Climb the yellow stairs", Vector3(0.8f, 0.8f, 0.8f), 1.5f);
        }
        else if (playerZ > -96.0f) {
            // Zone 4: Precision
            drawText(280, 200, "Time your jumps carefully", textColor, 2);
            drawText(240, 230, "These platforms are smaller!", Vector3(1.0f, 0.5f, 0.2f), 1.5f);
        }
        else if (playerZ > -120.0f) {
            // Zone 5: Final challenge
            drawText(300, 200, "Final challenge!", Vector3(1.0f, 0.2f, 0.2f), 2);
            drawText(240, 230, "Combine all your skills to reach the end", Vector3(0.8f, 0.8f, 0.8f), 1.5f);
        }
        else {
            // Completed!
            drawText(280, 200, "Tutorial Complete!", Vector3(0.2f, 1.0f, 0.2f), 3);
            drawText(260, 240, "Press N for next level", Vector3(0.8f, 0.8f, 0.8f), 2);
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
    // ========== ZONE 1: MOVEMENT PRACTICE (Z=0 to Z=-15) ==========
    // Large starting platform for movement practice
    EntityPlatform* platform_start = new EntityPlatform();
    platform_start->mesh = Mesh::Get("data/meshes/box.ASE");
    platform_start->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform_start->texture = nullptr;
    platform_start->color = Vector4(0.2f, 0.5f, 1.0f, 1.0f);  // Light blue
    platform_start->setScale(Vector3(0.20f, 0.02f, 0.20f));  // 20x2x20 units - extends from Z=-10 to Z=10
    platform_start->setPosition(Vector3(0.0f, -1.0f, 0.0f));
    entities.push_back(platform_start);

    // ========== ZONE 2: SIMPLE HORIZONTAL JUMPS (Z=-20 to Z=-40) ==========
    // Three platforms at same height for easy jumping practice
    // Platform 2.1
    EntityPlatform* platform2_1 = new EntityPlatform();
    platform2_1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform2_1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform2_1->texture = nullptr;
    platform2_1->color = Vector4(0.3f, 0.9f, 0.4f, 1.0f);  // Bright green
    platform2_1->setScale(Vector3(0.08f, 0.02f, 0.08f));  // 8x2x8 units
    platform2_1->setPosition(Vector3(0.0f, -1.0f, -22.0f));  // 12 unit gap from starting platform edge (8 units edge-to-edge)
    entities.push_back(platform2_1);

    // Platform 2.2
    EntityPlatform* platform2_2 = new EntityPlatform();
    platform2_2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform2_2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform2_2->texture = nullptr;
    platform2_2->color = Vector4(0.3f, 0.9f, 0.4f, 1.0f);  // Bright green
    platform2_2->setScale(Vector3(0.08f, 0.02f, 0.08f));
    platform2_2->setPosition(Vector3(0.0f, -1.0f, -34.0f));  // 8 unit gap (edge to edge)
    entities.push_back(platform2_2);

    // Platform 2.3
    EntityPlatform* platform2_3 = new EntityPlatform();
    platform2_3->mesh = Mesh::Get("data/meshes/box.ASE");
    platform2_3->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform2_3->texture = nullptr;
    platform2_3->color = Vector4(0.3f, 0.9f, 0.4f, 1.0f);  // Bright green
    platform2_3->setScale(Vector3(0.08f, 0.02f, 0.08f));
    platform2_3->setPosition(Vector3(0.0f, -1.0f, -47.0f));  // 9 unit gap - harder jump!
    entities.push_back(platform2_3);

    // Orb 1 at end of simple jump zone
    EntityOrb* orb1 = new EntityOrb();
    orb1->setPosition(Vector3(0.0f, 0.5f, -47.0f));
    entities.push_back(orb1);
    orbs.push_back(orb1);

    // ========== ZONE 3: VERTICAL PROGRESSION (Z=-48 to Z=-68) ==========
    // Ascending staircase with lateral movement
    // Platform 3.1 - Ground level entry
    EntityPlatform* platform3_1 = new EntityPlatform();
    platform3_1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform3_1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform3_1->texture = nullptr;
    platform3_1->color = Vector4(1.0f, 0.9f, 0.2f, 1.0f);  // Bright yellow
    platform3_1->setScale(Vector3(0.08f, 0.02f, 0.08f));  // 8x2x8 units
    platform3_1->setPosition(Vector3(0.0f, 0.0f, -56.0f));  // 5 unit gap from Zone 2
    entities.push_back(platform3_1);

    // Platform 3.2 - First climb
    EntityPlatform* platform3_2 = new EntityPlatform();
    platform3_2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform3_2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform3_2->texture = nullptr;
    platform3_2->color = Vector4(1.0f, 0.9f, 0.2f, 1.0f);  // Bright yellow
    platform3_2->setScale(Vector3(0.08f, 0.02f, 0.08f));
    platform3_2->setPosition(Vector3(4.0f, 2.0f, -65.0f));  // 2 units up, 4 units right, diagonal jump
    entities.push_back(platform3_2);

    // Platform 3.3 - Second climb
    EntityPlatform* platform3_3 = new EntityPlatform();
    platform3_3->mesh = Mesh::Get("data/meshes/box.ASE");
    platform3_3->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform3_3->texture = nullptr;
    platform3_3->color = Vector4(1.0f, 0.9f, 0.2f, 1.0f);  // Bright yellow
    platform3_3->setScale(Vector3(0.08f, 0.02f, 0.08f));
    platform3_3->setPosition(Vector3(-4.0f, 3.5f, -74.0f));  // 1.5 units up, 4 units left, zigzag pattern
    entities.push_back(platform3_3);

    // Platform 3.4 - Top platform
    EntityPlatform* platform3_4 = new EntityPlatform();
    platform3_4->mesh = Mesh::Get("data/meshes/box.ASE");
    platform3_4->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform3_4->texture = nullptr;
    platform3_4->color = Vector4(1.0f, 0.9f, 0.2f, 1.0f);  // Bright yellow
    platform3_4->setScale(Vector3(0.08f, 0.02f, 0.08f));
    platform3_4->setPosition(Vector3(0.0f, 5.0f, -83.0f));  // 1.5 units up, center, final staircase jump
    entities.push_back(platform3_4);

    // Orb 2 on highest platform
    EntityOrb* orb2 = new EntityOrb();
    orb2->setPosition(Vector3(0.0f, 6.0f, -83.0f));
    entities.push_back(orb2);
    orbs.push_back(orb2);

    // ========== ZONE 4: PRECISION JUMPS (Z=-74 to Z=-94) ==========
    // Smaller platforms requiring careful positioning
    // Platform 4.1
    EntityPlatform* platform4_1 = new EntityPlatform();
    platform4_1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform4_1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform4_1->texture = nullptr;
    platform4_1->color = Vector4(1.0f, 0.6f, 0.1f, 1.0f);  // Bright orange
    platform4_1->setScale(Vector3(0.06f, 0.02f, 0.06f));  // 6x2x6 units
    platform4_1->setPosition(Vector3(0.0f, 3.0f, -74.0f));  // 4 unit gap from Zone 3
    entities.push_back(platform4_1);

    // Platform 4.2 - Lateral jump right
    EntityPlatform* platform4_2 = new EntityPlatform();
    platform4_2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform4_2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform4_2->texture = nullptr;
    platform4_2->color = Vector4(1.0f, 0.6f, 0.1f, 1.0f);  // Bright orange
    platform4_2->setScale(Vector3(0.05f, 0.02f, 0.05f));  // 5x2x5 units - smaller!
    platform4_2->setPosition(Vector3(4.0f, 2.5f, -80.0f));
    entities.push_back(platform4_2);

    // Platform 4.3 - Diagonal jump left and up
    EntityPlatform* platform4_3 = new EntityPlatform();
    platform4_3->mesh = Mesh::Get("data/meshes/box.ASE");
    platform4_3->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform4_3->texture = nullptr;
    platform4_3->color = Vector4(1.0f, 0.6f, 0.1f, 1.0f);  // Bright orange
    platform4_3->setScale(Vector3(0.05f, 0.02f, 0.05f));  // 5x2x5 units
    platform4_3->setPosition(Vector3(-3.0f, 3.5f, -85.0f));
    entities.push_back(platform4_3);

    // Platform 4.4
    EntityPlatform* platform4_4 = new EntityPlatform();
    platform4_4->mesh = Mesh::Get("data/meshes/box.ASE");
    platform4_4->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform4_4->texture = nullptr;
    platform4_4->color = Vector4(1.0f, 0.6f, 0.1f, 1.0f);  // Bright orange
    platform4_4->setScale(Vector3(0.05f, 0.02f, 0.05f));  // 5x2x5 units
    platform4_4->setPosition(Vector3(3.0f, 4.0f, -90.0f));
    entities.push_back(platform4_4);

    // Platform 4.5
    EntityPlatform* platform4_5 = new EntityPlatform();
    platform4_5->mesh = Mesh::Get("data/meshes/box.ASE");
    platform4_5->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform4_5->texture = nullptr;
    platform4_5->color = Vector4(1.0f, 0.6f, 0.1f, 1.0f);  // Bright orange
    platform4_5->setScale(Vector3(0.06f, 0.02f, 0.06f));  // 6x2x6 units
    platform4_5->setPosition(Vector3(0.0f, 3.0f, -94.0f));
    entities.push_back(platform4_5);

    // Orb 3 in middle of precision zone
    EntityOrb* orb3 = new EntityOrb();
    orb3->setPosition(Vector3(-3.0f, 4.5f, -85.0f));
    entities.push_back(orb3);
    orbs.push_back(orb3);

    // ========== ZONE 5: FINAL CHALLENGE (Z=-100 to Z=-120) ==========
    // Hardest jumps combining distance and height
    // Platform 5.1
    EntityPlatform* platform5_1 = new EntityPlatform();
    platform5_1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform5_1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform5_1->texture = nullptr;
    platform5_1->color = Vector4(1.0f, 0.3f, 0.3f, 1.0f);  // Bright red
    platform5_1->setScale(Vector3(0.05f, 0.02f, 0.05f));  // 5x2x5 units
    platform5_1->setPosition(Vector3(-4.0f, 2.0f, -100.0f));  // 3 unit gap from Zone 4
    entities.push_back(platform5_1);

    // Platform 5.2 - Difficult diagonal jump
    EntityPlatform* platform5_2 = new EntityPlatform();
    platform5_2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform5_2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform5_2->texture = nullptr;
    platform5_2->color = Vector4(1.0f, 0.3f, 0.3f, 1.0f);  // Bright red
    platform5_2->setScale(Vector3(0.04f, 0.02f, 0.04f));  // 4x2x4 units - smallest!
    platform5_2->setPosition(Vector3(5.0f, 3.5f, -105.0f));  // 9 unit horizontal jump!
    entities.push_back(platform5_2);

    // Platform 5.3 - High jump
    EntityPlatform* platform5_3 = new EntityPlatform();
    platform5_3->mesh = Mesh::Get("data/meshes/box.ASE");
    platform5_3->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform5_3->texture = nullptr;
    platform5_3->color = Vector4(1.0f, 0.3f, 0.3f, 1.0f);  // Bright red
    platform5_3->setScale(Vector3(0.04f, 0.02f, 0.04f));  // 4x2x4 units
    platform5_3->setPosition(Vector3(-3.0f, 4.8f, -110.0f));  // Near max height jump
    entities.push_back(platform5_3);

    // Platform 5.4
    EntityPlatform* platform5_4 = new EntityPlatform();
    platform5_4->mesh = Mesh::Get("data/meshes/box.ASE");
    platform5_4->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform5_4->texture = nullptr;
    platform5_4->color = Vector4(1.0f, 0.3f, 0.3f, 1.0f);  // Bright red
    platform5_4->setScale(Vector3(0.05f, 0.02f, 0.05f));  // 5x2x5 units
    platform5_4->setPosition(Vector3(4.0f, 4.5f, -115.0f));
    entities.push_back(platform5_4);

    // Platform 5.5 - Victory platform
    EntityPlatform* platform5_5 = new EntityPlatform();
    platform5_5->mesh = Mesh::Get("data/meshes/box.ASE");
    platform5_5->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform5_5->texture = nullptr;
    platform5_5->color = Vector4(1.0f, 0.3f, 0.3f, 1.0f);  // Bright red
    platform5_5->setScale(Vector3(0.10f, 0.02f, 0.10f));  // 10x2x10 units - larger victory platform
    platform5_5->setPosition(Vector3(0.0f, 5.0f, -120.0f));
    entities.push_back(platform5_5);

    // Final orb as reward
    EntityOrb* orb4 = new EntityOrb();
    orb4->setPosition(Vector3(0.0f, 6.0f, -120.0f));
    entities.push_back(orb4);
    orbs.push_back(orb4);

    // Play background music
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
