#ifndef LEVEL_MANAGER_H
#define LEVEL_MANAGER_H

#include "level_config.h"
#include <vector>

// Manages all game levels and their configurations
class LevelManager {
public:
    // Singleton pattern for global access
    static LevelManager* instance;

    LevelManager();
    ~LevelManager();

    // Initialize all level configurations
    void initializeLevels();

    // Level access methods
    LevelConfig* getLevel(int index);
    LevelConfig* getLevelByName(const std::string& name);
    int getLevelCount() const { return levels.size(); }

    // Get current level index (managed by PlayStage)
    int getCurrentLevelIndex() const { return current_level_index; }
    void setCurrentLevelIndex(int index) { current_level_index = index; }

private:
    std::vector<LevelConfig> levels;
    int current_level_index = 0;

    // Helper methods to create each level
    LevelConfig createTutorialLevel();  // Level 1: Existing tutorial
    LevelConfig createLevel2();         // Level 2: Empty with low gravity
    LevelConfig createLevel3();         // Level 3: Empty with high gravity
};

#endif // LEVEL_MANAGER_H