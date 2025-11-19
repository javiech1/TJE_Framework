#include "game/world/world.h"
#include "framework/entities/entity.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "game/entities/entity_player.h"
#include "game/entities/entity_platform.h"
#include "game/entities/entity_orb.h"
#include "framework/utils.h"

World::World()
{
    //create player entity
    EntityPlayer* player = new EntityPlayer();
    player->mesh = Mesh::Get("data/meshes/box.ASE");
    player->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    player->texture = Texture::Get("data/textures/texture.tga");
    const float scale = 0.01f;
    player->setScale(scale);
    // Start player above the platform so it falls down
    player->setPosition(Vector3(0.0f, 5.0f, 0.0f));
    this->player = player;
    entities.push_back(player);
}

World::~World()
{
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

    drawText(10, 10, "Orbs collected: " + std::to_string(orbs_collected) + "/3", Vector3(1,1,1), 2);

}

void World::update(float delta_time)
{
    for (Entity* entity : entities)
    {
        entity->update(delta_time);
    }

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
    //check player collisions
    player->checkCollisions(entities);
}

void World::onKeyDown(SDL_KeyboardEvent event)
{

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
    platform_ground->setScale(Vector3(20.0f, 0.5f, 20.0f));
    platform_ground->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    entities.push_back(platform_ground);
/*
    EntityPlatform* platform_stair1 = new EntityPlatform();
    platform_stair1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform_stair1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    platform_stair1->texture = Texture::Get("data/textures/texture.tga");
    platform_stair1->setScale(5.0f);
    platform_stair1->setPosition(Vector3(5.0f, 2.0f, 0.0f));
    //entities.push_back(platform_stair1);

    EntityPlatform* platform_stair2 = new EntityPlatform();
    platform_stair2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform_stair2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    platform_stair2->texture = Texture::Get("data/textures/texture.tga");
    platform_stair2->setScale(5.0f);
    platform_stair2->setPosition(Vector3(10.0f, 4.0f, 0.0f));
    //entities.push_back(platform_stair2);

    //create orbs
    for (int i = 0; i < 3; i++) {
        EntityOrb* orb = new EntityOrb();
        orb->setPosition(Vector3((i - 1) * 2.0f, 1.0f, 0.0f));
        //entities.push_back(orb);
        //orbs.push_back(orb);
    }
    //orbs[0]->setPosition(Vector3(0.0f, 1.0f, 0.0f));
    //orbs[1]->setPosition(Vector3(5.0f, 5.0f, 0.0f));
    //orbs[2]->setPosition(Vector3(10.0f, 7.0f, 0.0f));
    */
}
