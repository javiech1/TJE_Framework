#pragma once
#include "framework/entities/entity_mesh.h"

class Camera;
class World;
class EntityPlatform;

class EntityPlayer : public EntityMesh {
    private:
        // Movement and physics
        Vector3 position;       // Player world position (separate from model matrix)
        Vector3 velocity;
        float speed;
        float jump_velocity;

        // Jump state
        bool is_grounded;
        bool jump_was_pressed;  // To prevent repeated jumps while holding button
        bool jump_requested;    // Jump input from handleInput to applyPhysics

        // Player properties
        float player_scale;
        float current_yaw;      // Current rotation angle (radians)
        float target_yaw;       // Target rotation angle (radians)
        World* world;           // Reference to world for gravity
        EntityPlatform* ground_platform = nullptr;  // Platform we're standing on (for carry)

        // Collision constants
        static constexpr float GROUND_NORMAL_THRESHOLD = 0.7f; // Normal.y threshold to consider surface as ground
        static constexpr float COLLISION_RADIUS_MULT = 0.5f;   // Unified collision radius multiplier

        void updateModelMatrix();

    public:

        EntityPlayer();
        virtual ~EntityPlayer();

        virtual void render(Camera* camera) override;
        virtual void update(float delta_time) override;
        void handleInput(float delta_time);
        void applyPhysics(float delta_time);
        void setScale(float scale);
        void setPosition(const Vector3& new_position);
        void setWorld(World* w) { world = w; }
        Vector3 getPosition() const { return position; }
        float getScale() const { return player_scale; }
        float getCollisionRadius() const { return player_scale * COLLISION_RADIUS_MULT; }
        void resetVelocity() { velocity = Vector3(0, 0, 0); }

        // Collision methods - separated for correct execution order
        void detectGround(const std::vector<Entity*>& entities);    // Updates is_grounded only
        void resolveCollisions(const std::vector<Entity*>& entities); // Resolves penetrations

};
