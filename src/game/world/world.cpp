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
    player->setPosition(Vector3(0.0f, scale * 0.5f, 0.0f));
    this->player = player;
    entities.push_back(player);

    //create platform entity
    EntityPlatform* platform = new EntityPlatform();
    platform->mesh = Mesh::Get("data/meshes/box.ASE");
    platform->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    platform->texture = Texture::Get("data/textures/texture.tga");
    platform->setScale(5.0f);
    platform->setPosition(Vector3(0.0f, 0.0f, 0.0f));
    entities.push_back(platform);

    //create some orbs
    for (int i = 0; i < 3; i++) {
        EntityOrb* orb = new EntityOrb();
        orb->setPosition(Vector3(i * 2.0f -2.0f, 1.0f, 0.0f));
        entities.push_back(orb);
        orbs.push_back(orb);
    }
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
