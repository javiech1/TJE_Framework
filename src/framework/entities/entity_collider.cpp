#include "entity_collider.h"
#include "graphics/mesh.h"

EntityCollider::EntityCollider() {
    mesh = nullptr;
    layer = eCollisionFilter::NONE;
    isInstanced = false;
}

EntityCollider::~EntityCollider() {
    // dont delete mesh (shared)
}