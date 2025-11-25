#ifndef LEVEL_MANAGER_H
#define LEVEL_MANAGER_H

#include "level_config.h"
#include <vector>

class LevelManager {
public:
    static LevelManager* instance;

    LevelManager();
    ~LevelManager();

    void initializeLevels();

    LevelConfig* getLevel(int index);
    LevelConfig* getLevelByName(const std::string& name);
    int getLevelCount() const { return levels.size(); }

    int getCurrentLevelIndex() const { return current_level_index; }
    void setCurrentLevelIndex(int index) { current_level_index = index; }

private:
    std::vector<LevelConfig> levels;
    int current_level_index = 0;

    LevelConfig createTutorialLevel();
    LevelConfig createLevel2();
    LevelConfig createLevel3();
};

#endif // LEVEL_MANAGER_H