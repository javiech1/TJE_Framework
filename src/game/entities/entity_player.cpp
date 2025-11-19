#include "entity_player.h"
#include "framework/input.h"
#include "game/game.h"
#include "game/entities/entity_platform.h"

EntityPlayer::EntityPlayer() : EntityMesh()
{
    speed = 10.0f;
    jump_force = 15.0f;
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
    bool moveForward = Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP);
    bool moveBackward = Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN);
    bool moveLeft = Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT);
    bool moveRight = Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT);

    if (moveForward) move_dir += forward;
    if (moveBackward) move_dir -= forward;
    if (moveLeft) move_dir += right;
    if (moveRight) move_dir -= right;
    if (Input::wasKeyPressed(SDL_SCANCODE_SPACE) && is_grounded)
    {
        velocity.y = jump_force;
        is_grounded = false;
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

    // Simple ground collision
    float ground_y = player_scale * 0.5f; // El centro del cubo debe estar a media altura sobre el suelo

    if(position.y <= ground_y) {
        position.y = ground_y;
        velocity.y = 0;
        is_grounded = true;
    } else {
        is_grounded = false;
    }

    rebuildModelMatrix();

    //player-platform collision

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
    //player is a cube for now
    float player_half_width = player_scale * 0.5f;
    float player_half_height = player_scale * 0.5f;


    //check platforms
    for(Entity* entity : entities) {
        EntityPlatform* platform = dynamic_cast<EntityPlatform*>(entity);
        if(!platform) continue;

        Vector3 platform_center = platform->model.getTranslation();
        Vector3 platform_half_size = platform->getHalfSize();

        //check collision X and Z;
        bool collisionX = abs(player_center.x - platform_center.x) < (player_half_width + platform_half_size.x);
        bool collisionZ = abs(player_center.z - platform_center.z) < (player_half_width + platform_half_size.z);

        if(collisionX && collisionZ) {
            //if horizantal alignment, check Y
            float distY = player_center.y - platform_center.y;
            float limitY = player_half_height + platform_half_size.y;

            //if just on top
            if(distY > 0 && distY < limitY && velocity.y <= 0) {
                //snap to platform top
                position.y = platform_center.y + limitY;
                velocity.y = 0;
                is_grounded = true;
            }
        }
    }
}