#include "entity_platform.h"
#include "framework/input.h"
#include "game/game.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "framework/camera.h"
#include "framework/collision.h"
#include <iostream>
#include <cmath>

EntityPlatform::EntityPlatform() : EntityCollider()
{
    half_size = Vector3(0.5f, 0.5f, 0.5f);
    scale_dimensions = Vector3(1.0f, 1.0f, 1.0f);
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
    if (movement_type == MovementType::NONE) {
        current_velocity = Vector3(0, 0, 0);
        return;
    }

    // Store last position for velocity calculation
    last_position = model.getTranslation();

    movement_time += delta_time;
    Vector3 new_pos;

    if (movement_type == MovementType::LINEAR) {
        // Ping-pong between start and end
        // movement_speed is cycles per second (full back-and-forth)
        float cycle = movement_time * movement_speed + movement_phase;
        float t = fmod(cycle, 2.0f);  // 0 to 2
        if (t > 1.0f) t = 2.0f - t;   // Reverse direction after 1.0

        // Smooth easing (smoothstep-like)
        t = t * t * (3.0f - 2.0f * t);

        new_pos = lerp(start_position, end_position, t);
    }
    else if (movement_type == MovementType::CIRCULAR) {
        // Orbit around center point
        // movement_speed is radians per second
        float angle = movement_time * movement_speed + movement_phase;
        new_pos.x = center_position.x + cos(angle) * orbit_radius;
        new_pos.y = center_position.y;
        new_pos.z = center_position.z + sin(angle) * orbit_radius;
    }

    // Calculate velocity (displacement / time)
    if (delta_time > 0.0001f) {
        current_velocity = (new_pos - last_position) * (1.0f / delta_time);
    }

    // Update position while preserving scale
    model.setIdentity();
    model.translate(new_pos.x, new_pos.y, new_pos.z);
    model.scale(scale_dimensions.x, scale_dimensions.y, scale_dimensions.z);
}

void EntityPlatform::setScale(Vector3 dimensions)
{
    // box.ASE is 100 units, so half is 50. Store for collision
    half_size = dimensions * 50.0f;
    scale_dimensions = dimensions;  // Store for movement matrix rebuilding

    // Use framework methods to build transformation matrix
    Vector3 position = model.getTranslation();

    model.setIdentity();
    model.translate(position.x, position.y, position.z);
    model.scale(dimensions.x, dimensions.y, dimensions.z);
}

void EntityPlatform::setPosition(const Vector3& new_position)
{
    // Rebuild matrix with position and scale
    model.setIdentity();
    model.translate(new_position.x, new_position.y, new_position.z);
    model.scale(scale_dimensions.x, scale_dimensions.y, scale_dimensions.z);
}

void EntityPlatform::setLinearMovement(const Vector3& start, const Vector3& end, float speed, float phase)
{
    movement_type = MovementType::LINEAR;
    start_position = start;
    end_position = end;
    movement_speed = speed;
    movement_phase = phase;
    movement_time = 0.0f;

    // Set initial position
    setPosition(start);
}

void EntityPlatform::setCircularMovement(const Vector3& center, float radius, float speed, float phase)
{
    movement_type = MovementType::CIRCULAR;
    center_position = center;
    orbit_radius = radius;
    movement_speed = speed;
    movement_phase = phase;
    movement_time = 0.0f;

    // Set initial position on orbit
    Vector3 initial_pos;
    initial_pos.x = center.x + cos(phase) * radius;
    initial_pos.y = center.y;
    initial_pos.z = center.z + sin(phase) * radius;
    setPosition(initial_pos);
}

Vector3 EntityPlatform::getCurrentPosition() const
{
    return model.getTranslation();
}

