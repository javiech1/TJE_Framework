#ifndef LEVEL_CONFIG_H
#define LEVEL_CONFIG_H

#include <string>
#include <vector>
#include "framework/utils.h"
#include <fstream>
#include <sstream>

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
        TUTORIAL,    // Use existing initTutorial() with platforms (legacy)
        EMPTY,       // Empty level for future design
        DATA         // Load from JSON data (platforms, orbs, reset_slabs vectors)
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
        std::string texture_path = "";  // Optional, empty for flat color
    };
    std::vector<PlatformDef> platforms;

    struct OrbDef {
        Vector3 position;
        Vector4 color = Vector4(1.0f, 0.9f, 0.3f, 1.0f);  // Default gold color
    };
    std::vector<OrbDef> orbs;

    struct ResetSlabDef {
        Vector3 position;
        Vector3 scale;
        Vector4 color = Vector4(1.0f, 0.2f, 0.2f, 0.4f);  // Default red semi-transparent
    };
    std::vector<ResetSlabDef> reset_slabs;

    // Static method to load configuration from JSON file
    static LevelConfig loadFromJSON(const std::string& filepath);
};

#endif // LEVEL_CONFIG_H