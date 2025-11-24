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

void World::initTutorial() {
    // ========== ZONE 1: MOVEMENT PRACTICE (Z=0 to Z=-15) ==========
    // ========== ZONE 1: MOVEMENT PRACTICE ==========
    // Platform 1.1 - Large starting platform
    EntityPlatform* platform1_1 = new EntityPlatform();
    platform1_1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform1_1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform1_1->texture = nullptr;
    platform1_1->color = Vector4(0.2f, 0.5f, 1.0f, 1.0f);  // Light blue
    platform1_1->setScale(Vector3(0.20f, 0.02f, 0.20f));  // 20x2x20 units - huge safe area
    platform1_1->setPosition(Vector3(0.0f, -1.0f, 0.0f));
    entities.push_back(platform1_1);

    // Platform 1.2 - Transition platform (NEW! Walking distance before first jump)
    EntityPlatform* platform1_2 = new EntityPlatform();
    platform1_2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform1_2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform1_2->texture = nullptr;
    platform1_2->color = Vector4(0.2f, 0.5f, 1.0f, 1.0f);  // Light blue
    platform1_2->setScale(Vector3(0.12f, 0.02f, 0.12f));  // 12x2x12 units
    platform1_2->setPosition(Vector3(0.0f, 0.0f, -12.0f));  // 12 units forward - walking distance
    entities.push_back(platform1_2);

    // ========== ZONE 2: BASIC JUMPING (Spiral begins) ==========
    // Platform 2.1 - First jump (easy, same height)
    EntityPlatform* platform2_1 = new EntityPlatform();
    platform2_1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform2_1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform2_1->texture = nullptr;
    platform2_1->color = Vector4(0.3f, 0.9f, 0.4f, 1.0f);  // Bright green
    platform2_1->setScale(Vector3(0.10f, 0.02f, 0.10f));  // 10x2x10 units
    platform2_1->setPosition(Vector3(0.0f, 0.0f, -20.0f));  // 8 units from transition - achievable!
    entities.push_back(platform2_1);

    // Platform 2.2 - Gentle rise with diagonal movement (starts spiral)
    EntityPlatform* platform2_2 = new EntityPlatform();
    platform2_2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform2_2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform2_2->texture = nullptr;
    platform2_2->color = Vector4(0.3f, 0.9f, 0.4f, 1.0f);  // Bright green
    platform2_2->setScale(Vector3(0.10f, 0.02f, 0.10f));  // 10x2x10 units
    platform2_2->setPosition(Vector3(4.0f, 1.0f, -27.0f));  // Diagonal right, +1.0 up (35% capacity)
    entities.push_back(platform2_2);

    // Platform 2.3 - Confidence builder (continue spiral)
    EntityPlatform* platform2_3 = new EntityPlatform();
    platform2_3->mesh = Mesh::Get("data/meshes/box.ASE");
    platform2_3->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform2_3->texture = nullptr;
    platform2_3->color = Vector4(0.3f, 0.9f, 0.4f, 1.0f);  // Bright green
    platform2_3->setScale(Vector3(0.10f, 0.02f, 0.10f));  // 10x2x10 units
    platform2_3->setPosition(Vector3(8.0f, 2.0f, -34.0f));  // Further right, +1.0 up
    entities.push_back(platform2_3);

    // Orb 1 - Reward for completing basic jumps
    EntityOrb* orb1 = new EntityOrb();
    orb1->setPosition(Vector3(8.0f, 4.0f, -34.0f));  // Above platform 2.3
    entities.push_back(orb1);
    orbs.push_back(orb1);

    // ========== ZONE 3: VERTICAL CLIMBING (Curved spiral climb) ==========
    // Platform 3.1 - Start climb (curves outward)
    EntityPlatform* platform3_1 = new EntityPlatform();
    platform3_1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform3_1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform3_1->texture = nullptr;
    platform3_1->color = Vector4(1.0f, 0.9f, 0.2f, 1.0f);  // Bright yellow
    platform3_1->setScale(Vector3(0.08f, 0.02f, 0.08f));  // 8x2x8 units
    platform3_1->setPosition(Vector3(10.0f, 3.5f, -41.0f));  // Further out, +1.5 up (52% capacity)
    entities.push_back(platform3_1);

    // Platform 3.2 - DRAMATIC CLIMB! (WOW moment at 87% capacity)
    EntityPlatform* platform3_2 = new EntityPlatform();
    platform3_2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform3_2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform3_2->texture = nullptr;
    platform3_2->color = Vector4(1.0f, 0.9f, 0.2f, 1.0f);  // Bright yellow
    platform3_2->setScale(Vector3(0.08f, 0.02f, 0.08f));  // 8x2x8 units
    platform3_2->setPosition(Vector3(10.0f, 6.0f, -49.0f));  // Stay out, +2.5 up! (87% capacity - challenging!)
    entities.push_back(platform3_2);

    // Platform 3.3 - Peak climb (curves back inward)
    EntityPlatform* platform3_3 = new EntityPlatform();
    platform3_3->mesh = Mesh::Get("data/meshes/box.ASE");
    platform3_3->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform3_3->texture = nullptr;
    platform3_3->color = Vector4(1.0f, 0.9f, 0.2f, 1.0f);  // Bright yellow
    platform3_3->setScale(Vector3(0.08f, 0.02f, 0.08f));  // 8x2x8 units
    platform3_3->setPosition(Vector3(8.0f, 8.0f, -56.0f));  // Curve inward, +2.0 up (70% capacity)
    entities.push_back(platform3_3);

    // Orb 2 - Reward for mastering vertical climbing
    EntityOrb* orb2 = new EntityOrb();
    orb2->setPosition(Vector3(8.0f, 10.0f, -56.0f));  // Above platform 3.3
    entities.push_back(orb2);
    orbs.push_back(orb2);

    // ========== ZONE 4: PRECISION CHALLENGE (Zigzag pattern) ==========
    // Platform 4.1 - Right swing (smaller platforms!)
    EntityPlatform* platform4_1 = new EntityPlatform();
    platform4_1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform4_1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform4_1->texture = nullptr;
    platform4_1->color = Vector4(1.0f, 0.6f, 0.1f, 1.0f);  // Bright orange
    platform4_1->setScale(Vector3(0.06f, 0.02f, 0.06f));  // 6x2x6 units - precision required!
    platform4_1->setPosition(Vector3(12.0f, 9.0f, -63.0f));  // Right swing, +1.0 up
    entities.push_back(platform4_1);

    // Platform 4.2 - Left swing (max horizontal+vertical challenge!)
    EntityPlatform* platform4_2 = new EntityPlatform();
    platform4_2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform4_2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform4_2->texture = nullptr;
    platform4_2->color = Vector4(1.0f, 0.6f, 0.1f, 1.0f);  // Bright orange
    platform4_2->setScale(Vector3(0.06f, 0.02f, 0.06f));  // 6x2x6 units
    platform4_2->setPosition(Vector3(4.0f, 10.5f, -70.0f));  // Sharp left, +1.5 up (10.6 unit diagonal!)
    entities.push_back(platform4_2);

    // Platform 4.3 - Center return (recovery jump)
    EntityPlatform* platform4_3 = new EntityPlatform();
    platform4_3->mesh = Mesh::Get("data/meshes/box.ASE");
    platform4_3->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform4_3->texture = nullptr;
    platform4_3->color = Vector4(1.0f, 0.6f, 0.1f, 1.0f);  // Bright orange
    platform4_3->setScale(Vector3(0.06f, 0.02f, 0.06f));  // 6x2x6 units
    platform4_3->setPosition(Vector3(0.0f, 11.5f, -77.0f));  // Return to center, +1.0 up
    entities.push_back(platform4_3);

    // Orb 3 - Reward for precision mastery
    EntityOrb* orb3 = new EntityOrb();
    orb3->setPosition(Vector3(0.0f, 13.5f, -77.0f));  // Above platform 4.3
    entities.push_back(orb3);
    orbs.push_back(orb3);

    // ========== ZONE 5: VICTORY PLATFORM! ==========
    // Large gold platform - you made it!
    EntityPlatform* platform5_1 = new EntityPlatform();
    platform5_1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform5_1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    platform5_1->texture = nullptr;
    platform5_1->color = Vector4(1.0f, 0.9f, 0.2f, 1.0f);  // Gold for victory!
    platform5_1->setScale(Vector3(0.14f, 0.02f, 0.14f));  // 14x2x14 units - welcoming and large
    platform5_1->setPosition(Vector3(0.0f, 12.5f, -85.0f));  // Return to center, +1.0 up
    entities.push_back(platform5_1);

    // Final orb - Tutorial complete!
    EntityOrb* orb4 = new EntityOrb();
    orb4->setPosition(Vector3(0.0f, 14.5f, -85.0f));  // Above victory platform
    entities.push_back(orb4);
    orbs.push_back(orb4);

    // ========== RESET SLABS (Safety boundaries) ==========
    // Kill floor - catch falls
    EntityResetSlab* slab_killfloor = new EntityResetSlab();
    slab_killfloor->mesh = Mesh::Get("data/meshes/box.ASE");
    slab_killfloor->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    slab_killfloor->color = Vector4(1.0f, 0.2f, 0.2f, 0.4f);  // Semi-transparent red
    slab_killfloor->setScale(Vector3(0.50f, 0.01f, 0.50f));  // 50×1×50 units - large catch area
    slab_killfloor->setPosition(Vector3(0.0f, -5.0f, -50.0f));  // Below platforms
    reset_slabs.push_back(slab_killfloor);

    // Right boundary wall
    EntityResetSlab* slab_right = new EntityResetSlab();
    slab_right->mesh = Mesh::Get("data/meshes/box.ASE");
    slab_right->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    slab_right->color = Vector4(1.0f, 0.2f, 0.2f, 0.3f);  // Semi-transparent red
    slab_right->setScale(Vector3(0.01f, 0.14f, 0.50f));  // 1×14×50 units - tall thin wall
    slab_right->setPosition(Vector3(18.0f, 7.0f, -45.0f));  // Right side
    reset_slabs.push_back(slab_right);

    // Left boundary wall
    EntityResetSlab* slab_left = new EntityResetSlab();
    slab_left->mesh = Mesh::Get("data/meshes/box.ASE");
    slab_left->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    slab_left->color = Vector4(1.0f, 0.2f, 0.2f, 0.3f);  // Semi-transparent red
    slab_left->setScale(Vector3(0.01f, 0.14f, 0.50f));  // 1×14×50 units
    slab_left->setPosition(Vector3(-18.0f, 7.0f, -45.0f));  // Left side
    reset_slabs.push_back(slab_left);

    // Back barrier - prevent backtracking
    EntityResetSlab* slab_back = new EntityResetSlab();
    slab_back->mesh = Mesh::Get("data/meshes/box.ASE");
    slab_back->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
    slab_back->color = Vector4(1.0f, 0.2f, 0.2f, 0.3f);  // Semi-transparent red
    slab_back->setScale(Vector3(0.40f, 0.14f, 0.01f));  // 40×14×1 units
    slab_back->setPosition(Vector3(0.0f, 7.0f, 15.0f));  // Behind start
    reset_slabs.push_back(slab_back);

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
    
    //based on config.type, call initTutorial(), initEmpty(), or load from data
    if(config.type == LevelConfig::TUTORIAL) {
        initTutorial();
    } else if(config.type == LevelConfig::EMPTY) {
        initEmpty();
    } else if(config.type == LevelConfig::DATA) {
        // Load level from JSON data (platforms, orbs, reset slabs)

        // Create platforms from config
        for (const auto& pdef : config.platforms) {
            EntityPlatform* platform = new EntityPlatform();
            platform->mesh = Mesh::Get("data/meshes/box.ASE");
            platform->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
            platform->color = pdef.color;

            // Load texture if specified
            if (!pdef.texture_path.empty()) {
                platform->texture = Texture::Get(pdef.texture_path.c_str());
            } else {
                platform->texture = nullptr;
            }

            platform->setScale(pdef.scale);
            platform->setPosition(pdef.position);
            entities.push_back(platform);
        }

        // Create orbs from config
        for (const auto& odef : config.orbs) {
            EntityOrb* orb = new EntityOrb();
            orb->setPosition(odef.position);
            // Orbs use default rendering, color is handled internally
            entities.push_back(orb);
            orbs.push_back(orb);
        }

        // Create reset slabs from config
        for (const auto& sdef : config.reset_slabs) {
            EntityResetSlab* slab = new EntityResetSlab();
            slab->mesh = Mesh::Get("data/meshes/box.ASE");
            slab->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
            slab->color = sdef.color;
            slab->setScale(sdef.scale);
            slab->setPosition(sdef.position);
            reset_slabs.push_back(slab);
        }

        std::cout << "Loaded level from JSON data: " << config.platforms.size()
                  << " platforms, " << config.orbs.size() << " orbs, "
                  << config.reset_slabs.size() << " reset slabs" << std::endl;
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

    // Clear reset slabs
    for (EntityResetSlab* slab : reset_slabs) {
        delete slab;
    }
    reset_slabs.clear();

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
