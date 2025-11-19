#include "entity_orb.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"

EntityOrb::EntityOrb() : EntityMesh()
{
    isCollected = false;
    radius = 0.5f;
    mesh = Mesh::Get("data/meshes/box.ASE");
    shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    texture = Texture::Get("data/textures/texture.tga");
}

EntityOrb::~EntityOrb()
{

}

void EntityOrb::render(Camera* camera)
{
    if(isCollected) return; //no render if collected

    EntityMesh::render(camera);
}

void EntityOrb::update(float delta_time)
{
    //rotation animation
    model.rotate( float(delta_time) * 2.0f, Vector3(0,1,0) );
}

void EntityOrb::setPosition(const Vector3& pos)
{
    model.setTranslation(pos.x, pos.y, pos.z);
}