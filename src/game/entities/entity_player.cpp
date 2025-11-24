#include "entity_player.h"
#include "game/world/world.h"
#include "framework/input.h"
#include "game/game.h"
#include "framework/collision.h"
#include <iostream>
#include <cmath>
#include <algorithm>

EntityPlayer::EntityPlayer() : EntityMesh()
{
    // Movement
    speed = 12.0f;
    jump_velocity = 9.0f;
    velocity = Vector3(0,0,0);

    // Jump state
    is_grounded = false;
    jump_was_pressed = false;
    jump_requested = false;

    // Player properties
    player_scale = 1.0f;
    current_yaw = 0.0f;
    target_yaw = 0.0f;
    world = nullptr;

    // Initialize model matrix with identity
    model.setIdentity();
}

EntityPlayer::~EntityPlayer()
{
}

void EntityPlayer::render(Camera* camera)
{
    EntityMesh::render(camera);
}

void EntityPlayer::update(float delta_time)
{
    // 1. Handle input (sets movement velocity and jump request)
    handleInput(delta_time);

    // 2. Apply physics (gravity, movement, friction)
    applyPhysics(delta_time);

    // 3. Check and resolve collisions
    // Note: World passes entities vector via checkCollisions()
    // This will be called from World::update()

    // 4. Smooth rotation towards target
    float rotation_speed = 15.0f;
    float yaw_diff = target_yaw - current_yaw;

    // Normalize angle difference to [-PI, PI] for shortest rotation
    while (yaw_diff > M_PI) yaw_diff -= 2.0f * M_PI;
    while (yaw_diff < -M_PI) yaw_diff += 2.0f * M_PI;

    // Interpolate rotation
    current_yaw += yaw_diff * std::min(1.0f, rotation_speed * delta_time);

    // Rebuild model matrix with new position and rotation
    updateModelMatrix();
}

void EntityPlayer::handleInput(float delta_time)
{
    Camera* camera = Game::instance ? Game::instance->camera : nullptr;
    Vector3 forward = camera ? (camera->center - camera->eye) : Vector3(0.0f, 0.0f, 1.0f);
    forward.y = 0.0f;
    if (forward.length() < 0.001f)
        forward = Vector3(0.0f, 0.0f, 1.0f);
    forward.normalize();

    Vector3 right = Vector3(0.0f, 1.0f, 0.0f).cross(forward);
    if (right.length() < 0.001f)
        right = Vector3(1.0f, 0.0f, 0.0f);
    right.normalize();

    Vector3 move_dir = Vector3(0,0,0);

    bool moveForward = Input::isKeyPressed(SDL_SCANCODE_W);
    bool moveBackward = Input::isKeyPressed(SDL_SCANCODE_S);
    bool moveLeft = Input::isKeyPressed(SDL_SCANCODE_A);
    bool moveRight = Input::isKeyPressed(SDL_SCANCODE_D);

    if (moveForward) move_dir += forward;
    if (moveBackward) move_dir -= forward;
    if (moveLeft) move_dir += right;
    if (moveRight) move_dir -= right;

    // Jump input with edge detection
    bool space_pressed = Input::isKeyPressed(SDL_SCANCODE_SPACE);
    if (space_pressed && !jump_was_pressed) {
        jump_requested = true;
    }
    jump_was_pressed = space_pressed;

    if (move_dir.length() > 0)
    {
        move_dir.normalize();
        velocity.x = move_dir.x * speed;
        velocity.z = move_dir.z * speed;

        // Calculate target rotation based on movement direction
        target_yaw = atan2(move_dir.x, move_dir.z);
    }
    else
    {
        velocity.x = 0;
        velocity.z = 0;
    }
}

void EntityPlayer::applyPhysics(float delta_time)
{
    // Process jump
    if (jump_requested) {
        if (is_grounded) {
            velocity.y = jump_velocity;
            is_grounded = false;
            std::cout << "JUMP! velocity.y = " << velocity.y << std::endl;
        }
        jump_requested = false;
    }

    // Apply gravity
    float gravity = world ? world->getGravity() : 9.8f;
    velocity.y -= gravity * delta_time;

    // Friction on ground
    if (is_grounded) {
        float friction = 5.0f;
        float damping = std::max(0.0f, 1.0f - (friction * delta_time));
        velocity.x *= damping;
        velocity.z *= damping;
    }

    // Apply movement directly to model matrix
    Vector3 current_pos = model.getTranslation();
    current_pos += velocity * delta_time;
    model.setTranslation(current_pos.x, current_pos.y, current_pos.z);
}

void EntityPlayer::setScale(float scale)
{
    player_scale = scale;
    updateModelMatrix();
}

void EntityPlayer::setPosition(const Vector3& new_position)
{
    model.setTranslation(new_position.x, new_position.y, new_position.z);
    updateModelMatrix();
}

void EntityPlayer::updateModelMatrix()
{
    Vector3 pos = model.getTranslation();

    model.setIdentity();
    model.translate(pos.x, pos.y, pos.z);
    model.rotate(current_yaw, Vector3(0, 1, 0));
    model.scale(player_scale, player_scale, player_scale);
}

void EntityPlayer::checkCollisions(const std::vector<Entity*>& entities)
{
    float player_radius = player_scale * 0.5f;
    Vector3 position = model.getTranslation();

    // Reset grounded state
    is_grounded = false;

    // Simplified collision resolution (2 iterations max)
    const int iterations = 2;

    for(int iter = 0; iter < iterations; ++iter)
    {
        bool collision_found = false;

        for(Entity* entity : entities) {
            if (entity == this) continue;

            // Use framework collision for ALL entities
            std::vector<sCollisionData> collisions;
            if(Collision::TestEntitySphere(entity, player_radius, position, collisions, eCollisionFilter::ALL)) {
                for(const sCollisionData& col : collisions) {
                    if(!col.collided) continue;

                    Vector3 collision_normal = col.col_normal;
                    float penetration = player_radius - col.distance;

                    // Push out of collision
                    position += collision_normal * penetration;

                    // Project velocity to slide along surface
                    float v_dot_n = velocity.dot(collision_normal);
                    if(v_dot_n < 0) {
                        velocity -= collision_normal * v_dot_n;
                    }

                    // Ground detection
                    if(collision_normal.y > 0.7f) {
                        is_grounded = true;
                        if(velocity.y < 0) velocity.y = 0;
                    }
                    // Ceiling detection
                    else if(collision_normal.y < -0.7f) {
                        if(velocity.y > 0) velocity.y = 0;
                    }

                    collision_found = true;
                    break; // Handle one collision per entity per iteration
                }
            }
        }

        if(!collision_found) break;
    }

    // Update model matrix with corrected position
    model.setTranslation(position.x, position.y, position.z);
}
