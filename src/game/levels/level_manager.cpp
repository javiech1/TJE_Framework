#include "level_manager.h"
#include <iostream>

// Initialize singleton instance
LevelManager* LevelManager::instance = nullptr;

LevelManager::LevelManager() {
    instance = this;
    initializeLevels();
}

LevelManager::~LevelManager() {
    if (instance == this) {
        instance = nullptr;
    }
}

void LevelManager::initializeLevels() {
    levels.clear();

    // Create and add all levels
    levels.push_back(createTutorialLevel());  // Level 1
    levels.push_back(createLevel2());         // Level 2
    levels.push_back(createLevel3());         // Level 3

    std::cout << "LevelManager: Initialized " << levels.size() << " levels" << std::endl;
}

LevelConfig* LevelManager::getLevel(int index) {
    if (index >= 0 && index < levels.size()) {
        return &levels[index];
    }
    return nullptr;
}

LevelConfig* LevelManager::getLevelByName(const std::string& name) {
    for (auto& level : levels) {
        if (level.name == name) {
            return &level;
        }
    }
    return nullptr;
}

// Level 1: Tutorial level with platforms and orbs
LevelConfig LevelManager::createTutorialLevel() {
    LevelConfig config;

    config.name = "Tutorial";
    config.type = LevelConfig::TUTORIAL;  // Uses existing initTutorial()
    config.gravity = 9.8f;  // Normal Earth gravity

    // Default skybox (will use the default rocks/grass textures)
    // Leave skybox_faces empty to use default

    config.player_start_position = Vector3(0.0f, 2.0f, 0.0f);
    config.background_music = "data/audio/stellar_drift.mp3";
    config.music_volume = 0.5f;

    // Platforms and orbs are created by initTutorial(), not here

    return config;
}

// Level 2: Empty level with low gravity (moon-like)
LevelConfig LevelManager::createLevel2() {
    LevelConfig config;

    config.name = "Low Gravity Zone";
    config.type = LevelConfig::TUTORIAL;  // Use tutorial platforms and orbs
    config.gravity = 4.9f;  // Half of Earth gravity (moon-like)

    // Could specify custom skybox here if desired
    // For now, use default skybox

    config.player_start_position = Vector3(0.0f, 2.0f, 0.0f);  // Start above platform
    config.background_music = "data/audio/stellar_drift.mp3";
    config.music_volume = 0.3f;

    // Platforms and orbs created by initTutorial()

    return config;
}

// Level 3: Empty level with high gravity
LevelConfig LevelManager::createLevel3() {
    LevelConfig config;

    config.name = "High Gravity Zone";
    config.type = LevelConfig::TUTORIAL;  // Use tutorial platforms and orbs
    config.gravity = 19.6f;  // Double Earth gravity

    // Could specify custom skybox here if desired
    // For now, use default skybox

    config.player_start_position = Vector3(0.0f, 2.0f, 0.0f);  // Start above platform
    config.background_music = "data/audio/stellar_drift.mp3";
    config.music_volume = 0.4f;

    // Platforms and orbs created by initTutorial()

    return config;
}