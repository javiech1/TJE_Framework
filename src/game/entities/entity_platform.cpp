#include "entity_platform.h"
#include "framework/input.h"
#include "game/game.h"

EntityPlatform::EntityPlatform() : EntityMesh()
{
    half_size = Vector3(0.5f, 0.5f, 0.5f);
}

EntityPlatform::~EntityPlatform()
{

}

void EntityPlatform::render(Camera* camera)
{
    EntityMesh::render(camera);
}

void EntityPlatform::update(float delta_time)
{
    //for moving platforms, if needed
}

