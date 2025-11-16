#include "entity_player.h"
#include "framework/input.h"

EntityPlayer::EntityPlayer() : EntityMesh()
{
    speed = 10.0f;
    jump_force = 15.0f;
    is_grounded = false;
    velocity = Vector3(0,0,0);
    camera = nullptr;
}

EntityPlayer::~EntityPlayer()
{

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

    // Update position
    model.translate(velocity * delta_time);

    // Simple ground collision
    Vector3 position = model.getTranslation();
    if(position.y <= 0) { //y position
        position.y = 0;
        model.setTranslation(position.x, position.y, position.z);
        velocity.y = 0;
        is_grounded = true;

    } else {
        is_grounded = false;
    }
}
