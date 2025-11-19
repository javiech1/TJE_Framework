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

void EntityPlatform::setScale(float scale)
{
    half_size = Vector3(scale * 0.5f, scale * 0.5f, scale * 0.5f);
    model.setIdentity();
    model.m[0] = scale;
    model.m[5] = scale;
    model.m[10] = scale;
    model.m[12] = model.m[12]; //keep position
    model.m[13] = model.m[13];
    model.m[14] = model.m[14];
}

void EntityPlatform::setPosition(const Vector3& new_position)
{
    model.m[12] = new_position.x;
    model.m[13] = new_position.y;
    model.m[14] = new_position.z;
}

