#pragma once
#include "framework/entities/entity_mesh.h"

class Camera;
class World;

class EntityPlayer : public EntityMesh {
    private:
        // Movement and physics
        Vector3 velocity;
        Vector3 position;
        float speed;
        float jump_velocity;    // Direct jump velocity (replaces jump_height)

        // Jump state and buffering
        bool is_grounded;
        bool was_grounded_last_frame;
        bool jump_pressed_last_frame;  // Track jump button state for edge detection
        float jump_buffer_time;         // Jump input buffer timer
        float coyote_time;             // Grace period after leaving ground

        // Player properties
        float player_scale;
        float current_yaw;      // Current rotation angle (radians)
        float target_yaw;       // Target rotation angle (radians)
        World* world;           // Reference to world for gravity

        // Constants
        static constexpr float JUMP_BUFFER_DURATION = 0.1f;  // 100ms input buffer
        static constexpr float COYOTE_TIME_DURATION = 0.1f;  // 100ms grace period
        static constexpr float GROUND_TOLERANCE = 0.1f;      // 10cm ground detection tolerance

        void rebuildModelMatrix();

    public:

        EntityPlayer();
        virtual ~EntityPlayer();

        virtual void render(Camera* camera) override;
        virtual void update(float delta_time) override;
        void handleInput(float delta_time);
        void applyPhysics(float delta_time);
        void prePhysicsUpdate(float delta_time);  // Movement and input
        void postPhysicsUpdate(float delta_time); // Gravity and integration
        void setScale(float scale);
        void setPosition(const Vector3& new_position);
        void setWorld(World* w) { world = w; }
        Vector3 getPosition() const { return position; }
        float getScale() const { return player_scale; }
        void checkCollisions(const std::vector<Entity*>& entities);

};
