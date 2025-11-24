#pragma once
#include "framework/entities/entity_collider.h"

class Camera;
class Shader;

class EntityResetSlab : public EntityCollider {
    private:
        Vector3 half_size;
        float pulse_time;  // For pulsing glow animation

    public:
        Shader* shader = nullptr;
        Vector4 color;  // Semi-transparent red

        EntityResetSlab();
        virtual ~EntityResetSlab();

        virtual void render(Camera* camera) override;
        virtual void update(float delta_time) override;
        void setScale(Vector3 dimensions);
        void setPosition(const Vector3& new_position);
        Vector3 getHalfSize() const { return half_size; }
        bool collidesWithPlayer(const Vector3& player_pos, float player_radius) const;
};
