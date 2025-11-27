#include "entity_collider.h"

#include "graphics/mesh.h"

EntityCollider::EntityCollider()
{
}

EntityCollider::EntityCollider(Mesh* mesh, Material mat, int layer)
	: EntityMesh(mesh, mat), layer(layer)
{
}

void EntityCollider::setupCollision(bool is_static)
{
	if (mesh) {
		mesh->createCollisionModel(is_static);
	}
}
