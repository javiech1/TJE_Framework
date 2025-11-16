#include "game/world/world.h"
#include "framework/entities/entity.h"
#include "framework/entities/entity_skybox.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"

World::World()
{
    //create skybox entity
    EntitySkybox* skybox = new EntitySkybox();

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

    entities.push_back(skybox);
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