#include "entity_player.h"
#include "entity_platform.h"
#include "game/world/world.h"
#include "framework/input.h"
#include "game/game.h"
#include "framework/collision.h"
#include "graphics/mesh.h"
#include <iostream>
#include <cmath>
#include <algorithm>

// Helper: Sphere vs AABB collision (bypasses COLDET)
// Returns true if collision, sets closest_point and penetration
static bool sphereVsAABB(const Vector3& sphere_center, float sphere_radius,
                         const Vector3& box_center, const Vector3& box_half_size,
                         Vector3& closest_point, float& penetration)
{
    // Find closest point on AABB to sphere center
    closest_point.x = std::max(box_center.x - box_half_size.x,
                               std::min(sphere_center.x, box_center.x + box_half_size.x));
    closest_point.y = std::max(box_center.y - box_half_size.y,
                               std::min(sphere_center.y, box_center.y + box_half_size.y));
    closest_point.z = std::max(box_center.z - box_half_size.z,
                               std::min(sphere_center.z, box_center.z + box_half_size.z));

    // Calculate distance from sphere center to closest point
    Vector3 diff = sphere_center - closest_point;
    float distance_sq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;

    if (distance_sq < sphere_radius * sphere_radius) {
        float distance = sqrt(distance_sq);
        penetration = sphere_radius - distance;
        return true;
    }
    return false;
}

EntityPlayer::EntityPlayer() : EntityMesh()
{
    // Movement
    position = Vector3(0, 0, 0);  // Will be set by World::loadLevel()
    speed = 12.0f;
    jump_velocity = 9.0f;
    velocity = Vector3(0,0,0);

    // Jump state
    is_grounded = false;
    jump_was_pressed = false;
    jump_requested = false;

    // Player properties
    player_scale = 0.4f;
    current_yaw = 0.0f;
    target_yaw = 0.0f;
    world = nullptr;

    // Initialize model matrix with identity and apply default scale
    model.setIdentity();
    updateModelMatrix();  // Apply initial scale to model matrix
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
    // NOTE: Input is handled by World::update() BEFORE checkCollisions()
    // This ensures is_grounded is correct when applyPhysics() runs

    // 1. Apply physics (gravity, movement, friction)
    // is_grounded was updated by checkCollisions() called from World
    applyPhysics(delta_time);

    // 2. Smooth rotation towards target
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

    // Update position with velocity
    // Don't touch model matrix here - updateModelMatrix() will rebuild it properly
    position += velocity * delta_time;
}

void EntityPlayer::setScale(float scale)
{
    player_scale = scale;
    updateModelMatrix();
}

void EntityPlayer::setPosition(const Vector3& new_position)
{
    position = new_position;  // Update position member variable
    updateModelMatrix();      // Rebuild model matrix with scale and rotation
}

void EntityPlayer::updateModelMatrix()
{
    // Use position member variable instead of reading from model
    // This ensures we preserve scale and rotation when rebuilding the matrix
    model.setIdentity();
    model.translate(position.x, position.y, position.z);
    model.rotate(current_yaw, Vector3(0, 1, 0));
    model.scale(player_scale, player_scale, player_scale);
}

// ============================================================================
// detectGround() - Updates is_grounded state BEFORE physics
// Called BEFORE applyPhysics() so friction/jumping work correctly
// ============================================================================
void EntityPlayer::detectGround(const std::vector<Entity*>& entities)
{
    float collision_radius = player_scale * COLLISION_RADIUS_MULT;

    // Reset grounded state
    is_grounded = false;

    Vector3 ray_dir(0, -1, 0);  // Straight down
    float ray_distance = collision_radius * 1.5f;

    // Check 5 points: center + 4 edge positions (for edge detection)
    float offset = collision_radius * 0.85f;
    Vector3 ray_offsets[] = {
        Vector3(0, 0, 0),              // Center
        Vector3(offset, 0, 0),         // Right
        Vector3(-offset, 0, 0),        // Left
        Vector3(0, 0, offset),         // Forward
        Vector3(0, 0, -offset)         // Back
    };

    for(const Vector3& offset_vec : ray_offsets) {
        sCollisionData ground_hit;
        ground_hit.distance = std::numeric_limits<float>::max();

        Vector3 ray_origin = position + offset_vec;

        if(Collision::TestSceneRay(entities, ray_origin, ray_dir, ground_hit,
                                    eCollisionFilter::FLOOR, true, ray_distance)) {
            if(ground_hit.collided && ground_hit.distance <= ray_distance) {
                is_grounded = true;
                break; // Found ground
            }
        }
    }
}

// ============================================================================
// resolveCollisions() - Resolves penetrations AFTER physics
// Called AFTER applyPhysics() to push player out of geometry
// ============================================================================
void EntityPlayer::resolveCollisions(const std::vector<Entity*>& entities)
{
    float collision_radius = player_scale * COLLISION_RADIUS_MULT;

    // --- GROUND SNAP (vertical correction) ---
    Vector3 ray_dir(0, -1, 0);
    float ray_distance = collision_radius * 1.5f;
    float offset = collision_radius * 0.85f;

    Vector3 ray_offsets[] = {
        Vector3(0, 0, 0),
        Vector3(offset, 0, 0),
        Vector3(-offset, 0, 0),
        Vector3(0, 0, offset),
        Vector3(0, 0, -offset)
    };

    for(const Vector3& offset_vec : ray_offsets) {
        sCollisionData ground_hit;
        ground_hit.distance = std::numeric_limits<float>::max();

        Vector3 ray_origin = position + offset_vec;

        if(Collision::TestSceneRay(entities, ray_origin, ray_dir, ground_hit,
                                    eCollisionFilter::FLOOR, true, ray_distance)) {
            if(ground_hit.collided && ground_hit.distance <= ray_distance) {
                is_grounded = true;

                float ground_y = ground_hit.col_point.y + collision_radius;
                float height_error = fabs(position.y - ground_y);

                if(height_error < 0.1f && velocity.y <= 0.0f) {
                    position.y = ground_y;
                    velocity.y = 0;
                }
                break;
            }
        }
    }

    // --- WALL/CEILING COLLISION using AABB (bypasses COLDET bugs) ---
    // COLDET's rayCollision and sphereCollision don't work for horizontal contact
    // Solution: Use manual sphere-vs-AABB collision for EntityPlatform

    // Multiple iterations to resolve complex corner cases
    const int max_iterations = 3;
    for(int iter = 0; iter < max_iterations; ++iter)
    {
        bool collision_found = false;

        // Test AABB collision against all platforms
        for(Entity* entity : entities)
        {
            // Only process EntityPlatform (skip player, orbs, etc.)
            EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
            if (!platform) continue;

            // Get platform bounds
            Vector3 box_center = platform->model.getTranslation();
            Vector3 box_half_size = platform->getHalfSize();

            // Only inflate AABB when player is BELOW the platform surface
            // This prevents false "wall collisions" when walking ON TOP of platforms
            float platform_top = box_center.y + box_half_size.y;
            bool player_above_platform = position.y > platform_top + 0.01f;

            if (!player_above_platform) {
                // Inflate AABB vertically to catch lateral collisions with thin platforms
                float min_half_height = collision_radius + 0.5f;
                if (box_half_size.y < min_half_height) {
                    box_half_size.y = min_half_height;
                }
            }

            // Test sphere-vs-AABB collision
            Vector3 closest_point;
            float penetration;

            if (sphereVsAABB(position, collision_radius, box_center, box_half_size,
                           closest_point, penetration))
            {
                // Calculate push direction (from closest point toward player)
                Vector3 push_direction = position - closest_point;

                // Handle edge case: player center inside box
                if (push_direction.length() < 0.001f) {
                    // Push toward nearest face
                    Vector3 to_center = position - box_center;
                    float abs_x = fabs(to_center.x);
                    float abs_y = fabs(to_center.y);
                    float abs_z = fabs(to_center.z);

                    if (abs_x > abs_y && abs_x > abs_z) {
                        push_direction = Vector3(to_center.x > 0 ? 1 : -1, 0, 0);
                    } else if (abs_y > abs_z) {
                        push_direction = Vector3(0, to_center.y > 0 ? 1 : -1, 0);
                    } else {
                        push_direction = Vector3(0, 0, to_center.z > 0 ? 1 : -1);
                    }
                    penetration = collision_radius;  // Full radius penetration
                } else {
                    push_direction.normalize();
                }

                // Separate ground/wall/ceiling based on push direction
                if (push_direction.y > GROUND_NORMAL_THRESHOLD)
                {
                    // Ground collision - handled by ground snap
                    is_grounded = true;
                }
                else if (push_direction.y < -GROUND_NORMAL_THRESHOLD)
                {
                    // Ceiling collision
                    position += push_direction * (penetration + 0.001f);
                    if (velocity.y > 0) velocity.y = 0;
                    collision_found = true;
                }
                else
                {
                    // Wall collision (horizontal)
                    position += push_direction * (penetration + 0.001f);

                    // Slide along wall - ONLY horizontal components
                    // This prevents the jump velocity from being amplified
                    Vector3 horizontal_push = Vector3(push_direction.x, 0, push_direction.z);
                    float h_len = horizontal_push.length();
                    if (h_len > 0.001f) {
                        horizontal_push = horizontal_push * (1.0f / h_len);  // normalize
                        float v_dot_n = velocity.x * horizontal_push.x + velocity.z * horizontal_push.z;
                        if (v_dot_n < 0) {
                            velocity.x -= horizontal_push.x * v_dot_n;
                            velocity.z -= horizontal_push.z * v_dot_n;
                        }
                    }

                    collision_found = true;
                }
            }
        }

        if (!collision_found) break;
    }

    // Sync model matrix with corrected position
    updateModelMatrix();
}
