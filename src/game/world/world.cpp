#include "game/world/world.h"
#include "framework/entities/entity.h"
#include "framework/entities/entity_skybox.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "game/entities/entity_player.h"

World::World()
{
    //create skybox entity - TEMPORALMENTE COMENTADO PARA DEBUG
    /*EntitySkybox* skybox = new EntitySkybox();

    skybox->mesh = Mesh::Get("data/meshes/cubemap.ASE");
    skybox->shader = Shader::Get("data/shaders/skybox.vs", "data/shaders/skybox.fs");

    skybox->texture = new Texture();
    std::vector<std::string> faces = {
        "data/textures/texture.tga",
        "data/textures/texture.tga",
        "data/textures/texture.tga",
        "data/textures/texture.tga",
        "data/textures/texture.tga",
        "data/textures/texture.tga"
    };
    skybox->texture->loadCubemap("skybox_temp", faces);

    entities.push_back(skybox);*/

    //create player entity
    EntityPlayer* player = new EntityPlayer();
    player->mesh = Mesh::Get("data/meshes/box.ASE");
    player->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    player->texture = Texture::Get("data/textures/texture.tga");
    player->model.setTranslation( Vector3(0,5,0) );
    player->setScale(5.0f); // Hacer el cubo 5 veces más grande

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
