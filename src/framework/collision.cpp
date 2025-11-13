#include "collision.h"
#include "framework/entities/entity_collider.h"
#include "graphics/mesh.h"
#include <assert.h>

void Collision::TestEntitySphereWithModel(EntityCollider* collider, const Matrix44& m, int model_index, float radius, const Vector3& center, std::vector<sCollisionData>& collisions)
{
	assert(collider && collider->mesh);

	Vector3 collision_point;
	Vector3 collision_normal;

	if (collider->mesh->testSphereCollision(m, center, radius, collision_point, collision_normal)) {
		sCollisionData data = {
			.col_point = collision_point,
			.col_normal = collision_normal.normalize(),
			.distance = center.distance(collision_point),
			.collided = true,
			.collider = collider
		};
		collisions.push_back(data);
	}
}

bool Collision::TestEntitySphere(Entity* e, float radius, const Vector3& center, std::vector<sCollisionData>& collisions, eCollisionFilter filter)
{
	// Check if the entity is a collider and passes the filter test
	EntityCollider* collider = dynamic_cast<EntityCollider*>(e);
	if (collider == nullptr || !(collider->layer & filter)) {
		return false;
	}

	if (!collider->isInstanced)
	{
		TestEntitySphereWithModel(collider, collider->model, -1, radius, center, collisions);
	}
	else
	{
		for (int i = 0; i < collider->models.size(); ++i)
		{
			TestEntitySphereWithModel(collider, collider->models[i], i, radius, center, collisions);
		}
	}

	return !collisions.empty();
}

bool Collision::TestEntityRayWithModel(EntityCollider* collider, const Matrix44& m, const Vector3& origin, const Vector3& direction, sCollisionData& collision_data, float max_ray_dist)
{
	Vector3 col_point;
	Vector3 col_normal;

	if (!collider->mesh->testRayCollision(m, origin, direction, col_point, col_normal, max_ray_dist)) {
		return false;
	}

	// There was a collision! Update if nearest..
	float new_distance = (col_point - origin).length();
	if (new_distance < collision_data.distance) {
		collision_data.distance = new_distance;
		collision_data.collided = true;
		collision_data.collider = collider;
		collision_data.col_point = col_point;
		collision_data.col_normal = col_normal;
	}

	return true;
}

bool Collision::TestEntityRay(Entity* e, const Vector3& origin, const Vector3& direction, sCollisionData& collision_data,
	int layer, bool closest, float max_ray_dist)
{
	// Check if the entity is a collider and passes the filter test
	EntityCollider* ec = dynamic_cast<EntityCollider*>(e);
	if (ec == nullptr || !(ec->layer & layer)) {
		return false;
	}

	bool collided = false;

	if (!ec->isInstanced) {

		collided |= TestEntityRayWithModel(ec, ec->model, origin, direction, collision_data, max_ray_dist);
	}
	else {
		for (const Matrix44& model : ec->models)
		{
			collided |= TestEntityRayWithModel(ec, model, origin, direction, collision_data, max_ray_dist);
				
			if (collided && !closest) {
				return true;
			}
		}
	}

	return collided;
}

bool Collision::TestSceneRay(const std::vector<Entity*>& entities, const Vector3& origin, const Vector3& direction, sCollisionData& collision_data, 
													int layer, bool closest, float max_ray_dist)
{
	bool collided = false;

	for (auto e : entities)
	{
		collided |= TestEntityRay(e, origin, direction, collision_data, layer, closest, max_ray_dist);

		if (collided && !closest) {
			return true;
		}
	}

	return collided;
}