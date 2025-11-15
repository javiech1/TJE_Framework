#include "entity_skybox.h"
#include "graphics/shader.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "framework/camera.h"
// Note: OpenGL (glDepthFunc) comes from entity.h -> includes.h -> SDL_opengl.h

EntitySkybox::EntitySkybox() {
    //empty becasue parent constructor does the job
}

EntitySkybox::~EntitySkybox() {
    //empty
}

void EntitySkybox::render(Camera* camera) {

    //validate, if something is missing, skip rendering
    if(!mesh || !shader || !visible) return;

    //configure OpenGl for skybox
    glDepthFunc(GL_LEQUAL); //allow depth = 1.0

    //activate shader and pass uniforms
    shader->enable();
    //pass matrices
    shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
    //pass camera position for correct skybox rendering
    shader->setUniform("u_camera_pos", camera->eye);
    //pass texture
    shader->setUniform("u_texture", texture);

    //render mesh
    mesh->render(GL_TRIANGLES);

    //disable shader
    shader->disable();

    //restore OpenGL 
    glDepthFunc(GL_LESS); //restore default

}