#include "entity_obstacle.h"
#include "game/game.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "framework/camera.h"
#include "framework/includes.h"  // For OpenGL
#include <cmath>

EntityObstacle::EntityObstacle() : EntityCollider()
{
    half_size = Vector3(0.5f, 0.5f, 0.5f);
    scale_dimensions = Vector3(1.0f, 1.0f, 1.0f);
    position = Vector3(0, 0, 0);
    color = Vector4(1.0f, 0.2f, 0.2f, 0.4f);  // Red translucent

    // Load mesh and shader
    mesh = Mesh::Get("data/meshes/box.ASE");
    shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");

    // Obstacles don't participate in regular collision detection
    // We handle collision manually with collidesWithPlayer()
    this->layer = eCollisionFilter::NONE;
}

EntityObstacle::~EntityObstacle()
{
    // Mesh and shader are shared resources, don't delete
}

void EntityObstacle::render(Camera* camera)
{
    if (!mesh || !shader || !visible) return;

    // Enable blending for translucent effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);  // Don't write to depth buffer

    shader->enable();
    shader->setUniform("u_model", model);
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    shader->setUniform("u_color", color);
    shader->setUniform("u_time", Game::instance->time);

    mesh->render(GL_TRIANGLES);
    shader->disable();

    // Restore OpenGL state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void EntityObstacle::update(float delta_time)
{
    if (movement_type == ObstacleMovementType::NONE) return;

    movement_time += delta_time;
    Vector3 new_pos;

    if (movement_type == ObstacleMovementType::LINEAR) {
        // Ping-pong between start and end
        float cycle = movement_time * movement_speed + movement_phase;
        float t = fmod(cycle, 2.0f);
        if (t > 1.0f) t = 2.0f - t;

        // Smooth easing
        t = t * t * (3.0f - 2.0f * t);

        new_pos = lerp(start_position, end_position, t);
    }
    else if (movement_type == ObstacleMovementType::CIRCULAR) {
        float angle = movement_time * movement_speed + movement_phase;
        new_pos.x = center_position.x + cos(angle) * orbit_radius;
        new_pos.y = center_position.y;
        new_pos.z = center_position.z + sin(angle) * orbit_radius;
    }

    position = new_pos;

    // Rebuild model matrix
    model.setIdentity();
    model.translate(new_pos.x, new_pos.y, new_pos.z);
    model.scale(scale_dimensions.x, scale_dimensions.y, scale_dimensions.z);
}

void EntityObstacle::setScale(Vector3 dimensions)
{
    half_size = dimensions * 50.0f;  // box.ASE is 100 units
    scale_dimensions = dimensions;

    Vector3 pos = model.getTranslation();
    model.setIdentity();
    model.translate(pos.x, pos.y, pos.z);
    model.scale(dimensions.x, dimensions.y, dimensions.z);
}

void EntityObstacle::setPosition(const Vector3& new_position)
{
    position = new_position;
    model.setIdentity();
    model.translate(new_position.x, new_position.y, new_position.z);
    model.scale(scale_dimensions.x, scale_dimensions.y, scale_dimensions.z);
}

void EntityObstacle::setLinearMovement(const Vector3& start, const Vector3& end, float speed, float phase)
{
    movement_type = ObstacleMovementType::LINEAR;
    start_position = start;
    end_position = end;
    movement_speed = speed;
    movement_phase = phase;
    movement_time = 0.0f;

    setPosition(start);
}

void EntityObstacle::setCircularMovement(const Vector3& center, float radius, float speed, float phase)
{
    movement_type = ObstacleMovementType::CIRCULAR;
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

bool EntityObstacle::collidesWithPlayer(const Vector3& player_pos, float player_radius)
{
    // Simple sphere vs AABB collision
    Vector3 closest_point;

    // Clamp player position to obstacle AABB
    closest_point.x = clamp(player_pos.x, position.x - half_size.x, position.x + half_size.x);
    closest_point.y = clamp(player_pos.y, position.y - half_size.y, position.y + half_size.y);
    closest_point.z = clamp(player_pos.z, position.z - half_size.z, position.z + half_size.z);

    // Check distance from player center to closest point
    float distance = (player_pos - closest_point).length();

    return distance < player_radius;
}
