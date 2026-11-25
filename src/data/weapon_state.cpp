#include "weapon_state.h"
#include <algorithm>

bool WeaponState::startReload()
{
    if (reloading() || mag_ >= def_->magazine || reserve_ <= 0) return false;
    reload_ = def_->reloadSec;
    burstLeft_ = 0;
    return true;
}

int WeaponState::update(float dt, bool trigger)
{
    const bool pressed = trigger && !prevTrigger_;
    prevTrigger_ = trigger;
    if (cooldown_ > 0) cooldown_ = std::max(0.0f, cooldown_ - dt);
    if (reload_ > 0)
    {
        reload_ -= dt;
        if (reload_ <= 0)
        {
            reload_ = 0;
            int take = std::min(def_->magazine - mag_, reserve_);
            mag_ += take;
            reserve_ -= take;
        }
        return 0;
    }
    bool want = false;
    switch (def_->mode)
    {
    case FireMode::Auto: want = trigger; break;
    case FireMode::Burst:
        if (pressed && burstLeft_ == 0) burstLeft_ = def_->burstCount;
        want = burstLeft_ > 0;
        break;
    default: want = pressed; break; // semi, pump, bolt: one shot per press
    }
    if (!want) return 0;
    if (mag_ <= 0)
    {
        burstLeft_ = 0;
        if (pressed || def_->mode == FireMode::Auto) startReload(); // click on empty starts the reload
        return 0;
    }
    if (cooldown_ > 0) return 0;
    --mag_;
    if (burstLeft_ > 0) --burstLeft_;
    cooldown_ = def_->secondsBetweenShots();
    return 1;
}
