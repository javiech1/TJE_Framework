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
#include "framework/collision.h"
#include "framework/audio.h"
#include <limits>

World::World()
{
    EntityPlayer* player = new EntityPlayer();
    player->mesh = Mesh::Get("data/meshes/arachnoid.obj");
    player->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    player->texture = Texture::Get("data/textures/arachnoid.png");
    player->setWorld(this);
    player->setOnJumpCallback([this]() { this->toggleTwinPlatforms(); });

    this->player = player;
    entities.push_back(player);

    skybox = new EntitySkybox();
    skybox->mesh = Mesh::Get("data/meshes/cubemap.ASE");
    skybox->shader = Shader::Get("data/shaders/skybox.vs", "data/shaders/skybox.fs");
    skybox->texture = new Texture();
    std::vector<std::string> faces = {
        "data/sky/px.png", "data/sky/nx.png",
        "data/sky/py.png", "data/sky/ny.png",
        "data/sky/pz.png", "data/sky/nz.png"
    };
    skybox->texture->loadCubemap("space_skybox", faces);
}

World::~World()
{
    if (skybox) {
        delete skybox;
        skybox = nullptr;
    }

    for (Entity* entity : entities) {
        delete entity;
    }
    entities.clear();
}

void World::render(Camera* camera)
{
    for (Entity* entity : entities) {
        entity->render(camera);
    }

    for (EntityResetSlab* slab : reset_slabs) {
        slab->render(camera);
    }

    for (EntityOrb* orb : orbs) {
        orb->render(camera);
    }

    for (EntityObstacle* obs : obstacles) {
        obs->render(camera);
    }

    drawText(10, 35, "Orbs collected: " + std::to_string(orbs_collected) + "/" + std::to_string(orbs.size()), Vector3(1,1,1), 2);
    drawText(10, 560, "WASD=Move  SPACE=Jump  R=Reset", Vector3(0.5f, 0.5f, 0.5f), 1.5f);
}

void World::update(float delta_time)
{
    player->handleInput(delta_time);
    player->detectGround(entities);
    player->update(delta_time);
    player->resolveCollisions(entities);

    for (Entity* entity : entities) {
        if (entity != player) {
            entity->update(delta_time);
        }
    }

    for (EntityResetSlab* slab : reset_slabs) {
        slab->update(delta_time);
    }

    for (EntityObstacle* obs : obstacles) {
        obs->update(delta_time);
    }

    if (player->getPosition().y < -20.0f) {
        reset();
        return;
    }

    for (EntityResetSlab* slab : reset_slabs) {
        if (slab->collidesWithPlayer(player->getPosition(), player->getCollisionRadius())) {
            reset();
            return;
        }
    }

    for (EntityObstacle* obs : obstacles) {
        if (obs->collidesWithPlayer(player->getPosition(), player->getCollisionRadius())) {
            reset();
            return;
        }
    }

    for (EntityOrb* orb : orbs) {
        if (!orb->getIsCollected()) {
            Vector3 player_pos = player->getPosition();
            Vector3 orb_pos = orb->getPosition();
            float collection_distance = player->getCollisionRadius() + orb->getRadius();
            float distance = (player_pos - orb_pos).length();

            if (distance < collection_distance) {
                orb->collect();
                orbs_collected++;
                Audio::Play("data/audio/721542__tildeyann__ping_sherman01.wav", 0.6f);
                last_checkpoint = orb_pos;
                last_checkpoint.y += 1.5f;
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
}

void World::clearLevel()
{
    for (auto it = entities.begin(); it != entities.end(); ) {
        if (*it != player) {
            delete *it;
            it = entities.erase(it);
        } else {
            ++it;
        }
    }

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

    twin_platforms.clear();
    orbs_collected = 0;
}

void World::loadLevel(const LevelConfig& config)
{
    current_config = config;
    clearLevel();
    gravity_value = config.gravity;

    if (config.type == LevelConfig::EMPTY) {
        initEmpty();
    }
    else if (config.type == LevelConfig::DATA) {
        for (const auto& plat_def : config.platforms) {
            EntityPlatform* platform = new EntityPlatform();
            platform->mesh = Mesh::Get("data/meshes/box.ASE");
            if (platform->mesh && !platform->mesh->collision_model) {
                platform->mesh->createCollisionModel();
            }
            platform->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
            platform->setPosition(plat_def.position);
            platform->setScale(plat_def.scale);
            platform->color = plat_def.color;

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

        for (const auto& orb_def : config.orbs) {
            EntityOrb* orb = new EntityOrb();
            orb->setPosition(orb_def.position);
            orbs.push_back(orb);
        }

        for (const auto& slab_def : config.reset_slabs) {
            EntityResetSlab* slab = new EntityResetSlab();
            slab->mesh = Mesh::Get("data/meshes/box.ASE");
            slab->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
            slab->setPosition(slab_def.position);
            slab->setScale(slab_def.scale);
            slab->color = slab_def.color;
            reset_slabs.push_back(slab);
        }

        for (const auto& obs_def : config.obstacles) {
            EntityObstacle* obstacle = new EntityObstacle();
            obstacle->setScale(obs_def.scale);
            obstacle->color = obs_def.color;

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

        for (const auto& twin_def : config.twin_platforms) {
            EntityPlatform* platform = new EntityPlatform();
            platform->mesh = Mesh::Get("data/meshes/box.ASE");
            if (platform->mesh && !platform->mesh->collision_model) {
                platform->mesh->createCollisionModel();
            }
            platform->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/platform.fs");
            platform->setPosition(twin_def.position);
            platform->setScale(twin_def.scale);
            platform->color = twin_def.color;
            platform->setTwinGroup(twin_def.group_id, twin_def.starts_active);
            entities.push_back(platform);
            twin_platforms.push_back(platform);
        }
    }

    player_start = config.player_start_position;
    last_checkpoint = config.player_start_position;

    if (player) {
        player->setPosition(config.player_start_position);
        player->resetVelocity();
    }

    reset();
}

void World::reset()
{
    if (player) {
        player->setPosition(last_checkpoint);
        player->resetVelocity();
    }
}

void World::fullReset()
{
    loadLevel(current_config);
}

void World::toggleTwinPlatforms()
{
    for (EntityPlatform* platform : twin_platforms) {
        platform->toggleTwinState();
    }
}
