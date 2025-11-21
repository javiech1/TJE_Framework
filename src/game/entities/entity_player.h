#pragma once
#include "framework/entities/entity_mesh.h"

class Camera;

class EntityPlayer : public EntityMesh {
    private:
        Vector3 velocity;
        Vector3 position;
        float speed;
        float jump_force;
        bool is_grounded;
        bool jump_pressed_last_frame;  // Track jump button state for edge detection
        float player_scale;
        float current_yaw;      // Current rotation angle (radians)
        float target_yaw;       // Target rotation angle (radians)
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
        Vector3 getPosition() const { return position; }
        float getScale() const { return player_scale; }
        void checkCollisions(const std::vector<Entity*>& entities);

};
