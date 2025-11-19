#pragma once
#include "framework/entities/entity_mesh.h"

class Camera;

class EntityPlatform : public EntityMesh {
    private:
        Vector3 half_size;

    public:

        EntityPlatform();
        virtual ~EntityPlatform();

        virtual void render(Camera* camera) override;
        virtual void update(float delta_time) override;
        void setScale(float scale);
        void setPosition(const Vector3& new_position);
        Vector3 getHalfSize() const { return half_size; }
};
