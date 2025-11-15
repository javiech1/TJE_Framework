#pragma once

#include "entity_mesh.h"

class EntitySkybox : public EntityMesh {
    public:
        EntitySkybox();
        virtual ~EntitySkybox();

        virtual void render(Camera* camera) override;

};