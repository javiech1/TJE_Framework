#include "entity_platform.h"
#include "framework/input.h"
#include "game/game.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "framework/camera.h"
#include <iostream>

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

    // Custom render implementation with our color
    shader->enable();
    shader->setUniform("u_model", model);
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    shader->setUniform("u_color", color);  // Use our custom color
    shader->setUniform("u_time", Game::instance->time);

    if(texture) {
        shader->setUniform("u_texture", texture, 0);
    }

    mesh->render(GL_TRIANGLES);
    shader->disable();

    // DEBUG: Print platform info once
    static bool printed = false;
    if(!printed) {
        Vector3 pos = model.getTranslation();
        std::cout << "Platform rendering - Pos: (" << pos.x << ", " << pos.y << ", " << pos.z << "), Half-size: (" << half_size.x << ", " << half_size.y << ", " << half_size.z << ")" << std::endl;
        printed = true;
    }
}

void EntityPlatform::update(float delta_time)
{
    //for moving platforms, if needed
}

void EntityPlatform::setScale(Vector3 dimensions)
{
    // box.ASE is 100 units, so half is 50. Multiply scale by 50 to get actual half_size
    half_size = dimensions * 50.0f;

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

