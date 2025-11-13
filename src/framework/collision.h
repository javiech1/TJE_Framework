#pragma once

#include "includes.h"
#include "framework.h"

class Entity;
class EntityCollider;
class Mesh;

// Edit as needed...
enum eCollisionFilter {
	NONE = 0,
	FLOOR = 1 << 0,
	WALL = 1 << 1,
	PLAYER = 1 << 2,
	ENEMY = 1 << 3,
	SCENARIO = FLOOR | WALL,
	ALL = 0xFF
};

struct sCollisionData {
	Vector3 col_point;
	Vector3 col_normal;
	float distance = 3.4e+38F;
	bool collided = false;
	EntityCollider* collider = nullptr;
};

struct sActiveCollisionData {
	sCollisionData data;
	Matrix44 model;
	int model_index = -1;
};

class Collision {

	static void TestEntitySphereWithModel(EntityCollider* collider, const Matrix44& m, int model_index, float radius, const Vector3& center, std::vector<sCollisionData>& collisions);
	static bool TestEntityRayWithModel(EntityCollider* collider, const Matrix44& m, const Vector3& origin, const Vector3& direction, sCollisionData& collision_data, float max_ray_dist = 3.4e+38F);

public:

	static bool TestEntitySphere(Entity* e, float radius, const Vector3& center, std::vector<sCollisionData>& collisions, eCollisionFilter filter);
	static bool TestEntityRay(Entity* e, const Vector3& origin, const Vector3& direction, sCollisionData& collision_data, int layer = eCollisionFilter::ALL, bool closest = false, float max_ray_dist = 3.4e+38F);
	static bool TestSceneRay(const std::vector<Entity*>& entities, const Vector3& origin, const Vector3& direction, sCollisionData& collision_data, int layer = eCollisionFilter::ALL, bool closest = false, float max_ray_dist = 3.4e+38F);
};