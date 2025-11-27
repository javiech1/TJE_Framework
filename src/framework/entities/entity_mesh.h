#pragma once

#include "entity.h"
#include "graphics/material.h"

class Mesh;
class Shader;

class EntityMesh : public Entity {

public:
	Mesh* mesh = nullptr;
	Material material;

	bool isInstanced = false;
	std::vector<Matrix44> models;  // For multiple instances

	EntityMesh();
	EntityMesh(Mesh* mesh, Material mat);
	virtual ~EntityMesh() {};

	void render(Camera* camera) override;
	void update(float delta_time) override;
};
