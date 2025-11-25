#ifndef LEVEL_CONFIG_H
#define LEVEL_CONFIG_H

#include <string>
#include <vector>
#include "framework/utils.h"

// Level configuration structure
// Defines all properties that can vary between levels
struct LevelConfig {
    // Basic level properties
    std::string name;
    float gravity = 9.8f;

    // Skybox configuration (optional)
    std::vector<std::string> skybox_faces;

    // Level initialization type
    enum LevelType {
        EMPTY,       // Empty level for future design
        DATA         // Level with platforms, orbs, etc.
    };
    LevelType type = EMPTY;

    // Player configuration
    Vector3 player_start_position = Vector3(0.0f, 2.0f, 0.0f);

    // Audio settings
    std::string background_music;
    float music_volume = 0.5f;

    // Entity definitions (used when type == DATA)
    struct PlatformDef {
        Vector3 position;
        Vector3 scale;
        Vector4 color;
        std::string texture_path = "";

        // Movement parameters (optional)
        std::string movement_type = "none";  // "none", "linear", "circular"
        Vector3 movement_start;
        Vector3 movement_end;
        float movement_speed = 1.0f;
        float movement_phase = 0.0f;
        float orbit_radius = 0.0f;
        Vector3 orbit_center;
    };
    std::vector<PlatformDef> platforms;

    struct OrbDef {
        Vector3 position;
        Vector4 color = Vector4(1.0f, 0.9f, 0.3f, 1.0f);
    };
    std::vector<OrbDef> orbs;

    struct ResetSlabDef {
        Vector3 position;
        Vector3 scale;
        Vector4 color = Vector4(1.0f, 0.2f, 0.2f, 0.4f);
    };
    std::vector<ResetSlabDef> reset_slabs;

    struct ObstacleDef {
        Vector3 position;
        Vector3 scale;
        Vector4 color = Vector4(1.0f, 0.2f, 0.2f, 0.4f);

        // Movement parameters
        std::string movement_type = "linear";
        Vector3 movement_start;
        Vector3 movement_end;
        float movement_speed = 1.0f;
        float movement_phase = 0.0f;
        float orbit_radius = 0.0f;
        Vector3 orbit_center;
    };
    std::vector<ObstacleDef> obstacles;

    struct TwinPlatformDef {
        Vector3 position;
        Vector3 scale;
        Vector4 color;
        int group_id;
        bool starts_active;
    };
    std::vector<TwinPlatformDef> twin_platforms;
};

#endif // LEVEL_CONFIG_H
