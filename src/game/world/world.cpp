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
#include "game/entities/entity_obstacle.h"
#include "framework/utils.h"
#include "framework/audio.h"
#include "framework/collision.h"
#include <iostream>
#include <limits>

World::World()
{
    //create player entity
    EntityPlayer* player = new EntityPlayer();
    player->mesh = Mesh::Get("data/meshes/arachnoid.obj");
    player->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    player->texture = Texture::Get("data/textures/arachnoid.png");
    // Player scale is defined in EntityPlayer constructor (0.1f)
    // Player position is set by loadLevel() from level config
    player->setWorld(this);  // Connect player to world for gravity

    // Set up twin platform toggle callback (triggers on any jump)
    player->setOnJumpCallback([this]() {
        this->toggleTwinPlatforms();
    });

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

    // Render obstacles (translucent - rendered last for correct blending)
    for (EntityObstacle* obs : obstacles)
    {
        obs->render(camera);
    }

    // UI Text - orb counter
    drawText(10, 35, "Orbs collected: " + std::to_string(orbs_collected) + "/" + std::to_string(orbs.size()), Vector3(1,1,1), 2);

    // Minimal controls hint (tutorial is intuitive)
    drawText(10, 560, "WASD=Move  SPACE=Jump  R=Reset", Vector3(0.5f, 0.5f, 0.5f), 1.5f);

}

void World::update(float delta_time)
{
    // CORRECT UPDATE ORDER for platformer physics:
    // 1. Input - what does the player want to do?
    // 2. Detect ground - is_grounded for friction/jumping
    // 3. Physics - apply movement with correct is_grounded
    // 4. Resolve collisions - push out of geometry AFTER moving

    // 1. Handle player input
    player->handleInput(delta_time);

    // 2. Detect ground BEFORE physics (so is_grounded is correct)
    player->detectGround(entities);

    // 3. Apply physics (movement, gravity, friction with correct is_grounded)
    player->update(delta_time);

    // 4. Resolve collisions AFTER physics (push out of penetrations)
    player->resolveCollisions(entities);

    // Update other entities (not player - already updated above)
    for (Entity* entity : entities)
    {
        if (entity != player) {
            entity->update(delta_time);
        }
    }

    // Update reset slabs (for pulsing animation)
    for (EntityResetSlab* slab : reset_slabs)
    {
        slab->update(delta_time);
    }

    // Update obstacles (movement animation)
    for (EntityObstacle* obs : obstacles)
    {
        obs->update(delta_time);
    }

    // Check if player has fallen below the world
    // Threshold lowered to -20.0f to avoid accidental resets on lower platforms
    if (player->getPosition().y < -20.0f) {
        std::cout << "Player fell! Respawning..." << std::endl;
        reset();
        return; // Skip rest of update this frame
    }

    // Check reset slab collisions
    for (EntityResetSlab* slab : reset_slabs) {
        if (slab->collidesWithPlayer(player->getPosition(), player->getCollisionRadius())) {
            std::cout << "Hit reset slab! Respawning..." << std::endl;
            reset();
            return; // Skip rest of update this frame
        }
    }

    // Check obstacle collisions
    for (EntityObstacle* obs : obstacles) {
        if (obs->collidesWithPlayer(player->getPosition(), player->getCollisionRadius())) {
            std::cout << "Hit obstacle! Respawning..." << std::endl;
            reset();
            return; // Skip rest of update this frame
        }
    }

    // Check orb collection - Simple sphere-to-sphere collision
    for (EntityOrb* orb : orbs) {
        if(!orb->getIsCollected()) {
            // Get positions
            Vector3 player_pos = player->getPosition();
            Vector3 orb_pos = orb->getPosition();

            // Calculate collision radii using unified method
            float player_radius = player->getCollisionRadius();
            float orb_radius = orb->getRadius();
            float collection_distance = player_radius + orb_radius;

            // Check distance between centers
            float distance = (player_pos - orb_pos).length();

            if (distance < collection_distance) {
                orb->collect();
                orbs_collected++;

                // Update checkpoint to orb position
                last_checkpoint = orb_pos;
                last_checkpoint.y += 1.5f;  // Spawn slightly above orb

                std::cout << "Checkpoint! Orb collected (" << orbs_collected << "/" << orbs.size() << ")" << std::endl;
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

    // Clear orbs, reset slabs, and obstacles
    for (EntityOrb* orb : orbs) {
        delete orb;
    }
    orbs.clear();

    for (EntityResetSlab* slab : reset_slabs) {
        delete slab;
    }
    reset_slabs.clear();

    for (EntityObstacle* obs : obstacles) {
        delete obs;
    }
    obstacles.clear();

    // Twin platforms are in entities vector, just clear tracking list
    twin_platforms.clear();

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

    // Handle different level types (populate geometry before placing player)
    if (config.type == LevelConfig::EMPTY) {
        initEmpty();
    }
    else if (config.type == LevelConfig::DATA) {
        // Load platforms from config
        for (const auto& plat_def : config.platforms) {
            EntityPlatform* platform = new EntityPlatform();
            platform->mesh = Mesh::Get("data/meshes/box.ASE");

            // Ensure collision model is created for the mesh
            if (platform->mesh && !platform->mesh->collision_model) {
                platform->mesh->createCollisionModel();
            }

            platform->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
            platform->setPosition(plat_def.position);
            platform->setScale(plat_def.scale);
            platform->color = plat_def.color;

            // Configure movement if specified
            if (plat_def.movement_type == "linear") {
                platform->setLinearMovement(plat_def.movement_start, plat_def.movement_end,
                                           plat_def.movement_speed, plat_def.movement_phase);
            }
            else if (plat_def.movement_type == "circular") {
                platform->setCircularMovement(plat_def.orbit_center, plat_def.orbit_radius,
                                             plat_def.movement_speed, plat_def.movement_phase);
            }

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

        // Load obstacles from config
        for (const auto& obs_def : config.obstacles) {
            EntityObstacle* obstacle = new EntityObstacle();
            obstacle->setScale(obs_def.scale);
            obstacle->color = obs_def.color;

            // Configure movement
            if (obs_def.movement_type == "linear") {
                obstacle->setLinearMovement(obs_def.movement_start, obs_def.movement_end,
                                           obs_def.movement_speed, obs_def.movement_phase);
            }
            else if (obs_def.movement_type == "circular") {
                obstacle->setCircularMovement(obs_def.orbit_center, obs_def.orbit_radius,
                                             obs_def.movement_speed, obs_def.movement_phase);
            }
            else {
                obstacle->setPosition(obs_def.position);
            }

            obstacles.push_back(obstacle);
        }

        // Load twin platforms from config
        for (const auto& twin_def : config.twin_platforms) {
            EntityPlatform* platform = new EntityPlatform();
            platform->mesh = Mesh::Get("data/meshes/box.ASE");

            // Ensure collision model is created for the mesh
            if (platform->mesh && !platform->mesh->collision_model) {
                platform->mesh->createCollisionModel();
            }

            platform->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
            platform->setPosition(twin_def.position);
            platform->setScale(twin_def.scale);
            platform->color = twin_def.color;

            // Configure twin behavior
            platform->setTwinGroup(twin_def.group_id, twin_def.starts_active);

            entities.push_back(platform);
            twin_platforms.push_back(platform);  // Track for toggle callback
        }

        std::cout << "Loaded level from data file" << std::endl;
        std::cout << "  Platforms: " << config.platforms.size() << std::endl;
        std::cout << "  Twin Platforms: " << config.twin_platforms.size() << std::endl;
        std::cout << "  Orbs: " << config.orbs.size() << std::endl;
        std::cout << "  Reset Slabs: " << config.reset_slabs.size() << std::endl;
        std::cout << "  Obstacles: " << config.obstacles.size() << std::endl;
    }

    // Initialize checkpoint system
    player_start = config.player_start_position;
    last_checkpoint = config.player_start_position;

    // Place player at configured position - let gravity handle landing
    if (player) {
        player->setPosition(config.player_start_position);
        player->resetVelocity();
        // Physics loop (detectGround + resolveCollisions) handles falling onto platform
    }

    // Play background music if specified
    if (!config.background_music.empty()) {
        music_channel = Audio::Play(config.background_music.c_str(), config.music_volume, true);
    }

    std::cout << "Level '" << config.name << "' loaded. Gravity: " << gravity_value << std::endl;

    // Trigger reset to ensure player properly lands on platform
    reset();
}

void World::reset()
{
    // Reset player to last checkpoint (soft reset)
    if (player) {
        player->setPosition(last_checkpoint);
        player->resetVelocity();
        // Let gravity handle landing at checkpoint
    }
    // Don't reset collected orbs - they stay collected
}

void World::fullReset()
{
    // Full level restart - reload everything
    loadLevel(current_config);
    std::cout << "Full level reset!" << std::endl;
}

void World::toggleTwinPlatforms()
{
    if (twin_platforms.empty()) return;

    for (EntityPlatform* platform : twin_platforms) {
        platform->toggleTwinState();
    }
    std::cout << "Twin platforms toggled!" << std::endl;
}
