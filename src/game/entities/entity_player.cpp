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
    speed = 10.0f;
    jump_velocity = 8.0f;  // Direct jump velocity (gravity-independent)
    velocity = Vector3(0,0,0);
    position = Vector3(0,0,0);

    // Jump state
    is_grounded = false;
    was_grounded_last_frame = false;
    jump_pressed_last_frame = false;
    jump_buffer_time = 0.0f;
    coyote_time = 0.0f;

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
    // New update order:
    // 1. Handle input (sets movement velocity)
    handleInput(delta_time);

    // 2. Pre-physics (movement without gravity)
    prePhysicsUpdate(delta_time);

    // Note: Collision detection will be called from World::update()
    // between prePhysicsUpdate and postPhysicsUpdate

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

    // Jump with SPACE - with buffering and coyote time
    bool jump_pressed_now = Input::isKeyPressed(SDL_SCANCODE_SPACE);

    // Update jump buffer when space is pressed
    if (jump_pressed_now && !jump_pressed_last_frame) {
        jump_buffer_time = JUMP_BUFFER_DURATION;
    }

    // Decrease buffer timer
    if (jump_buffer_time > 0) {
        jump_buffer_time -= delta_time;
    }

    // Update coyote timer
    if (was_grounded_last_frame && !is_grounded) {
        // Just left the ground - start coyote timer
        coyote_time = COYOTE_TIME_DURATION;
    } else if (coyote_time > 0) {
        coyote_time -= delta_time;
    }

    // Can jump if: grounded OR within coyote time
    bool can_jump = is_grounded || coyote_time > 0;

    // Execute jump if buffer exists and can jump
    if (jump_buffer_time > 0 && can_jump) {
        // Use direct jump velocity (gravity-independent for consistent feel)
        velocity.y = jump_velocity;
        is_grounded = false;
        jump_buffer_time = 0;  // Clear the buffer
        coyote_time = 0;  // Clear coyote time

        float gravity = world ? world->getGravity() : 9.8f;
        std::cout << "JUMP! velocity.y = " << velocity.y << " (gravity=" << gravity << ")" << std::endl;
    }

    // Debug output when space is pressed
    if (jump_pressed_now && !jump_pressed_last_frame) {
        std::cout << "Space pressed! is_grounded = " << is_grounded
                  << ", can_jump = " << can_jump
                  << ", coyote_time = " << coyote_time << std::endl;
    }

    // Update state for next frame
    jump_pressed_last_frame = jump_pressed_now;

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

void EntityPlayer::prePhysicsUpdate(float delta_time)
{
    // Apply horizontal movement only (no gravity yet)
    // This is called before collision detection
    Vector3 movement = Vector3(velocity.x, 0, velocity.z) * delta_time;
    position += movement;

    rebuildModelMatrix();
}

void EntityPlayer::postPhysicsUpdate(float delta_time)
{
    // Apply gravity and vertical movement AFTER collision detection
    // Only apply gravity if not grounded
    if (!is_grounded) {
        float gravity = world ? world->getGravity() : 9.8f;
        velocity.y -= gravity * delta_time;
    } else {
        // Apply small damping to prevent micro-bounces
        if (std::abs(velocity.y) < 0.1f) {
            velocity.y = 0;
        }
    }

    // Apply vertical movement
    position.y += velocity.y * delta_time;

    rebuildModelMatrix();
}

void EntityPlayer::applyPhysics(float delta_time)
{
    // Legacy method - now split into pre and post physics
    // Keep for compatibility if needed
    prePhysicsUpdate(delta_time);
    postPhysicsUpdate(delta_time);
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
    // Proper collision radius for stability
    float player_radius = player_scale * 0.8f;
    const float MAX_CORRECTION = 0.5f;

    // Track previous grounded state for coyote time
    was_grounded_last_frame = is_grounded;
    is_grounded = false;

    // === RAY-BASED GROUND DETECTION (More precise) ===
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

            // Stop downward velocity
            if (velocity.y <= 0) {
                velocity.y = 0;
            }

            // Snap to ground surface
            if (ground_distance < player_radius - GROUND_TOLERANCE) {
                // We're inside the ground, push up
                position.y = ground_hit.col_point.y + player_radius;
            }
            else if (ground_distance > player_radius && was_grounded_last_frame) {
                // We're slightly above ground but were grounded, pull down for stability
                position.y = ground_hit.col_point.y + player_radius;
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

    // Additional ground stability when consistently grounded
    if(is_grounded && was_grounded_last_frame) {
        // Small downward bias to maintain ground contact
        position.y -= 0.002f;
    }

    rebuildModelMatrix();
}