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
        float jump_velocity;    // Direct jump velocity

        // Jump state
        bool is_grounded;
        bool jump_was_pressed;  // To prevent repeated jumps while holding button
        bool jump_requested;    // Jump input from handleInput to applyPhysics

        // Player properties
        float player_scale;
        float current_yaw;      // Current rotation angle (radians)
        float target_yaw;       // Target rotation angle (radians)
        World* world;           // Reference to world for gravity

        // Constants
        static constexpr float GROUND_TOLERANCE = 0.02f;      // 2cm ground detection tolerance

        void rebuildModelMatrix();

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
        void resetVelocity() { velocity = Vector3(0, 0, 0); }
        void checkCollisions(const std::vector<Entity*>& entities);

};
