#pragma once

#include "framework/entities/entity_mesh.h"

class EntityOrb : public EntityMesh {
    private:
        bool isCollected = false;
        float radius = 0.5f;

    public:
        EntityOrb();
        virtual ~EntityOrb();

        virtual void render(Camera* camera) override;
        virtual void update(float delta_time) override;

        bool getIsCollected() const { return isCollected; }
        void collect() { isCollected = true; }

        void setPosition(const Vector3& pos);

};