#include "entity_mesh.h"

#include "framework/camera.h"
#include "graphics/mesh.h"
#include "graphics/shader.h"
#include "graphics/texture.h"

EntityMesh::EntityMesh()
{
}

EntityMesh::EntityMesh(Mesh* mesh, Material mat)
	: mesh(mesh), material(mat)
{
}

void EntityMesh::render(Camera* camera)
{
	if (!mesh || !material.shader) {
		Entity::render(camera);
		return;
	}

	Shader* shader = material.shader;
	shader->enable();

	shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	shader->setUniform("u_color", material.color);

	if (material.diffuse) {
		shader->setUniform("u_texture", material.diffuse, 0);
	}

	if (isInstanced && !models.empty()) {
		mesh->renderInstanced(GL_TRIANGLES, models.data(), (int)models.size());
	}
	else {
		shader->setUniform("u_model", getGlobalMatrix());
		mesh->render(GL_TRIANGLES);
	}

	shader->disable();

	Entity::render(camera);
}

void EntityMesh::update(float delta_time)
{
	Entity::update(delta_time);
}
