#include "level_manager.h"

LevelManager* LevelManager::instance = nullptr;

namespace {

LevelConfig::PlatformDef platform(
    float x, float y, float z,
    float sx, float sy, float sz,
    const Vector4& color)
{
    LevelConfig::PlatformDef p;
    p.position = Vector3(x, y, z);
    p.scale = Vector3(sx, sy, sz);
    p.color = color;
    p.movement_type = "none";
    return p;
}

LevelConfig::PlatformDef movingPlatformLinear(
    float x, float y, float z,
    float sx, float sy, float sz,
    float ex, float ey, float ez,
    float speed, float phase = 0.0f)
{
    LevelConfig::PlatformDef p;
    p.position = Vector3(x, y, z);
    p.scale = Vector3(sx, sy, sz);
    p.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    p.movement_type = "linear";
    p.movement_start = Vector3(x, y, z);
    p.movement_end = Vector3(ex, ey, ez);
    p.movement_speed = speed;
    p.movement_phase = phase;
    return p;
}

LevelConfig::PlatformDef movingPlatformCircular(
    float x, float y, float z,
    float sx, float sy, float sz,
    float radius, float speed, float phase = 0.0f)
{
    LevelConfig::PlatformDef p;
    p.position = Vector3(x, y, z);
    p.scale = Vector3(sx, sy, sz);
    p.color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
    p.movement_type = "circular";
    p.orbit_center = Vector3(x, y, z);
    p.orbit_radius = radius;
    p.movement_speed = speed;
    p.movement_phase = phase;
    return p;
}

LevelConfig::OrbDef orb(float x, float y, float z)
{
    LevelConfig::OrbDef o;
    o.position = Vector3(x, y, z);
    return o;
}

LevelConfig::TwinPlatformDef twinPlatform(
    float x, float y, float z,
    float sx, float sy, float sz,
    const Vector4& color,
    int groupId, bool startsActive)
{
    LevelConfig::TwinPlatformDef t;
    t.position = Vector3(x, y, z);
    t.scale = Vector3(sx, sy, sz);
    t.color = color;
    t.group_id = groupId;
    t.starts_active = startsActive;
    return t;
}

LevelConfig::ObstacleDef obstacleLinear(
    float x, float y, float z,
    float sx, float sy, float sz,
    float startX, float startY, float startZ,
    float endX, float endY, float endZ,
    float speed)
{
    LevelConfig::ObstacleDef o;
    o.position = Vector3(x, y, z);
    o.scale = Vector3(sx, sy, sz);
    o.movement_type = "linear";
    o.movement_start = Vector3(startX, startY, startZ);
    o.movement_end = Vector3(endX, endY, endZ);
    o.movement_speed = speed;
    return o;
}

LevelConfig::ObstacleDef obstacleCircular(
    float x, float y, float z,
    float sx, float sy, float sz,
    float cx, float cy, float cz,
    float radius, float speed)
{
    LevelConfig::ObstacleDef o;
    o.position = Vector3(x, y, z);
    o.scale = Vector3(sx, sy, sz);
    o.movement_type = "circular";
    o.orbit_center = Vector3(cx, cy, cz);
    o.orbit_radius = radius;
    o.movement_speed = speed;
    return o;
}

}

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
    levels.push_back(createTutorialLevel());
    levels.push_back(createLevel2());
    levels.push_back(createLevel3());
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

LevelConfig LevelManager::createTutorialLevel() {
    LevelConfig config;
    config.name = "Tutorial";
    config.type = LevelConfig::DATA;
    config.gravity = 9.8f;
    config.player_start_position = Vector3(0.0f, 1.5f, 0.0f);
    config.background_music = "data/audio/stellar_drift.mp3";
    config.music_volume = 0.5f;

    const Vector4 WHITE(1.0f, 1.0f, 1.0f, 1.0f);

    config.platforms.push_back(platform(0.0f, 0.0f, 0.0f, 0.20f, 0.01f, 0.20f, WHITE));
    config.platforms.push_back(platform(0.0f, 0.0f, -28.0f, 0.15f, 0.01f, 0.15f, WHITE));
    config.orbs.push_back(orb(0.0f, 2.0f, -28.0f));

    config.platforms.push_back(platform(0.0f, 2.0f, -46.0f, 0.10f, 0.01f, 0.10f, WHITE));
    config.platforms.push_back(platform(6.0f, 4.5f, -60.0f, 0.10f, 0.01f, 0.10f, WHITE));
    config.platforms.push_back(platform(-6.0f, 7.0f, -74.0f, 0.10f, 0.01f, 0.10f, WHITE));
    config.platforms.push_back(platform(0.0f, 10.0f, -88.0f, 0.10f, 0.01f, 0.10f, WHITE));
    config.orbs.push_back(orb(0.0f, 12.0f, -88.0f));

    config.platforms.push_back(platform(0.0f, 10.5f, -104.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.platforms.push_back(movingPlatformLinear(-4.0f, 11.0f, -120.0f, 0.08f, 0.01f, 0.08f, 4.0f, 11.0f, -120.0f, 0.4f, 0.0f));
    config.platforms.push_back(platform(0.0f, 11.5f, -134.0f, 0.06f, 0.01f, 0.06f, WHITE));
    config.platforms.push_back(movingPlatformLinear(0.0f, 12.0f, -148.0f, 0.08f, 0.01f, 0.08f, 0.0f, 15.0f, -148.0f, 0.35f, 0.0f));
    config.platforms.push_back(platform(0.0f, 16.0f, -162.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.platforms.push_back(movingPlatformCircular(0.0f, 16.5f, -180.0f, 0.08f, 0.01f, 0.08f, 3.0f, 0.5f, 0.0f));
    config.orbs.push_back(orb(0.0f, 18.5f, -180.0f));

    config.platforms.push_back(platform(0.0f, 17.0f, -198.0f, 0.10f, 0.01f, 0.10f, WHITE));
    config.obstacles.push_back(obstacleLinear(0.0f, 17.8f, -210.0f, 0.02f, 0.04f, 0.15f, -6.0f, 17.8f, -210.0f, 6.0f, 17.8f, -210.0f, 0.6f));
    config.platforms.push_back(platform(0.0f, 17.5f, -220.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.obstacles.push_back(obstacleLinear(0.0f, 18.5f, -232.0f, 0.02f, 0.06f, 0.15f, 6.0f, 18.5f, -232.0f, -6.0f, 18.5f, -232.0f, 0.7f));
    config.platforms.push_back(platform(0.0f, 18.0f, -242.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.obstacles.push_back(obstacleLinear(0.0f, 19.0f, -254.0f, 0.12f, 0.04f, 0.02f, 0.0f, 17.0f, -254.0f, 0.0f, 21.0f, -254.0f, 0.5f));
    config.platforms.push_back(platform(5.0f, 18.5f, -262.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.obstacles.push_back(obstacleCircular(0.0f, 19.5f, -274.0f, 0.02f, 0.05f, 0.02f, 0.0f, 19.5f, -274.0f, 2.5f, 0.7f));
    config.platforms.push_back(platform(0.0f, 20.0f, -280.0f, 0.10f, 0.01f, 0.10f, WHITE));
    config.orbs.push_back(orb(0.0f, 22.0f, -280.0f));

    config.platforms.push_back(platform(0.0f, 20.0f, -296.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.obstacles.push_back(obstacleLinear(0.0f, 18.0f, -308.0f, 0.18f, 0.03f, 0.02f, -5.0f, 18.0f, -308.0f, 5.0f, 18.0f, -308.0f, 0.8f));
    config.platforms.push_back(platform(0.0f, 16.0f, -314.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.platforms.push_back(movingPlatformLinear(3.0f, 14.5f, -330.0f, 0.08f, 0.01f, 0.08f, -3.0f, 14.5f, -330.0f, 0.5f, 0.0f));
    config.obstacles.push_back(obstacleLinear(0.0f, 15.0f, -330.0f, 0.02f, 0.06f, 0.08f, 0.0f, 13.0f, -330.0f, 0.0f, 17.0f, -330.0f, 0.6f));
    config.platforms.push_back(platform(0.0f, 14.0f, -346.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.orbs.push_back(orb(0.0f, 16.0f, -346.0f));

    config.platforms.push_back(platform(0.0f, 14.5f, -362.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.platforms.push_back(movingPlatformCircular(0.0f, 16.0f, -378.0f, 0.06f, 0.01f, 0.06f, 3.0f, 0.55f, 0.0f));
    config.obstacles.push_back(obstacleCircular(0.0f, 17.0f, -378.0f, 0.02f, 0.04f, 0.02f, 0.0f, 17.0f, -378.0f, 2.0f, 0.8f));
    config.platforms.push_back(movingPlatformLinear(0.0f, 18.0f, -394.0f, 0.06f, 0.01f, 0.06f, 0.0f, 21.0f, -394.0f, 0.45f, 0.0f));
    config.obstacles.push_back(obstacleLinear(0.0f, 19.5f, -394.0f, 0.15f, 0.02f, 0.02f, -4.0f, 19.5f, -394.0f, 4.0f, 19.5f, -394.0f, 0.75f));
    config.platforms.push_back(movingPlatformCircular(-2.5f, 22.0f, -410.0f, 0.06f, 0.01f, 0.06f, 2.5f, 0.6f, 0.0f));
    config.platforms.push_back(movingPlatformCircular(2.5f, 22.5f, -410.0f, 0.06f, 0.01f, 0.06f, 2.5f, 0.6f, 3.14f));
    config.obstacles.push_back(obstacleLinear(-5.0f, 22.5f, -410.0f, 0.01f, 0.08f, 0.12f, -5.0f, 20.0f, -410.0f, -5.0f, 25.0f, -410.0f, 0.55f));
    config.obstacles.push_back(obstacleLinear(5.0f, 22.5f, -410.0f, 0.01f, 0.08f, 0.12f, 5.0f, 20.0f, -410.0f, 5.0f, 25.0f, -410.0f, 0.55f));
    config.platforms.push_back(movingPlatformCircular(0.0f, 24.0f, -426.0f, 0.06f, 0.01f, 0.06f, 2.5f, 0.7f, 0.0f));
    config.obstacles.push_back(obstacleLinear(0.0f, 25.0f, -438.0f, 0.02f, 0.05f, 0.12f, -5.0f, 25.0f, -438.0f, 5.0f, 25.0f, -438.0f, 0.85f));
    config.obstacles.push_back(obstacleLinear(0.0f, 25.8f, -438.0f, 0.02f, 0.05f, 0.12f, 5.0f, 25.8f, -438.0f, -5.0f, 25.8f, -438.0f, 0.85f));
    config.platforms.push_back(platform(0.0f, 25.5f, -444.0f, 0.08f, 0.01f, 0.08f, WHITE));
    config.orbs.push_back(orb(0.0f, 27.5f, -444.0f));

    config.platforms.push_back(platform(0.0f, 26.0f, -466.0f, 0.25f, 0.01f, 0.25f, WHITE));

    return config;
}

LevelConfig LevelManager::createLevel2() {
    LevelConfig config;
    config.name = "Phase Shift";
    config.type = LevelConfig::DATA;
    config.gravity = 9.8f;
    config.player_start_position = Vector3(0.0f, 1.5f, 0.0f);
    config.background_music = "data/audio/stellar_drift.mp3";
    config.music_volume = 0.5f;

    // Common colors
    const Vector4 BLUE(0.5f, 0.7f, 1.0f, 1.0f);
    const Vector4 WALL_BLUE(0.3f, 0.5f, 0.9f, 1.0f);
    const Vector4 CYAN(0.2f, 0.9f, 0.9f, 1.0f);
    const Vector4 ORANGE(1.0f, 0.6f, 0.2f, 1.0f);
    const Vector4 GOLD(1.0f, 0.9f, 0.3f, 1.0f);

    // Section 1: Start Area
    config.platforms.push_back(platform(0.0f, 0.0f, 0.0f, 0.20f, 0.01f, 0.20f, BLUE));
    config.platforms.push_back(platform(0.0f, 2.0f, -30.0f, 0.10f, 0.01f, 0.10f, BLUE));
    config.platforms.push_back(platform(-6.0f, 8.0f, -30.0f, 0.02f, 0.10f, 0.06f, WALL_BLUE));
    config.platforms.push_back(platform(12.0f, 10.0f, -45.0f, 0.12f, 0.01f, 0.12f, BLUE));
    config.orbs.push_back(orb(12.0f, 12.0f, -45.0f));

    // Section 2: Twin Platform Introduction
    config.twin_platforms.push_back(twinPlatform(-6.0f, 12.0f, -65.0f, 0.10f, 0.01f, 0.10f, CYAN, 0, true));
    config.twin_platforms.push_back(twinPlatform(6.0f, 15.0f, -85.0f, 0.10f, 0.01f, 0.10f, ORANGE, 0, false));
    config.platforms.push_back(platform(0.0f, 17.0f, -108.0f, 0.12f, 0.01f, 0.12f, BLUE));
    config.orbs.push_back(orb(0.0f, 19.0f, -108.0f));

    // Section 3: Wall Jump with Twin
    config.platforms.push_back(platform(4.0f, 18.0f, -130.0f, 0.08f, 0.01f, 0.08f, BLUE));
    config.platforms.push_back(platform(12.0f, 22.0f, -135.0f, 0.02f, 0.10f, 0.06f, WALL_BLUE));
    config.twin_platforms.push_back(twinPlatform(-5.0f, 23.0f, -135.0f, 0.12f, 0.01f, 0.12f, ORANGE, 1, false));
    config.platforms.push_back(platform(-27.0f, 24.0f, -135.0f, 0.08f, 0.01f, 0.08f, BLUE));
    config.orbs.push_back(orb(-27.0f, 26.0f, -135.0f));

    // Section 4: Wall Climb Corridor
    config.platforms.push_back(platform(-35.0f, 26.0f, -140.0f, 0.06f, 0.25f, 0.02f, WALL_BLUE));
    config.platforms.push_back(platform(-45.0f, 28.0f, -130.0f, 0.06f, 0.25f, 0.02f, WALL_BLUE));
    config.platforms.push_back(platform(-55.0f, 30.0f, -140.0f, 0.06f, 0.25f, 0.02f, WALL_BLUE));
    config.platforms.push_back(platform(-65.0f, 32.0f, -130.0f, 0.06f, 0.25f, 0.02f, WALL_BLUE));
    config.platforms.push_back(platform(-80.0f, 34.0f, -135.0f, 0.10f, 0.01f, 0.10f, BLUE));
    config.orbs.push_back(orb(-80.0f, 36.0f, -135.0f));

    // Section 5: Twin Wall Climb (Diagonal)
    config.platforms.push_back(platform(-91.0f, 36.0f, -135.0f, 0.08f, 0.01f, 0.08f, BLUE));
    config.twin_platforms.push_back(twinPlatform(-102.0f, 40.0f, -140.0f, 0.06f, 0.12f, 0.02f, CYAN, 2, true));
    config.twin_platforms.push_back(twinPlatform(-102.0f, 43.0f, -130.0f, 0.06f, 0.12f, 0.02f, ORANGE, 2, false));
    config.twin_platforms.push_back(twinPlatform(-107.0f, 46.0f, -140.0f, 0.06f, 0.12f, 0.02f, CYAN, 3, true));
    config.twin_platforms.push_back(twinPlatform(-107.0f, 49.0f, -130.0f, 0.06f, 0.12f, 0.02f, ORANGE, 3, false));
    config.twin_platforms.push_back(twinPlatform(-112.0f, 51.0f, -140.0f, 0.06f, 0.12f, 0.02f, CYAN, 4, true));
    config.twin_platforms.push_back(twinPlatform(-112.0f, 54.0f, -130.0f, 0.06f, 0.12f, 0.02f, ORANGE, 4, false));
    config.platforms.push_back(platform(-120.0f, 57.0f, -135.0f, 0.10f, 0.01f, 0.10f, BLUE));
    config.orbs.push_back(orb(-120.0f, 59.0f, -135.0f));

    // Section 5.5: Vertical Twin Climb Tower
    config.twin_platforms.push_back(twinPlatform(-120.0f, 60.0f, -140.0f, 0.06f, 0.12f, 0.02f, CYAN, 5, true));
    config.twin_platforms.push_back(twinPlatform(-120.0f, 63.0f, -130.0f, 0.06f, 0.12f, 0.02f, ORANGE, 5, false));
    config.twin_platforms.push_back(twinPlatform(-120.0f, 66.0f, -140.0f, 0.06f, 0.10f, 0.02f, CYAN, 5, true));
    config.twin_platforms.push_back(twinPlatform(-120.0f, 69.0f, -130.0f, 0.06f, 0.10f, 0.02f, ORANGE, 5, false));
    config.twin_platforms.push_back(twinPlatform(-120.0f, 72.0f, -140.0f, 0.06f, 0.06f, 0.02f, CYAN, 5, true));
    config.twin_platforms.push_back(twinPlatform(-120.0f, 75.0f, -130.0f, 0.06f, 0.06f, 0.02f, ORANGE, 5, false));
    config.twin_platforms.push_back(twinPlatform(-120.0f, 78.0f, -140.0f, 0.06f, 0.04f, 0.02f, CYAN, 5, true));
    config.twin_platforms.push_back(twinPlatform(-120.0f, 81.0f, -130.0f, 0.06f, 0.04f, 0.02f, ORANGE, 5, false));
    config.platforms.push_back(platform(-120.0f, 85.0f, -140.0f, 0.10f, 0.01f, 0.10f, BLUE));
    config.orbs.push_back(orb(-120.0f, 87.0f, -140.0f));

    // Section 6: Moving Wall Finale
    config.platforms.push_back(platform(-141.0f, 87.0f, -135.0f, 0.08f, 0.01f, 0.08f, BLUE));
    config.platforms.push_back(movingPlatformLinear(-152.0f, 89.0f, -140.0f, 0.06f, 0.12f, 0.02f, -152.0f, 101.0f, -140.0f, 0.12f, 0.0f));
    config.platforms.push_back(movingPlatformLinear(-157.0f, 92.0f, -130.0f, 0.06f, 0.12f, 0.02f, -157.0f, 104.0f, -130.0f, 0.15f, 0.0f));
    config.platforms.push_back(movingPlatformLinear(-162.0f, 95.0f, -140.0f, 0.06f, 0.12f, 0.02f, -162.0f, 107.0f, -140.0f, 0.18f, 0.0f));
    config.platforms.push_back(movingPlatformLinear(-167.0f, 98.0f, -130.0f, 0.06f, 0.12f, 0.02f, -167.0f, 110.0f, -130.0f, 0.20f, 0.0f));

    // Goal Platform
    config.platforms.push_back(platform(-188.0f, 101.0f, -135.0f, 0.25f, 0.01f, 0.25f, GOLD));
    config.orbs.push_back(orb(-188.0f, 103.0f, -135.0f));

    return config;
}

LevelConfig LevelManager::createLevel3() {
    LevelConfig config;
    config.name = "High Gravity Zone";
    config.type = LevelConfig::EMPTY;
    config.gravity = 19.6f;
    config.player_start_position = Vector3(0.0f, 2.0f, 0.0f);
    config.background_music = "data/audio/stellar_drift.mp3";
    config.music_volume = 0.4f;
    return config;
}
