#pragma once
#include "weapons.h"

// Runtime state of one weapon (ammo, cooldown, reload, burst). Pure CPU, no GL or audio.
class WeaponState
{
public:
    explicit WeaponState(const WeaponDef &def) : def_(&def), mag_(def.magazine), reserve_(def.reserve) {}

    const WeaponDef &def() const { return *def_; }
    int magazine() const { return mag_; }
    int reserve() const { return reserve_; }
    bool reloading() const { return reload_ > 0; }

    // Advance by dt seconds. `trigger` is whether the fire button is held.
    // Returns the number of shots fired during this step (0 or 1).
    int update(float dt, bool trigger);
    // Starts a reload when useful (magazine not full, reserve left). Returns true if started.
    bool startReload();
    // Cancels reload and burst (used when switching away).
    void cancel() { reload_ = 0; burstLeft_ = 0; }

private:
    const WeaponDef *def_;
    int mag_, reserve_;
    float cooldown_ = 0, reload_ = 0;
    int burstLeft_ = 0;
    bool prevTrigger_ = false;
};
