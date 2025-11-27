#pragma once

#include "entity_mesh.h"
#include "collision/collision.h"

class EntityCollider : public EntityMesh {

public:
	int layer = eCollisionFilter::SCENARIO;

	EntityCollider();
	EntityCollider(Mesh* mesh, Material mat, int layer = eCollisionFilter::SCENARIO);
	virtual ~EntityCollider() {};

	void setupCollision(bool is_static = true);
};
