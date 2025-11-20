#pragma once

#include "framework/entities/entity_mesh.h"

class EntityOrb : public EntityMesh {
    private:
        bool isCollected = false;
        float radius = 0.5f;
        float rotation_angle = 0.0f;
        float scale_factor = 0.004f; // Smaller than player (player is 0.01)
        Vector3 position;
        Vector4 color;

    public:
        EntityOrb();
        virtual ~EntityOrb();

        virtual void render(Camera* camera) override;
        virtual void update(float delta_time) override;

        bool getIsCollected() const { return isCollected; }
        void collect() { isCollected = true; visible = false; }
        void reset() { isCollected = false; visible = true; }

        void setPosition(const Vector3& pos);
        Vector3 getPosition() const { return position; }

};