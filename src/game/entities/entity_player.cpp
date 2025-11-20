#include "entity_player.h"
#include "framework/input.h"
#include "game/game.h"
#include "game/entities/entity_platform.h"
#include <iostream>

EntityPlayer::EntityPlayer() : EntityMesh()
{
    speed = 10.0f;
    jump_force = 5.0f;  // Reduced for more realistic jump height
    is_grounded = false;
    velocity = Vector3(0,0,0);
    position = Vector3(0,0,0);
    player_scale = 1.0f;
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
    handleInput(delta_time);
    applyPhysics(delta_time);
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
    // Jump with SPACE - using edge detection to jump only once per press
    static bool space_held = false;
    if (Input::isKeyPressed(SDL_SCANCODE_SPACE))
    {
        if (!space_held && is_grounded) {
            velocity.y = jump_force;
            is_grounded = false;
        }
        space_held = true;
    } else {
        space_held = false;
    }

    if (move_dir.length() > 0)
    {
        move_dir.normalize();
        velocity.x = move_dir.x * speed;
        velocity.z = move_dir.z * speed;
    } else
    {
        velocity.x = 0;
        velocity.z = 0;
    }
}

void EntityPlayer::applyPhysics(float delta_time)
{
    // Apply gravity
    velocity.y -= 9.81f * delta_time;

    // Update position based on velocity
    position += velocity * delta_time;

    // Reset grounded state (will be set by checkCollisions if on platform)
    is_grounded = false;

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
    model.m[0] = player_scale;
    model.m[5] = player_scale;
    model.m[10] = player_scale;
    model.m[12] = position.x;
    model.m[13] = position.y;
    model.m[14] = position.z;
}

void EntityPlayer::checkCollisions(const std::vector<Entity*>& entities)
{
    Vector3 player_center = position;
    // box.ASE mesh is ~100 units (from -50 to 50), so at scale 1.0, half-size is 50
    float player_half_width = player_scale * 50.0f;
    float player_half_height = player_scale * 50.0f;

    // Store old position for rollback if needed
    Vector3 old_position = position;

    //check platforms
    for(Entity* entity : entities) {
        EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
        if(!platform) continue;

        Vector3 platform_center = platform->model.getTranslation();
        Vector3 platform_half_size = platform->getHalfSize();

        // Calculate overlaps on each axis
        float overlapX = (player_half_width + platform_half_size.x) - abs(player_center.x - platform_center.x);
        float overlapY = (player_half_height + platform_half_size.y) - abs(player_center.y - platform_center.y);
        float overlapZ = (player_half_width + platform_half_size.z) - abs(player_center.z - platform_center.z);

        // Check if there's a collision (all overlaps are positive)
        if(overlapX > 0 && overlapY > 0 && overlapZ > 0) {
            // Find the axis with smallest overlap (shortest separation distance)
            if(overlapX < overlapY && overlapX < overlapZ) {
                // Separate on X axis
                if(player_center.x < platform_center.x) {
                    position.x = platform_center.x - platform_half_size.x - player_half_width;
                } else {
                    position.x = platform_center.x + platform_half_size.x + player_half_width;
                }
                velocity.x = 0;
            }
            else if(overlapZ < overlapY) {
                // Separate on Z axis
                if(player_center.z < platform_center.z) {
                    position.z = platform_center.z - platform_half_size.z - player_half_width;
                } else {
                    position.z = platform_center.z + platform_half_size.z + player_half_width;
                }
                velocity.z = 0;
            }
            else {
                // Separate on Y axis
                if(player_center.y < platform_center.y) {
                    // Hit from below
                    position.y = platform_center.y - platform_half_size.y - player_half_height;
                    velocity.y = 0; // Stop upward movement
                } else {
                    // Landing on top
                    position.y = platform_center.y + platform_half_size.y + player_half_height;
                    velocity.y = 0;
                    is_grounded = true;
                }
            }

            // Update player center for next iteration
            player_center = position;
            rebuildModelMatrix();
        }
    }
}