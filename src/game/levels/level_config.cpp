#include "level_config.h"
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

// Helper function to trim whitespace from strings
static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

// Safe float parsing with error handling
static bool parseFloat(const std::string& str, float& result, int line_number, const std::string& context) {
    try {
        size_t idx;
        result = std::stof(str, &idx);

        // Check if entire string was consumed
        if (idx != str.length()) {
            std::cerr << "Error: Line " << line_number << " (" << context << "): Invalid float '"
                      << str << "' - contains extra characters" << std::endl;
            return false;
        }
        return true;
    }
    catch (const std::invalid_argument&) {
        std::cerr << "Error: Line " << line_number << " (" << context << "): Invalid float '"
                  << str << "' - not a number" << std::endl;
        return false;
    }
    catch (const std::out_of_range&) {
        std::cerr << "Error: Line " << line_number << " (" << context << "): Float '"
                  << str << "' - out of range" << std::endl;
        return false;
    }
}

// Helper function to parse a vector from string "x y z" with validation
static bool parseVector3(const std::string& str, Vector3& result, int line_number, const std::string& context) {
    std::stringstream ss(str);
    float x, y, z;

    // Extract values
    ss >> x >> y >> z;

    // Check if extraction succeeded
    if (ss.fail()) {
        std::cerr << "Error: Line " << line_number << " (" << context << "): Invalid Vector3 '"
                  << str << "' - expected 3 numbers" << std::endl;
        return false;
    }

    // Check if there's extra data
    std::string extra;
    ss >> extra;
    if (!extra.empty()) {
        std::cerr << "Warning: Line " << line_number << " (" << context << "): Extra data after Vector3: '"
                  << extra << "'" << std::endl;
    }

    result = Vector3(x, y, z);
    return true;
}

// Helper function to parse a vector4 from string "r g b a" with validation
static bool parseVector4(const std::string& str, Vector4& result, int line_number, const std::string& context) {
    std::stringstream ss(str);
    float r, g, b, a;

    // Extract values
    ss >> r >> g >> b >> a;

    // Check if extraction succeeded
    if (ss.fail()) {
        std::cerr << "Error: Line " << line_number << " (" << context << "): Invalid Vector4 '"
                  << str << "' - expected 4 numbers" << std::endl;
        return false;
    }

    // Check if there's extra data
    std::string extra;
    ss >> extra;
    if (!extra.empty()) {
        std::cerr << "Warning: Line " << line_number << " (" << context << "): Extra data after Vector4: '"
                  << extra << "'" << std::endl;
    }

    // Sanity check color values
    if (r < 0.0f || r > 1.0f || g < 0.0f || g > 1.0f || b < 0.0f || b > 1.0f || a < 0.0f || a > 1.0f) {
        std::cerr << "Warning: Line " << line_number << " (" << context << "): Color values should be in [0,1] range. Got: "
                  << r << " " << g << " " << b << " " << a << std::endl;
    }

    result = Vector4(r, g, b, a);
    return true;
}

// Count pipe delimiters in string
static int countPipes(const std::string& str) {
    int count = 0;
    for (char c : str) {
        if (c == '|') count++;
    }
    return count;
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
    int error_count = 0;
    int warning_count = 0;

    std::cout << "Loading level from: " << filepath << std::endl;

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
            warning_count++;
            continue;
        }

        // Extract key and value
        std::string key = trim(line.substr(0, colon_pos));
        std::string value = trim(line.substr(colon_pos + 1));

        if (value.empty()) {
            std::cerr << "Warning: Line " << line_number << " (" << key << "): Empty value" << std::endl;
            warning_count++;
            continue;
        }

        // Parse based on key
        if (key == "name") {
            config.name = value;
        }
        else if (key == "gravity") {
            float g;
            if (parseFloat(value, g, line_number, "gravity")) {
                config.gravity = g;
                if (g <= 0.0f) {
                    std::cerr << "Warning: Line " << line_number << ": Gravity should be positive. Got: " << g << std::endl;
                    warning_count++;
                }
            } else {
                error_count++;
            }
        }
        else if (key == "player_start") {
            Vector3 pos;
            if (parseVector3(value, pos, line_number, "player_start")) {
                config.player_start_position = pos;
            } else {
                error_count++;
            }
        }
        else if (key == "music") {
            config.background_music = value;
        }
        else if (key == "music_volume") {
            float vol;
            if (parseFloat(value, vol, line_number, "music_volume")) {
                config.music_volume = vol;
                if (vol < 0.0f || vol > 1.0f) {
                    std::cerr << "Warning: Line " << line_number << ": Music volume should be in [0,1]. Got: " << vol << std::endl;
                    warning_count++;
                }
            } else {
                error_count++;
            }
        }
        else if (key == "platform") {
            // Validate pipe delimiter count
            int pipe_count = countPipes(value);
            if (pipe_count != 2) {
                std::cerr << "Error: Line " << line_number << " (platform): Expected 2 pipes '|', found "
                          << pipe_count << ". Format: pos_x pos_y pos_z | scale_x scale_y scale_z | r g b a" << std::endl;
                error_count++;
                continue;
            }

            // Parse platform: position | scale | color
            PlatformDef plat;

            // Split by | delimiter
            std::stringstream ss(value);
            std::string position_str, scale_str, color_str;

            std::getline(ss, position_str, '|');
            std::getline(ss, scale_str, '|');
            std::getline(ss, color_str, '|');

            position_str = trim(position_str);
            scale_str = trim(scale_str);
            color_str = trim(color_str);

            // Validate each component
            bool valid = true;
            Vector3 pos, scale;
            Vector4 color;

            if (!parseVector3(position_str, pos, line_number, "platform position")) {
                valid = false;
                error_count++;
            }

            if (!parseVector3(scale_str, scale, line_number, "platform scale")) {
                valid = false;
                error_count++;
            } else {
                // Sanity check scales
                if (scale.x <= 0.0f || scale.y <= 0.0f || scale.z <= 0.0f) {
                    std::cerr << "Warning: Line " << line_number << " (platform): Scale values should be positive. Got: "
                              << scale.x << " " << scale.y << " " << scale.z << std::endl;
                    warning_count++;
                }
            }

            if (!parseVector4(color_str, color, line_number, "platform color")) {
                valid = false;
                error_count++;
            }

            if (valid) {
                plat.position = pos;
                plat.scale = scale;
                plat.color = color;
                plat.texture_path = "";  // No texture support in text format yet
                config.platforms.push_back(plat);
            }
        }
        else if (key == "orb") {
            // Parse orb: position
            OrbDef orb;
            Vector3 pos;
            if (parseVector3(value, pos, line_number, "orb position")) {
                orb.position = pos;
                // Use default color from struct
                config.orbs.push_back(orb);
            } else {
                error_count++;
            }
        }
        else if (key == "reset_slab") {
            // Validate pipe delimiter count
            int pipe_count = countPipes(value);
            if (pipe_count != 2) {
                std::cerr << "Error: Line " << line_number << " (reset_slab): Expected 2 pipes '|', found "
                          << pipe_count << ". Format: pos_x pos_y pos_z | scale_x scale_y scale_z | r g b a" << std::endl;
                error_count++;
                continue;
            }

            // Parse reset_slab: position | scale | color
            ResetSlabDef slab;

            // Split by | delimiter
            std::stringstream ss(value);
            std::string position_str, scale_str, color_str;

            std::getline(ss, position_str, '|');
            std::getline(ss, scale_str, '|');
            std::getline(ss, color_str, '|');

            position_str = trim(position_str);
            scale_str = trim(scale_str);
            color_str = trim(color_str);

            // Validate each component
            bool valid = true;
            Vector3 pos, scale;
            Vector4 color;

            if (!parseVector3(position_str, pos, line_number, "reset_slab position")) {
                valid = false;
                error_count++;
            }

            if (!parseVector3(scale_str, scale, line_number, "reset_slab scale")) {
                valid = false;
                error_count++;
            } else {
                // Sanity check scales
                if (scale.x <= 0.0f || scale.y <= 0.0f || scale.z <= 0.0f) {
                    std::cerr << "Warning: Line " << line_number << " (reset_slab): Scale values should be positive. Got: "
                              << scale.x << " " << scale.y << " " << scale.z << std::endl;
                    warning_count++;
                }
            }

            if (!parseVector4(color_str, color, line_number, "reset_slab color")) {
                valid = false;
                error_count++;
            }

            if (valid) {
                slab.position = pos;
                slab.scale = scale;
                slab.color = color;
                config.reset_slabs.push_back(slab);
            }
        }
        else {
            std::cerr << "Warning: Unknown key '" << key << "' on line " << line_number << std::endl;
            warning_count++;
        }
    }

    // Set type to DATA since we loaded from file
    config.type = LevelType::DATA;

    file.close();

    // Print summary
    std::cout << "========================================" << std::endl;
    std::cout << "Loaded level '" << config.name << "' from " << filepath << std::endl;
    std::cout << "  Platforms: " << config.platforms.size() << std::endl;
    std::cout << "  Orbs: " << config.orbs.size() << std::endl;
    std::cout << "  Reset Slabs: " << config.reset_slabs.size() << std::endl;

    if (warning_count > 0) {
        std::cout << "  Warnings: " << warning_count << std::endl;
    }

    if (error_count > 0) {
        std::cout << "  ERRORS: " << error_count << std::endl;
        std::cerr << "Level loaded with errors - some entities may not have been created!" << std::endl;
    }

    std::cout << "========================================" << std::endl;

    return config;
}
