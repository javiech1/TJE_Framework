#ifndef LEVEL_CONFIG_H
#define LEVEL_CONFIG_H

#include <string>
#include <vector>
#include "framework/utils.h"

// Level configuration structure
// Defines all properties that can vary between levels
struct LevelConfig {
    // Basic level properties
    std::string name;                    // Display name for the level
    float gravity = 9.8f;                // Gravity strength (default Earth-like)

    // Skybox configuration (optional)
    // If empty, uses default skybox
    std::vector<std::string> skybox_faces;  // 6 faces: +X, -X, +Y, -Y, +Z, -Z

    // Level initialization type
    enum LevelType {
        TUTORIAL,    // Use existing initTutorial() with platforms
        EMPTY        // Empty level for future design
    };
    LevelType type = EMPTY;

    // Player configuration
    Vector3 player_start_position = Vector3(0.0f, 2.0f, 0.0f);

    // Audio settings
    std::string background_music;
    float music_volume = 0.5f;

    // Future expansion: Platform and entity definitions
    // Currently unused for empty levels
    struct PlatformDef {
        Vector3 position;
        Vector3 scale;
        Vector4 color;
        std::string texture_path;
    };
    std::vector<PlatformDef> platforms;

    struct OrbDef {
        Vector3 position;
        Vector4 color;
    };
    std::vector<OrbDef> orbs;
};

#endif // LEVEL_CONFIG_H