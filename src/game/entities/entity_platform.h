#pragma once
#include "framework/entities/entity_collider.h"

class Camera;
class Texture;
class Shader;

// Movement types for platforms
enum class MovementType { NONE, LINEAR, CIRCULAR };

class EntityPlatform : public EntityCollider {
    private:
        Vector3 half_size;
        Vector3 scale_dimensions;  // Store scale for matrix rebuilding

        // Movement system
        MovementType movement_type = MovementType::NONE;
        Vector3 start_position;      // For LINEAR: point A
        Vector3 end_position;        // For LINEAR: point B
        Vector3 center_position;     // For CIRCULAR: orbit center
        float orbit_radius = 0.0f;   // For CIRCULAR
        float movement_speed = 1.0f; // Speed multiplier
        float movement_time = 0.0f;  // Elapsed time
        float movement_phase = 0.0f; // Starting phase

    public:
        Texture* texture = nullptr;
        Shader* shader = nullptr;
        Vector4 color;  // Color for rendering with flat shader

        EntityPlatform();
        virtual ~EntityPlatform();

        virtual void render(Camera* camera) override;
        virtual void update(float delta_time) override;
        void setScale(Vector3 dimensions);
        void setPosition(const Vector3& new_position);
        Vector3 getHalfSize() const { return half_size; }

        // Movement setup methods
        void setLinearMovement(const Vector3& start, const Vector3& end, float speed, float phase = 0.0f);
        void setCircularMovement(const Vector3& center, float radius, float speed, float phase = 0.0f);
        Vector3 getCurrentPosition() const;
};
  
