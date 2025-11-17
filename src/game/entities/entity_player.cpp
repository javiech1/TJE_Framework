#include "entity_player.h"
#include "framework/input.h"

EntityPlayer::EntityPlayer() : EntityMesh()
{
    speed = 10.0f;
    jump_force = 15.0f;
    is_grounded = false;
    velocity = Vector3(0,0,0);
    camera = nullptr;
    player_scale = 1.0f;
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

    // Get current position before updating
    Vector3 position = model.getTranslation();

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

    // Rebuild the transformation matrix: scale first, then translate so translation stays unscaled
    model.setIdentity();
    model.scale(player_scale, player_scale, player_scale);
    model.translate(position);
}

void EntityPlayer::setScale(float scale)
{
    player_scale = scale;
    // Rebuild matrix with new scale respecting translation
    Vector3 position = model.getTranslation();
    model.setIdentity();
    model.scale(scale, scale, scale);
    model.translate(position);
}
