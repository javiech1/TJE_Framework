#include "entity_player.h"
#include "entity_platform.h"
#include "game/world/world.h"
#include "framework/input.h"
#include "game/game.h"
#include "framework/collision.h"
#include "graphics/mesh.h"
#include <cmath>
#include <algorithm>

static bool sphereVsAABB(const Vector3& sphere_center, float sphere_radius,
                         const Vector3& box_center, const Vector3& box_half_size,
                         Vector3& closest_point, float& penetration)
{
    closest_point.x = std::max(box_center.x - box_half_size.x,
                               std::min(sphere_center.x, box_center.x + box_half_size.x));
    closest_point.y = std::max(box_center.y - box_half_size.y,
                               std::min(sphere_center.y, box_center.y + box_half_size.y));
    closest_point.z = std::max(box_center.z - box_half_size.z,
                               std::min(sphere_center.z, box_center.z + box_half_size.z));

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
    position = Vector3(0, 0, 0);
    speed = 12.0f;
    jump_velocity = 9.0f;
    velocity = Vector3(0,0,0);

    is_grounded = false;
    jump_was_pressed = false;
    jump_requested = false;

    player_scale = 0.4f;
    current_yaw = 0.0f;
    target_yaw = 0.0f;
    world = nullptr;

    model.setIdentity();
    updateModelMatrix();
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
    applyPhysics(delta_time);

    float rotation_speed = 15.0f;
    float yaw_diff = target_yaw - current_yaw;

    while (yaw_diff > M_PI) yaw_diff -= 2.0f * M_PI;
    while (yaw_diff < -M_PI) yaw_diff += 2.0f * M_PI;

    current_yaw += yaw_diff * std::min(1.0f, rotation_speed * delta_time);
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

        if (wall_jump_momentum_timer <= 0.0f) {
            velocity.x = move_dir.x * speed;
            velocity.z = move_dir.z * speed;
        }

        target_yaw = atan2(move_dir.x, move_dir.z);
    }
    else if (wall_jump_momentum_timer <= 0.0f) {
        velocity.x = 0;
        velocity.z = 0;
    }
}

void EntityPlayer::applyPhysics(float delta_time)
{
    if (wall_jump_cooldown > 0.0f) wall_jump_cooldown -= delta_time;
    if (wall_jump_momentum_timer > 0.0f) wall_jump_momentum_timer -= delta_time;

    if (wall_cling_timer > 0.0f) {
        wall_cling_timer -= delta_time;
        if (wall_cling_timer > 0.0f && !is_touching_wall) {
            is_touching_wall = true;
        }
    }

    if (jump_requested) {
        if (is_grounded) {
            velocity.y = jump_velocity;
            is_grounded = false;
            ground_platform = nullptr;
            notifyJump();
        }
        else if (is_touching_wall && wall_jump_cooldown <= 0.0f) {
            velocity.y = jump_velocity * 0.9f;
            velocity.x = wall_normal.x * WALL_JUMP_HORIZONTAL;
            velocity.z = wall_normal.z * WALL_JUMP_HORIZONTAL;
            wall_jump_cooldown = WALL_JUMP_COOLDOWN;
            wall_jump_momentum_timer = WALL_JUMP_MOMENTUM_LOCK;
            notifyJump();
        }
        jump_requested = false;
    }

    float gravity = world ? world->getGravity() : 9.8f;
    velocity.y -= gravity * delta_time;

    if (is_grounded) {
        float friction = 5.0f;
        float damping = std::max(0.0f, 1.0f - (friction * delta_time));
        velocity.x *= damping;
        velocity.z *= damping;
    }

    position += velocity * delta_time;

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
    position = new_position;
    updateModelMatrix();
}

void EntityPlayer::updateModelMatrix()
{
    model.setIdentity();
    model.translate(position.x, position.y, position.z);
    model.rotate(current_yaw, Vector3(0, 1, 0));
    model.scale(player_scale, player_scale, player_scale);
}

void EntityPlayer::detectGround(const std::vector<Entity*>& entities)
{
    float collision_radius = player_scale * COLLISION_RADIUS_MULT;
    is_grounded = false;
    ground_platform = nullptr;

    Vector3 ray_dir(0, -1, 0);
    float ray_distance = collision_radius * 1.5f;
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

                if (hit_platform && hit_platform->isTwin() && !hit_platform->isTwinActive()) {
                    continue;
                }

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
                    if (hit_ghost) continue;
                }

                is_grounded = true;
                if (hit_platform && hit_platform->isMoving()) {
                    ground_platform = hit_platform;
                }
                break;
            }
        }
    }
}

void EntityPlayer::resolveCollisions(const std::vector<Entity*>& entities)
{
    float collision_radius = player_scale * COLLISION_RADIUS_MULT;
    is_touching_wall = false;

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

                if (hit_platform && hit_platform->isTwin() && !hit_platform->isTwinActive()) {
                    continue;
                }

                if (!hit_platform) {
                    bool hit_ghost = false;
                    for (Entity* entity : entities) {
                        EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
                        if (!platform) continue;
                        if (!platform->isTwin() || platform->isTwinActive()) continue;

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
                    if (hit_ghost) continue;
                }

                is_grounded = true;
                float ground_y = ground_hit.col_point.y + collision_radius;
                float height_error = fabs(position.y - ground_y);

                if (height_error < 0.1f && velocity.y <= 0.0f) {
                    position.y = ground_y;
                    velocity.y = 0;
                }
                break;
            }
        }
    }

    const int max_iterations = 3;
    for (int iter = 0; iter < max_iterations; ++iter) {
        bool collision_found = false;

        for (Entity* entity : entities) {
            EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
            if (!platform) continue;
            if (platform->isTwin() && !platform->isTwinActive()) continue;

            Vector3 box_center = platform->model.getTranslation();
            Vector3 box_half_size = platform->getHalfSize();

            float platform_top = box_center.y + box_half_size.y;
            bool player_above_platform = position.y > platform_top + 0.01f;

            if (!player_above_platform) {
                float min_half_height = collision_radius + 0.5f;
                if (box_half_size.y < min_half_height) {
                    box_half_size.y = min_half_height;
                }
            }

            Vector3 closest_point;
            float penetration;

            if (sphereVsAABB(position, collision_radius, box_center, box_half_size,
                           closest_point, penetration)) {
                Vector3 push_direction = position - closest_point;

                if (push_direction.length() < 0.001f) {
                    float dist_to_x_min = fabs(position.x - (box_center.x - box_half_size.x));
                    float dist_to_x_max = fabs(position.x - (box_center.x + box_half_size.x));
                    float dist_to_y_min = fabs(position.y - (box_center.y - box_half_size.y));
                    float dist_to_y_max = fabs(position.y - (box_center.y + box_half_size.y));
                    float dist_to_z_min = fabs(position.z - (box_center.z - box_half_size.z));
                    float dist_to_z_max = fabs(position.z - (box_center.z + box_half_size.z));

                    float min_x = std::min(dist_to_x_min, dist_to_x_max);
                    float min_y = std::min(dist_to_y_min, dist_to_y_max);
                    float min_z = std::min(dist_to_z_min, dist_to_z_max);

                    if (min_x <= min_y && min_x <= min_z) {
                        push_direction = Vector3(position.x > box_center.x ? 1 : -1, 0, 0);
                    } else if (min_y <= min_z) {
                        push_direction = Vector3(0, position.y > box_center.y ? 1 : -1, 0);
                    } else {
                        push_direction = Vector3(0, 0, position.z > box_center.z ? 1 : -1);
                    }
                    penetration = collision_radius;
                } else {
                    push_direction.normalize();
                }

                if (push_direction.y > GROUND_NORMAL_THRESHOLD) {
                    is_grounded = true;
                }
                else if (push_direction.y < -GROUND_NORMAL_THRESHOLD) {
                    position += push_direction * (penetration + 0.001f);
                    if (velocity.y > 0) velocity.y = 0;
                    collision_found = true;
                }
                else {
                    position += push_direction * (penetration + 0.001f);
                    is_touching_wall = true;
                    wall_normal = push_direction;
                    wall_cling_timer = WALL_CLING_TIME;

                    Vector3 horizontal_push = Vector3(push_direction.x, 0, push_direction.z);
                    float h_len = horizontal_push.length();
                    if (h_len > 0.001f) {
                        horizontal_push = horizontal_push * (1.0f / h_len);
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

    if (!is_touching_wall) {
        float proximity_radius = collision_radius * 1.2f;

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
                    to_player = to_player * (1.0f / dist);
                    if (fabs(to_player.y) < GROUND_NORMAL_THRESHOLD) {
                        is_touching_wall = true;
                        wall_normal = to_player;
                        wall_cling_timer = WALL_CLING_TIME;
                        break;
                    }
                }
            }
        }
    }

    updateModelMatrix();
}

void EntityPlayer::settleToGround(const std::vector<Entity*>& entities)
{
    float collision_radius = player_scale * COLLISION_RADIUS_MULT;
    const float SPAWN_RAY_DISTANCE = 2.0f;

    Vector3 ray_dir(0, -1, 0);
    sCollisionData ground_hit;
    ground_hit.distance = std::numeric_limits<float>::max();

    if (Collision::TestSceneRay(entities, position, ray_dir, ground_hit,
                                 eCollisionFilter::FLOOR, true, SPAWN_RAY_DISTANCE)) {
        if (ground_hit.collided) {
            float ground_y = ground_hit.col_point.y + collision_radius;
            position.y = ground_y;
            velocity.y = 0;
            is_grounded = true;
            updateModelMatrix();
        }
    }
}
