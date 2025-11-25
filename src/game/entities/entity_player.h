#pragma once
#include "framework/entities/entity_mesh.h"
#include <functional>

class Camera;
class World;
class EntityPlatform;

class EntityPlayer : public EntityMesh {
    private:
        Vector3 position;
        Vector3 velocity;
        float speed;
        float jump_velocity;

        bool is_grounded;
        bool jump_was_pressed;
        bool jump_requested;

        bool is_touching_wall = false;
        Vector3 wall_normal = Vector3(0,0,0);
        float wall_jump_cooldown = 0.0f;

        float player_scale;
        float current_yaw;
        float target_yaw;
        World* world;
        EntityPlatform* ground_platform = nullptr;

        static constexpr float GROUND_NORMAL_THRESHOLD = 0.7f;
        static constexpr float COLLISION_RADIUS_MULT = 0.5f;

        static constexpr float WALL_JUMP_COOLDOWN = 0.15f;
        static constexpr float WALL_JUMP_HORIZONTAL = 12.0f;
        static constexpr float WALL_JUMP_MOMENTUM_LOCK = 0.30f;
        static constexpr float WALL_CLING_TIME = 0.12f;

        float wall_jump_momentum_timer = 0.0f;
        float wall_cling_timer = 0.0f;

        std::function<void()> on_jump_callback = nullptr;
        void notifyJump() { if (on_jump_callback) on_jump_callback(); }

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

        void detectGround(const std::vector<Entity*>& entities);
        void resolveCollisions(const std::vector<Entity*>& entities);
        void settleToGround(const std::vector<Entity*>& entities);

        void setOnJumpCallback(std::function<void()> callback) { on_jump_callback = callback; }
};
