#include "entity_platform.h"
#include "framework/input.h"
#include "game/game.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "framework/camera.h"

EntityPlatform::EntityPlatform() : EntityMesh()
{
    half_size = Vector3(0.5f, 0.5f, 0.5f);
    color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);  // Default white color
}

EntityPlatform::~EntityPlatform()
{

}

void EntityPlatform::render(Camera* camera)
{
    if(!mesh || !shader || !visible) return;

    // Set color uniform for flat shader
    shader->enable();
    shader->setUniform("u_color", color);
    shader->disable();

    // Call parent render
    EntityMesh::render(camera);
}

void EntityPlatform::update(float delta_time)
{
    //for moving platforms, if needed
}

void EntityPlatform::setScale(Vector3 dimensions)
{
    half_size = dimensions * 0.5f;

    Vector3 position = model.getTranslation();

    model.setIdentity();
    model.m[0] = dimensions.x;
    model.m[5] = dimensions.y;
    model.m[10] = dimensions.z;

    // Set translation directly without calling setTranslation (which would reset the matrix)
    model.m[12] = position.x;
    model.m[13] = position.y;
    model.m[14] = position.z;
}


void EntityPlatform::setPosition(const Vector3& new_position)
{
    model.m[12] = new_position.x;
    model.m[13] = new_position.y;
    model.m[14] = new_position.z;
}

