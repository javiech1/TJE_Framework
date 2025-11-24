#include "entity_platform.h"
#include "framework/input.h"
#include "game/game.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "framework/camera.h"
#include "framework/collision.h"
#include <iostream>

EntityPlatform::EntityPlatform() : EntityCollider()
{
    half_size = Vector3(0.5f, 0.5f, 0.5f);
    color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);  // Default white color

    // Set collision layer for platforms (FLOOR type)
    this->layer = eCollisionFilter::FLOOR;
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
    shader->setUniform("u_camera_pos", camera->eye);  // Pass camera position for lighting

    if(texture) {
        shader->setUniform("u_texture", texture, 0);
    }

    mesh->render(GL_TRIANGLES);
    shader->disable();
}

void EntityPlatform::update(float delta_time)
{
    //for moving platforms, if needed
}

void EntityPlatform::setScale(Vector3 dimensions)
{
    // box.ASE is 100 units, so half is 50. Store for potential future use
    half_size = dimensions * 50.0f;

    // Use framework methods to build transformation matrix
    Vector3 position = model.getTranslation();

    model.setIdentity();
    model.translate(position.x, position.y, position.z);
    model.scale(dimensions.x, dimensions.y, dimensions.z);
}

void EntityPlatform::setPosition(const Vector3& new_position)
{
    // Use framework method instead of direct matrix manipulation
    model.setTranslation(new_position.x, new_position.y, new_position.z);
}

