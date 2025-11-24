#include "level_config.h"
#include <iostream>
#include <sstream>
#include <string>

// Helper function to trim whitespace from strings
static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Helper function to parse a vector from string "x y z"
static Vector3 parseVector3(const std::string& str) {
    std::stringstream ss(str);
    float x, y, z;
    ss >> x >> y >> z;
    return Vector3(x, y, z);
}

// Helper function to parse a vector4 from string "r g b a"
static Vector4 parseVector4(const std::string& str) {
    std::stringstream ss(str);
    float r, g, b, a;
    ss >> r >> g >> b >> a;
    return Vector4(r, g, b, a);
}

LevelConfig LevelConfig::loadFromJSON(const std::string& filepath) {
    LevelConfig config;

    // Open file
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open level file: " << filepath << std::endl;
        return config;  // Return empty config
    }

    std::string line;
    int line_number = 0;

    // Parse file line by line
    while (std::getline(file, line)) {
        line_number++;
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Find the colon separator
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            std::cerr << "Warning: Line " << line_number << " has no colon separator, skipping" << std::endl;
            continue;
        }

        // Extract key and value
        std::string key = trim(line.substr(0, colon_pos));
        std::string value = trim(line.substr(colon_pos + 1));

        // Parse based on key
        if (key == "name") {
            config.name = value;
        }
        else if (key == "gravity") {
            config.gravity = std::stof(value);
        }
        else if (key == "player_start") {
            config.player_start_position = parseVector3(value);
        }
        else if (key == "music") {
            config.background_music = value;
        }
        else if (key == "music_volume") {
            config.music_volume = std::stof(value);
        }
        else if (key == "platform") {
            // Parse platform: position | scale | color
            PlatformDef plat;

            // Split by | delimiter
            std::stringstream ss(value);
            std::string position_str, scale_str, color_str;

            std::getline(ss, position_str, '|');
            std::getline(ss, scale_str, '|');
            std::getline(ss, color_str, '|');

            plat.position = parseVector3(trim(position_str));
            plat.scale = parseVector3(trim(scale_str));
            plat.color = parseVector4(trim(color_str));
            plat.texture_path = "";  // No texture support in text format yet

            config.platforms.push_back(plat);
        }
        else if (key == "orb") {
            // Parse orb: position
            OrbDef orb;
            orb.position = parseVector3(value);
            // Use default color from struct
            config.orbs.push_back(orb);
        }
        else if (key == "reset_slab") {
            // Parse reset_slab: position | scale | color
            ResetSlabDef slab;

            // Split by | delimiter
            std::stringstream ss(value);
            std::string position_str, scale_str, color_str;

            std::getline(ss, position_str, '|');
            std::getline(ss, scale_str, '|');
            std::getline(ss, color_str, '|');

            slab.position = parseVector3(trim(position_str));
            slab.scale = parseVector3(trim(scale_str));
            slab.color = parseVector4(trim(color_str));

            config.reset_slabs.push_back(slab);
        }
        else {
            std::cerr << "Warning: Unknown key '" << key << "' on line " << line_number << std::endl;
        }
    }

    // Set type to DATA since we loaded from file
    config.type = LevelType::DATA;

    file.close();

    std::cout << "Loaded level '" << config.name << "' from " << filepath << std::endl;
    std::cout << "  Platforms: " << config.platforms.size() << std::endl;
    std::cout << "  Orbs: " << config.orbs.size() << std::endl;
    std::cout << "  Reset Slabs: " << config.reset_slabs.size() << std::endl;

    return config;
}
