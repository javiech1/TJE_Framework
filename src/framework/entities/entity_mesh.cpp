#include "entity_mesh.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "framework/camera.h"

EntityMesh::EntityMesh() {
    mesh = nullptr;
    texture = nullptr;
    shader = nullptr;
    isInstanced = false;
}

void EntityMesh::render(Camera* camera) {
    if(!mesh || !shader || !visible) return;
    if(isInstanced) {
        renderInstanced(camera);
        return;
    }
    shader->enable();
    shader->setUniform("u_model", model);
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    if(texture) {
        shader->setUniform("u_texture", texture);
    }
    mesh->render(GL_TRIANGLES);
    shader->disable();

    //render children
    Entity::render(camera);
}

void EntityMesh::renderInstanced(Camera* camera) {
    if(!mesh || !shader || !visible || models.empty()) return;
    shader->enable();
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    if(texture) {
        shader->setUniform("u_texture", texture);
    }
    mesh->renderInstanced(GL_TRIANGLES, models.data(), int(models.size()));
    shader->disable();
}