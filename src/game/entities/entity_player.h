#pragma once
#include "framework/entities/entity_mesh.h"

class Camera;

class EntityPlayer : public EntityMesh {
    private:     
        Vector3 velocity;
        float speed;
        float jump_force;
        bool is_grounded;
        Camera* camera;

    public:

        EntityPlayer();
        virtual ~EntityPlayer();

        virtual void update(float delta_time) override;
        void handleInput(float delta_time);
        void applyPhysics(float delta_time);

};
