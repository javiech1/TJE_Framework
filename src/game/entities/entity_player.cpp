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

        // Don't override horizontal velocity during wall jump momentum lock
        if (wall_jump_momentum_timer <= 0.0f) {
            velocity.x = move_dir.x * speed;
            velocity.z = move_dir.z * speed;
        }

        // Calculate target rotation based on movement direction
        target_yaw = atan2(move_dir.x, move_dir.z);
    }
    else if (wall_jump_momentum_timer <= 0.0f)
    {
        // Only stop if not in wall jump momentum
        velocity.x = 0;
        velocity.z = 0;
    }
}

void EntityPlayer::applyPhysics(float delta_time)
{
    // Update wall jump cooldown
    if (wall_jump_cooldown > 0.0f) {
        wall_jump_cooldown -= delta_time;
    }

    // Update wall jump momentum timer
    if (wall_jump_momentum_timer > 0.0f) {
        wall_jump_momentum_timer -= delta_time;
    }

    // Update wall cling timer - keeps is_touching_wall true for input buffer
    if (wall_cling_timer > 0.0f) {
        wall_cling_timer -= delta_time;
        // Wall cling allows wall jump even after leaving wall
        // is_touching_wall was set in resolveCollisions, cling keeps it valid
        if (wall_cling_timer > 0.0f && !is_touching_wall) {
            // Note: wall_normal retained from last wall contact
            is_touching_wall = true;
        }
    }

    // Process jump (ground jump OR wall jump)
    if (jump_requested) {
        if (is_grounded) {
            // Normal ground jump
            velocity.y = jump_velocity;
            is_grounded = false;
            ground_platform = nullptr;
            notifyJump();  // Notify for twin platforms
            std::cout << "JUMP! velocity.y = " << velocity.y << std::endl;
        }
        else if (is_touching_wall && wall_jump_cooldown <= 0.0f) {
            // Wall jump - push away from wall + up
            velocity.y = jump_velocity * 0.9f;  // Slightly less vertical than ground jump
            velocity.x = wall_normal.x * WALL_JUMP_HORIZONTAL;
            velocity.z = wall_normal.z * WALL_JUMP_HORIZONTAL;
            wall_jump_cooldown = WALL_JUMP_COOLDOWN;
            wall_jump_momentum_timer = WALL_JUMP_MOMENTUM_LOCK;  // Lock input briefly
            notifyJump();  // Notify for twin platforms
            std::cout << "WALL JUMP! normal=(" << wall_normal.x << "," << wall_normal.z << ")" << std::endl;
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

    // Apply moving platform carry - move with the platform
    if (is_grounded && ground_platform) {
        Vector3 platform_vel = ground_platform->getVelocity();
        position += platform_vel * delta_time;
    }
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
// Also tracks which platform we're standing on for platform carry
// ============================================================================
void EntityPlayer::detectGround(const std::vector<Entity*>& entities)
{
    float collision_radius = player_scale * COLLISION_RADIUS_MULT;

    // Reset grounded state and platform reference
    is_grounded = false;
    ground_platform = nullptr;

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
                // DON'T set is_grounded yet! First find which platform we hit
                // and check if it's a ghost twin (which shouldn't count as ground)

                EntityPlatform* hit_platform = nullptr;
                for (Entity* entity : entities) {
                    EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
                    if (!platform) continue;

                    Vector3 plat_pos = platform->model.getTranslation();
                    Vector3 plat_half = platform->getHalfSize();

                    // Check if the hit point is within this platform's bounds (with tolerance)
                    bool in_x = ground_hit.col_point.x >= plat_pos.x - plat_half.x - 0.1f &&
                                ground_hit.col_point.x <= plat_pos.x + plat_half.x + 0.1f;
                    bool in_z = ground_hit.col_point.z >= plat_pos.z - plat_half.z - 0.1f &&
                                ground_hit.col_point.z <= plat_pos.z + plat_half.z + 0.1f;
                    bool near_y = fabs(ground_hit.col_point.y - (plat_pos.y + plat_half.y)) < 0.2f;

                    if (in_x && in_z && near_y) {
                        hit_platform = platform;
                        break;
                    }
                }

                // Skip ghost twin platforms - they're not real ground!
                if (hit_platform && hit_platform->isTwin() && !hit_platform->isTwinActive()) {
                    continue;  // Try next ray offset, this one hit a ghost
                }

                // SECONDARY CHECK: If we couldn't identify the platform, check if ANY ghost twin
                // contains the hit point (handles wall-shaped platforms where bounds check fails)
                if (!hit_platform) {
                    bool hit_ghost = false;
                    for (Entity* entity : entities) {
                        EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
                        if (!platform) continue;
                        if (!platform->isTwin() || platform->isTwinActive()) continue;  // Only check ghosts

                        Vector3 plat_pos = platform->model.getTranslation();
                        Vector3 plat_half = platform->getHalfSize();

                        // Expanded bounds check for ghost detection
                        bool in_x = ground_hit.col_point.x >= plat_pos.x - plat_half.x - 0.5f &&
                                    ground_hit.col_point.x <= plat_pos.x + plat_half.x + 0.5f;
                        bool in_z = ground_hit.col_point.z >= plat_pos.z - plat_half.z - 0.5f &&
                                    ground_hit.col_point.z <= plat_pos.z + plat_half.z + 0.5f;
                        bool in_y = ground_hit.col_point.y >= plat_pos.y - plat_half.y - 0.5f &&
                                    ground_hit.col_point.y <= plat_pos.y + plat_half.y + 0.5f;

                        if (in_x && in_z && in_y) {
                            hit_ghost = true;
                            break;
                        }
                    }
                    if (hit_ghost) {
                        continue;  // Skip this ray, it hit a ghost
                    }
                }

                // Valid ground found!
                is_grounded = true;

                // Store moving platform for carry
                if (hit_platform && hit_platform->isMoving()) {
                    ground_platform = hit_platform;
                }

                break; // Found valid ground
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

    // Reset wall contact each frame (will be set if collision detected)
    is_touching_wall = false;

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
                // Find which platform we hit - check for ghost twins
                EntityPlatform* hit_platform = nullptr;
                for (Entity* entity : entities) {
                    EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
                    if (!platform) continue;

                    Vector3 plat_pos = platform->model.getTranslation();
                    Vector3 plat_half = platform->getHalfSize();

                    bool in_x = ground_hit.col_point.x >= plat_pos.x - plat_half.x - 0.1f &&
                                ground_hit.col_point.x <= plat_pos.x + plat_half.x + 0.1f;
                    bool in_z = ground_hit.col_point.z >= plat_pos.z - plat_half.z - 0.1f &&
                                ground_hit.col_point.z <= plat_pos.z + plat_half.z + 0.1f;
                    bool near_y = fabs(ground_hit.col_point.y - (plat_pos.y + plat_half.y)) < 0.2f;

                    if (in_x && in_z && near_y) {
                        hit_platform = platform;
                        break;
                    }
                }

                // Skip ghost twin platforms - don't snap to them!
                if (hit_platform && hit_platform->isTwin() && !hit_platform->isTwinActive()) {
                    continue;  // Try next ray offset
                }

                // SECONDARY CHECK: If we couldn't identify the platform, check if ANY ghost twin
                // contains the hit point (handles wall-shaped platforms where bounds check fails)
                if (!hit_platform) {
                    bool hit_ghost = false;
                    for (Entity* entity : entities) {
                        EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
                        if (!platform) continue;
                        if (!platform->isTwin() || platform->isTwinActive()) continue;  // Only check ghosts

                        Vector3 plat_pos = platform->model.getTranslation();
                        Vector3 plat_half = platform->getHalfSize();

                        // Expanded bounds check for ghost detection
                        bool in_x = ground_hit.col_point.x >= plat_pos.x - plat_half.x - 0.5f &&
                                    ground_hit.col_point.x <= plat_pos.x + plat_half.x + 0.5f;
                        bool in_z = ground_hit.col_point.z >= plat_pos.z - plat_half.z - 0.5f &&
                                    ground_hit.col_point.z <= plat_pos.z + plat_half.z + 0.5f;
                        bool in_y = ground_hit.col_point.y >= plat_pos.y - plat_half.y - 0.5f &&
                                    ground_hit.col_point.y <= plat_pos.y + plat_half.y + 0.5f;

                        if (in_x && in_z && in_y) {
                            hit_ghost = true;
                            break;
                        }
                    }
                    if (hit_ghost) {
                        continue;  // Skip this ray, it hit a ghost
                    }
                }

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

            // Skip inactive twin platforms (ghost platforms have no collision)
            if (platform->isTwin() && !platform->isTwinActive()) continue;

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
                    // Push toward NEAREST FACE (not center!)
                    // Calculate distance to each face
                    float dist_to_x_min = fabs(position.x - (box_center.x - box_half_size.x));
                    float dist_to_x_max = fabs(position.x - (box_center.x + box_half_size.x));
                    float dist_to_y_min = fabs(position.y - (box_center.y - box_half_size.y));
                    float dist_to_y_max = fabs(position.y - (box_center.y + box_half_size.y));
                    float dist_to_z_min = fabs(position.z - (box_center.z - box_half_size.z));
                    float dist_to_z_max = fabs(position.z - (box_center.z + box_half_size.z));

                    float min_x = std::min(dist_to_x_min, dist_to_x_max);
                    float min_y = std::min(dist_to_y_min, dist_to_y_max);
                    float min_z = std::min(dist_to_z_min, dist_to_z_max);

                    // Push toward the closest face
                    if (min_x <= min_y && min_x <= min_z) {
                        // X face is closest - push horizontally
                        push_direction = Vector3(position.x > box_center.x ? 1 : -1, 0, 0);
                    } else if (min_y <= min_z) {
                        // Y face is closest - push vertically
                        push_direction = Vector3(0, position.y > box_center.y ? 1 : -1, 0);
                    } else {
                        // Z face is closest - push in depth
                        push_direction = Vector3(0, 0, position.z > box_center.z ? 1 : -1);
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

                    // Track wall contact for wall jumping
                    is_touching_wall = true;
                    wall_normal = push_direction;  // Points away from wall
                    wall_cling_timer = WALL_CLING_TIME;  // Reset cling timer for input buffer

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

    // --- PROXIMITY-BASED WALL DETECTION (doesn't require penetration) ---
    // This allows wall jump when sliding alongside walls, not just pushing into them
    // Uses expanded radius to detect "near wall" state
    if (!is_touching_wall) {
        float proximity_radius = collision_radius * 1.2f;  // 20% larger for detection

        for (Entity* entity : entities) {
            EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
            if (!platform) continue;
            if (platform->isTwin() && !platform->isTwinActive()) continue;

            Vector3 box_center = platform->model.getTranslation();
            Vector3 box_half_size = platform->getHalfSize();

            Vector3 closest_point;
            float penetration;

            if (sphereVsAABB(position, proximity_radius, box_center, box_half_size,
                           closest_point, penetration)) {
                Vector3 to_player = position - closest_point;
                float dist = to_player.length();
                if (dist > 0.001f) {
                    to_player = to_player * (1.0f / dist);  // normalize
                    // Only count as wall if horizontal (not ground/ceiling)
                    if (fabs(to_player.y) < GROUND_NORMAL_THRESHOLD) {
                        is_touching_wall = true;
                        wall_normal = to_player;
                        wall_cling_timer = WALL_CLING_TIME;
                        break;  // Found a wall, stop searching
                    }
                }
            }
        }
    }

    // Sync model matrix with corrected position
    updateModelMatrix();
}

// ============================================================================
// settleToGround() - One-time spawn settling
// Called after loadLevel() or reset() to snap player to ground from spawn height
// Uses longer ray than normal gameplay to handle elevated spawn positions
// ============================================================================
void EntityPlayer::settleToGround(const std::vector<Entity*>& entities)
{
    float collision_radius = player_scale * COLLISION_RADIUS_MULT;

    // Spawn settling ray - long enough to detect ground from spawn heights up to ~2 units
    // Normal gameplay uses collision_radius * 1.5 = 0.3 units
    // Spawn settling uses 2.0 units to handle elevated spawns
    const float SPAWN_RAY_DISTANCE = 2.0f;

    Vector3 ray_dir(0, -1, 0);  // Straight down
    sCollisionData ground_hit;
    ground_hit.distance = std::numeric_limits<float>::max();

    if (Collision::TestSceneRay(entities, position, ray_dir, ground_hit,
                                 eCollisionFilter::FLOOR, true, SPAWN_RAY_DISTANCE)) {
        if (ground_hit.collided) {
            // Snap to ground surface + collision radius
            float ground_y = ground_hit.col_point.y + collision_radius;
            position.y = ground_y;
            velocity.y = 0;
            is_grounded = true;
            updateModelMatrix();
            std::cout << "Spawn settled to ground at Y=" << ground_y << std::endl;
        }
    }
}
