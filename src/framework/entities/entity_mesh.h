#pragma once

#include "entity.h"

class Mesh;
class Texture;
class Shader;

class EntityMesh : public Entity {
    public:
        Mesh* mesh = nullptr;
        Texture* texture = nullptr;
        Shader* shader = nullptr;

        //instancing
        bool isInstanced = false;
        std::vector<Matrix44> models;

        EntityMesh();
        virtual ~EntityMesh();

        virtual void render(Camera* camera) override;
        void renderInstanced(Camera* camera);
};