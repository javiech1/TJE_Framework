#include "entity_player.h"
#include "game/world/world.h"
#include "framework/input.h"
#include "game/game.h"
#include "game/entities/entity_platform.h"
#include "framework/collision.h"
#include <iostream>
#include <cmath>

EntityPlayer::EntityPlayer() : EntityMesh()
{
    // Movement
    speed = 6.5f;  // Reduced from 10.0f for more precise control (max ~10 unit horizontal jumps)
    jump_velocity = 7.5f;  // Balanced for 2.87 unit max height (challenging but achievable)
    velocity = Vector3(0,0,0);
    position = Vector3(0,0,0);

    // Jump state
    is_grounded = false;
    jump_was_pressed = false;

    // Player properties
    player_scale = 1.0f;
    current_yaw = 0.0f;    // Start facing forward (positive Z)
    target_yaw = 0.0f;
    world = nullptr;  // Will be set by World when player is created

    rebuildModelMatrix();
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
    // Simple update order:
    // 1. Handle input (sets movement velocity and jump)
    handleInput(delta_time);

    // Note: World will call applyPhysics and checkCollisions

    // Smooth rotation towards target (mechanical/robotic style)
    float rotation_speed = 15.0f; // Fast rotation for robotic feel
    float yaw_diff = target_yaw - current_yaw;

    // Normalize angle difference to [-PI, PI] for shortest rotation path
    while (yaw_diff > M_PI) yaw_diff -= 2.0f * M_PI;
    while (yaw_diff < -M_PI) yaw_diff += 2.0f * M_PI;

    // Interpolate rotation (clamped to avoid overshooting)
    current_yaw += yaw_diff * std::min(1.0f, rotation_speed * delta_time);
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
    // Only use WASD for player movement (arrows are for camera control)
    bool moveForward = Input::isKeyPressed(SDL_SCANCODE_W);
    bool moveBackward = Input::isKeyPressed(SDL_SCANCODE_S);
    bool moveLeft = Input::isKeyPressed(SDL_SCANCODE_A);
    bool moveRight = Input::isKeyPressed(SDL_SCANCODE_D);

    if (moveForward) move_dir += forward;
    if (moveBackward) move_dir -= forward;
    if (moveLeft) move_dir += right;
    if (moveRight) move_dir -= right;

    // Note: Jump is handled in applyPhysics after collision detection

    if (move_dir.length() > 0)
    {
        move_dir.normalize();
        velocity.x = move_dir.x * speed;
        velocity.z = move_dir.z * speed;

        // Calculate target rotation based on movement direction
        // atan2 gives us the angle in radians
        target_yaw = atan2(move_dir.x, move_dir.z);
    } else
    {
        velocity.x = 0;
        velocity.z = 0;
    }
}

void EntityPlayer::applyPhysics(float delta_time)
{
    // Check for jump input (after collision detection has set is_grounded)
    bool jump_pressed = Input::isKeyPressed(SDL_SCANCODE_SPACE);

    // Jump only on button press (not held) and when grounded
    if (jump_pressed && !jump_was_pressed && is_grounded) {
        velocity.y = jump_velocity;
        is_grounded = false;  // Immediately mark as not grounded when jumping
        std::cout << "JUMP! velocity.y = " << velocity.y << std::endl;
    }
    jump_was_pressed = jump_pressed;  // Update for next frame

    // Get gravity value
    float gravity = world ? world->getGravity() : 9.8f;

    // Only apply gravity if not grounded or if jumping up
    if (!is_grounded || velocity.y > 0) {
        velocity.y -= gravity * delta_time;
    } else {
        // When grounded and not jumping, ensure no downward velocity
        velocity.y = 0;
    }

    // Apply all movement (horizontal and vertical)
    position += velocity * delta_time;

    rebuildModelMatrix();
}

void EntityPlayer::setScale(float scale)
{
    player_scale = scale;
    rebuildModelMatrix();
}

void EntityPlayer::setPosition(const Vector3& new_position)
{
    position = new_position;
    rebuildModelMatrix();
}

void EntityPlayer::rebuildModelMatrix()
{
    model.setIdentity();

    // First apply translation
    model.translate(position.x, position.y, position.z);

    // Then apply rotation around Y axis
    model.rotate(current_yaw, Vector3(0, 1, 0));

    // Finally apply scale
    model.scale(player_scale, player_scale, player_scale);
}

void EntityPlayer::checkCollisions(const std::vector<Entity*>& entities)
{
    // Reduced collision radius to prevent floating
    float player_radius = player_scale * 0.5f;
    const float MAX_CORRECTION = 0.5f;

    // Reset grounded state
    is_grounded = false;

    // === RAY-BASED GROUND DETECTION ===
    // Cast a ray downward from player center
    Vector3 ray_origin = position;
    Vector3 ray_direction(0.0f, -1.0f, 0.0f);
    sCollisionData ground_hit;
    ground_hit.distance = FLT_MAX;  // Initialize to max distance

    // Use framework's ray casting API for precise ground detection
    bool hit_ground = Collision::TestSceneRay(entities, ray_origin, ray_direction,
                                              ground_hit, eCollisionFilter::FLOOR,
                                              true, player_radius + 0.2f);

    if (hit_ground && ground_hit.collided) {
        float ground_distance = ground_hit.distance;

        // Check if we're close enough to the ground
        if (ground_distance <= player_radius + GROUND_TOLERANCE) {
            is_grounded = true;

            // Only snap if we're significantly off from ideal position
            // Subtract offset to make player visually sit on platform
            float ideal_height = ground_hit.col_point.y + player_radius - 0.1f;
            if (std::abs(position.y - ideal_height) > GROUND_TOLERANCE * 0.5f) {
                position.y = ideal_height;
            }
        }
    }

    // === SPHERE COLLISION FOR WALLS AND CEILINGS ===
    Vector3 player_center = position;

    for(Entity* entity : entities) {
        std::vector<sCollisionData> collisions;

        if(Collision::TestEntitySphere(entity, player_radius, player_center, collisions, eCollisionFilter::FLOOR)) {
            for(const sCollisionData& col : collisions) {
                if(!col.collided) continue;

                float penetration = player_radius - col.distance;

                // Only handle wall and ceiling collisions here (ground handled by ray)
                if(col.col_normal.y <= 0.7f && penetration > GROUND_TOLERANCE) {
                    // Ceiling collision (normal pointing downward)
                    if(col.col_normal.y < -0.7f) {
                        float correction = std::min(penetration, MAX_CORRECTION);
                        position.y -= correction;
                        if(velocity.y > 0) {
                            velocity.y = 0;
                        }
                    }
                    // Wall collision (normal mostly horizontal)
                    else {
                        float correction = std::min(penetration, MAX_CORRECTION);
                        position += col.col_normal * correction;

                        // Remove velocity component toward wall
                        float dot = velocity.dot(col.col_normal);
                        if(dot < 0) {
                            velocity = velocity - col.col_normal * dot;
                        }
                    }

                    // Update player center for next iteration
                    player_center = position;
                }
            }
        }
    }

    rebuildModelMatrix();
}