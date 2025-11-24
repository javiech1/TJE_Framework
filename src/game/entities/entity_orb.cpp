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
    scale_factor = 1.5f;  // Radius 1 (Diameter 2) * 2.5 = 5 units
    position = Vector3(0,0,0);
    color = Vector4(1.0f, 1.0f, 1.0f, 1.0f); // White so texture shows
    mesh = Mesh::Get("data/meshes/sphere.obj");
    shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    texture = Texture::Get("data/textures/orb.png");
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
    
    if(texture) {
        shader->setUniform("u_texture", texture, 0);
    }

    mesh->render(GL_TRIANGLES);
    shader->disable();
}

void EntityOrb::update(float delta_time)
{
    if(isCollected) return;

    // Continuous rotation
    rotation_angle += delta_time * 1.0f; // Rotation speed

    // Rebuild transformation matrix
    model.setIdentity();
    model.translate(position.x, position.y, position.z);
    model.rotate(rotation_angle, Vector3(0, 1, 0)); // Spin around Y axis
    model.scale(scale_factor, scale_factor, scale_factor);
}

void EntityOrb::setPosition(const Vector3& pos)
{
    position = pos;
    model.m[12] = position.x;
    model.m[13] = position.y;
    model.m[14] = position.z;
}
