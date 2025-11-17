#include "entity_player.h"
#include "framework/input.h"

EntityPlayer::EntityPlayer() : EntityMesh()
{
    speed = 10.0f;
    jump_force = 15.0f;
    is_grounded = false;
    velocity = Vector3(0,0,0);
    position = Vector3(0,0,0);
    camera = nullptr;
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

    Vector3 direction = Vector3(0,0,0);
    if (Input::isKeyPressed(SDL_SCANCODE_W)) direction += Vector3(0,0,1);
    if (Input::isKeyPressed(SDL_SCANCODE_S)) direction += Vector3(0,0,-1);
    if (Input::isKeyPressed(SDL_SCANCODE_A)) direction += Vector3(-1,0,0);
    if (Input::isKeyPressed(SDL_SCANCODE_D)) direction += Vector3(1,0,0);
    if (Input::wasKeyPressed(SDL_SCANCODE_SPACE) && is_grounded)
    {
        velocity.y = jump_force;
        is_grounded = false;
    }

    if (direction.length() > 0)
    {
        direction.normalize();
        velocity.x = direction.x * speed;
        velocity.z = direction.z * speed;
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
