#include "entity_reset_slab.h"
#include "framework/camera.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "game/game.h"
#include <cmath>

EntityResetSlab::EntityResetSlab() : EntityCollider()
{
    half_size = Vector3(0.5f, 0.5f, 0.5f);
    color = Vector4(1.0f, 0.2f, 0.2f, 0.4f);  // Semi-transparent red
    pulse_time = 0.0f;

    // Set collision layer (could be FLOOR or custom)
    this->layer = eCollisionFilter::FLOOR;
}

EntityResetSlab::~EntityResetSlab()
{

}

void EntityResetSlab::render(Camera* camera)
{
    if(!mesh || !shader || !visible) return;

    // Enable alpha blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Calculate pulsing alpha (breathe between 0.3 and 0.5)
    float pulse_alpha = 0.3f + 0.2f * (0.5f + 0.5f * sin(pulse_time * 3.0f));
    Vector4 pulse_color = color;
    pulse_color.w = pulse_alpha;

    shader->enable();
    shader->setUniform("u_model", model);
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    shader->setUniform("u_color", pulse_color);
    shader->setUniform("u_time", Game::instance->time);
    shader->setUniform("u_camera_pos", camera->eye);

    mesh->render(GL_TRIANGLES);
    shader->disable();

    // Restore blending state
    glDisable(GL_BLEND);
}

void EntityResetSlab::update(float delta_time)
{
    pulse_time += delta_time;
}

void EntityResetSlab::setScale(Vector3 dimensions)
{
    // box.ASE is 100 units, so half is 50. Multiply scale by 50 to get actual half_size
    half_size = dimensions * 50.0f;

    Vector3 position = model.getTranslation();

    model.setIdentity();
    model.m[0] = dimensions.x;
    model.m[5] = dimensions.y;
    model.m[10] = dimensions.z;

    // Set translation directly
    model.m[12] = position.x;
    model.m[13] = position.y;
    model.m[14] = position.z;
}

void EntityResetSlab::setPosition(const Vector3& new_position)
{
    model.m[12] = new_position.x;
    model.m[13] = new_position.y;
    model.m[14] = new_position.z;
}

bool EntityResetSlab::collidesWithPlayer(const Vector3& player_pos, float player_radius) const
{
    std::vector<sCollisionData> collisions;
    // We cast this to Entity* (non-const) because TestEntitySphere expects Entity*
    // but we are in a const method. This is safe as long as we don't modify the entity in TestEntitySphere
    return Collision::TestEntitySphere(const_cast<EntityResetSlab*>(this), player_radius, player_pos, collisions, eCollisionFilter::ALL);
}
