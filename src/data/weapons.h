#pragma once
#include <string>
#include <vector>

enum class WeaponClass { Pistol, SMG, Rifle, Shotgun, Sniper, LMG, Launcher, Melee };
enum class FireMode { Semi, Auto, Burst, Pump, Bolt };

// One weapon, fully described by data (assets/data/weapons.json). Nothing here is hard-coded in gameplay.
struct WeaponDef
{
    std::string id, name;
    WeaponClass cls = WeaponClass::Rifle;
    FireMode mode = FireMode::Semi;
    float damage = 20;        // per projectile, at close range
    int pellets = 1;          // projectiles per shot (shotgun > 1)
    float roundsPerMinute = 600;
    int burstCount = 1;
    int magazine = 30;
    int reserve = 120;
    float reloadSec = 2.0f;
    float spreadDeg = 1.0f;       // hip-fire cone
    float adsSpreadDeg = 0.2f;    // aimed cone
    float recoilKick = 1.0f;      // degrees of muzzle climb per shot
    float range = 200;            // metres where damage falls to minFalloff
    float minFalloff = 0.4f;      // damage multiplier at max range
    float muzzleVelocity = 800;   // m/s
    float adsZoom = 1.3f;         // FOV divisor when aiming
    float moveSpeedMul = 1.0f;    // carried-weapon speed multiplier
    float weightKg = 3.0f;
    std::string model;            // glTF/OBJ path relative to assets/resources/weapons ("" = placeholder)
    std::string sound;            // fire sound (relative to assets/resources/audio)
    float modelScale = 1.0f;
    float modelLength = 1.5f;     // first-person length in gun-matrix units

    float secondsBetweenShots() const { return 60.0f / roundsPerMinute; }
    // Damage multiplier at distance d (linear falloff to minFalloff at range).
    float falloff(float d) const
    {
        if (d <= 0) return 1.0f;
        if (d >= range) return minFalloff;
        return 1.0f - (1.0f - minFalloff) * (d / range);
    }
};

class WeaponDatabase
{
public:
    // Parses weapons.json text. Throws std::runtime_error on malformed JSON or invalid values.
    static WeaponDatabase fromJson(const std::string &text);
    static WeaponDatabase fromFile(const std::string &path);
    const std::vector<WeaponDef> &all() const { return weapons_; }
    const WeaponDef *find(const std::string &id) const;
    size_t size() const { return weapons_.size(); }

private:
    std::vector<WeaponDef> weapons_;
};

const char *toString(WeaponClass c);
const char *toString(FireMode m);
