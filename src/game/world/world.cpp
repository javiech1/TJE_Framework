#include "game/world/world.h"
#include "framework/entities/entity.h"
#include "graphics/mesh.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include "game/entities/entity_player.h"
#include "game/entities/entity_platform.h"
#include "game/entities/entity_orb.h"
#include "framework/utils.h"
#include "framework/audio.h"

World::World()
{
    //create player entity
    EntityPlayer* player = new EntityPlayer();
    player->mesh = Mesh::Get("data/meshes/box.ASE");
    player->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/texture.fs");
    player->texture = Texture::Get("data/textures/texture.tga");
    const float scale = 0.01f;  // Scale down 100-unit mesh to 1 unit
    player->setScale(scale);
    // Start player above the platform so it falls down
    player->setPosition(Vector3(0.0f, 2.0f, 0.0f));
    this->player = player;
    entities.push_back(player);
}

World::~World()
{
    // Stop background music if playing
    if (music_channel) {
        Audio::Stop(music_channel);
        music_channel = 0;
    }

    for( Entity* entity : entities )
    {
        delete entity;
    }
    entities.clear();
}

void World::render(Camera* camera)
{
    for (Entity* entity : entities)
    {
        entity->render(camera);
    }

    drawText(10, 35, "Orbs collected: " + std::to_string(orbs_collected) + "/3", Vector3(1,1,1), 2);

}

void World::update(float delta_time)
{
    for (Entity* entity : entities)
    {
        entity->update(delta_time);
    }

    for (EntityOrb* orb : orbs) {
        if(!orb->getIsCollected()) {
            float distance = player->distance(orb);
            if (distance < 1.0f) {
                orb->collect();
                orbs_collected++;
                //TODO: add effect or smth
                std::cout << "Orb collected!" << std::endl;
            }
        }
    }
    //check player collisions
    player->checkCollisions(entities);
}

void World::onKeyDown(SDL_KeyboardEvent event)
{
    if (event.keysym.sym == SDLK_r) {
        reset();
    }
}

void World::onKeyUp(SDL_KeyboardEvent event)
{ 

}

void World::onMouseMove(SDL_MouseMotionEvent event)
{

}

Vector3 World::getPlayerPosition() const {
    return player ? player->getPosition() : Vector3();
}

float World::getPlayerScale() const {
    return player ? player->getScale() : 1.0f;
}

void World::initTutorial() {
    //create platform
    EntityPlatform* platform_ground = new EntityPlatform();
    platform_ground->mesh = Mesh::Get("data/meshes/box.ASE");
    platform_ground->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    platform_ground->texture = nullptr;  // No texture, using solid color
    platform_ground->color = Vector4(0.2f, 0.4f, 0.8f, 1.0f);  // Blue color
    platform_ground->setScale(Vector3(0.2f, 0.01f, 0.2f));  // 20×1×20 units platform
    platform_ground->setPosition(Vector3(0.0f, -0.5f, 0.0f));  // Top at y=0
    entities.push_back(platform_ground);


    EntityPlatform* platform_stair1 = new EntityPlatform();
    platform_stair1->mesh = Mesh::Get("data/meshes/box.ASE");
    platform_stair1->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    platform_stair1->texture = nullptr;  // No texture, using solid color
    platform_stair1->color = Vector4(0.8f, 0.4f, 0.2f, 1.0f); 
    platform_stair1->setScale(Vector3(0.1f, 0.01f, 0.1f)); //10x1x10 units
    platform_stair1->setPosition(Vector3(0.0f, 0.5f, -20.0f));
    entities.push_back(platform_stair1);
    
    EntityPlatform* platform_stair2 = new EntityPlatform();
    platform_stair2->mesh = Mesh::Get("data/meshes/box.ASE");
    platform_stair2->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
    platform_stair2->texture = nullptr;  // No texture, using solid color
    platform_stair2->color = Vector4(0.8f, 0.4f, 0.2f, 1.0f);
    platform_stair2->setScale(Vector3(0.1f, 0.01f, 0.1f)); //10x1x10 units
    platform_stair2->setPosition(Vector3(0.0f, 1.5f, -35.0f));
    entities.push_back(platform_stair2);
    
    
    //create orbs
    // Orb 1: On main platform (platform top at y=0)
    EntityOrb* orb1 = new EntityOrb();
    orb1->setPosition(Vector3(0.0f, 0.7f, 0.0f));  // Well above ground platform
    entities.push_back(orb1);
    orbs.push_back(orb1);

    // Orb 2: On first stair platform (platform top at y=1.0)
    EntityOrb* orb2 = new EntityOrb();
    orb2->setPosition(Vector3(0.0f, 1.7f, -20.0f));  // Well above platform_stair1
    entities.push_back(orb2);
    orbs.push_back(orb2);

    // Orb 3: On second stair platform (platform top at y=2.0)
    EntityOrb* orb3 = new EntityOrb();
    orb3->setPosition(Vector3(0.0f, 2.7f, -35.0f));  // Well above platform_stair2
    entities.push_back(orb3);
    orbs.push_back(orb3);

    // Play background music in loop
    music_channel = Audio::Play("data/audio/stellar_drift.mp3", 0.5f, BASS_SAMPLE_LOOP);
    if (music_channel) {
        std::cout << "Tutorial music started!" << std::endl;
    } else {
        std::cout << "Warning: Could not play background music" << std::endl;
    }
}

void World::reset()
{
    // Reset player position
    player->setPosition(Vector3(0.0f, 2.0f, 0.0f));

    // Reset orbs collected counter
    orbs_collected = 0;

    // Reset orbs to not collected
    for (EntityOrb* orb : orbs) {
        orb->reset();
    }

    std::cout << "World reset! Press SPACE to jump, R to reset." << std::endl;
}
