#pragma once
#include "framework/entities/entity_collider.h"

class Camera;
class Texture;
class Shader;

// Reuse MovementType from entity_platform.h
enum class ObstacleMovementType { NONE, LINEAR, CIRCULAR };

class EntityObstacle : public EntityCollider {
    private:
        Vector3 half_size;
        Vector3 scale_dimensions;
        Vector3 position;

        // Movement system
        ObstacleMovementType movement_type = ObstacleMovementType::LINEAR;
        Vector3 start_position;
        Vector3 end_position;
        Vector3 center_position;
        float orbit_radius = 0.0f;
        float movement_speed = 1.0f;
        float movement_time = 0.0f;
        float movement_phase = 0.0f;

    public:
        Shader* shader = nullptr;
        Vector4 color;  // Red translucent by default

        EntityObstacle();
        virtual ~EntityObstacle();

        virtual void render(Camera* camera) override;
        virtual void update(float delta_time) override;

        void setScale(Vector3 dimensions);
        void setPosition(const Vector3& new_position);
        Vector3 getHalfSize() const { return half_size; }
        Vector3 getPosition() const { return position; }

        // Movement setup
        void setLinearMovement(const Vector3& start, const Vector3& end, float speed, float phase = 0.0f);
        void setCircularMovement(const Vector3& center, float radius, float speed, float phase = 0.0f);

        // Collision check
        bool collidesWithPlayer(const Vector3& player_pos, float player_radius);
};
