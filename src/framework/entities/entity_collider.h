#pragma once
#include "entity.h"
#include "framework/collision.h"  // Para eCollisionFilter

class Mesh;

class EntityCollider : public Entity {
   public:
        Mesh* mesh = nullptr;
        eCollisionFilter layer = NONE;

        //Instancing
        bool isInstanced = false;
        std::vector<Matrix44> models;

        EntityCollider();
        virtual ~EntityCollider();
};