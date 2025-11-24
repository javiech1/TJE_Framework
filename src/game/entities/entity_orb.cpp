#include "entity_orb.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/utils.h"
#include "game/game.h"

EntityOrb::EntityOrb() : EntityMesh()
{
    isCollected = false;
    radius = 0.5f;
    rotation_angle = 0.0f;
    scale_factor = 0.05f;  // Visible collectible size (~4-6 units diameter)
    position = Vector3(0,0,0);
    color = Vector4(1.0f, 0.84f, 0.0f, 1.0f); // Gold color
    mesh = Mesh::Get("data/meshes/box.ASE");
    shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    texture = nullptr; // No texture, will use color
}

EntityOrb::~EntityOrb()
{

}

void EntityOrb::render(Camera* camera)
{
    if(isCollected || !visible) return; //no render if collected

    if(!mesh || !shader) return;

    // Custom render with our color
    shader->enable();
    shader->setUniform("u_model", model);
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    shader->setUniform("u_color", color);
    shader->setUniform("u_time", Game::instance->time);

    mesh->render(GL_TRIANGLES);
    shader->disable();
}

void EntityOrb::update(float delta_time)
{
    if(isCollected) return;

    // Continuous rotation for diamond effect
    rotation_angle += delta_time * 3.0f; // Rotation speed

    // Rebuild transformation matrix
    model.setIdentity();

    // Apply scale to make it smaller and slightly elongated vertically for diamond effect
    model.m[0] = scale_factor * 0.8f;  // Slightly narrower
    model.m[5] = scale_factor * 1.2f;  // Taller for diamond shape
    model.m[10] = scale_factor * 0.8f; // Slightly narrower

    // Apply rotations to create proper diamond shape
    model.rotate(45.0f * DEG2RAD, Vector3(1, 0, 0)); // Tilt forward to show corner
    model.rotate(45.0f * DEG2RAD, Vector3(0, 0, 1)); // Rotate to diamond orientation
    model.rotate(rotation_angle, Vector3(0, 1, 0)); // Spin around Y axis

    // Apply position
    model.m[12] = position.x;
    model.m[13] = position.y;
    model.m[14] = position.z;
}

void EntityOrb::setPosition(const Vector3& pos)
{
    position = pos;
    model.m[12] = position.x;
    model.m[13] = position.y;
    model.m[14] = position.z;
}