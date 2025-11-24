#pragma once

#include "framework/entities/entity_collider.h"

class Shader;
class Texture;

class EntityOrb : public EntityCollider {
    private:
        bool isCollected = false;
        float radius = 0.5f;
        float rotation_angle = 0.0f;
        float scale_factor = 1.5f;  // Visual scale for rendering
        Vector3 position;
        Vector4 color;

    public:
        Shader* shader = nullptr;
        Texture* texture = nullptr;

        EntityOrb();
        virtual ~EntityOrb();

        virtual void render(Camera* camera) override;
        virtual void update(float delta_time) override;

        bool getIsCollected() const { return isCollected; }
        void collect() { isCollected = true; visible = false; }
        void reset() { isCollected = false; visible = true; }

        void setPosition(const Vector3& pos);
        Vector3 getPosition() const { return position; }
        float getRadius() const { return scale_factor * 0.5f; }

};
