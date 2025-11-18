#include "game/world/world.h"
#include "framework/entities/entity.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "game/entities/entity_player.h"

World::World()
{
    //create player entity
    EntityPlayer* player = new EntityPlayer();
    player->mesh = Mesh::Get("data/meshes/box.ASE");
    player->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    player->texture = Texture::Get("data/textures/texture.tga");
    const float scale = 3.0f;
    player->setScale(scale); // Hacer el cubo 3 veces más grande
    player->setPosition(Vector3(0,scale * 0.5f,0));
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
}
void World::update(float delta_time)
{
    for (Entity* entity : entities)
    {
        entity->update(delta_time);
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
