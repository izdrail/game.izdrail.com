#include <cassert>
#include <cstdio>
#include "../src/data/weapon_state.h"

#ifndef WEAPONS_JSON
#define WEAPONS_JSON "assets/data/weapons.json"
#endif

// Run the weapon for `secs` at 120 steps/second, trigger held or pressed per `hold`.
static int simulate(WeaponState &w, float secs, bool hold)
{
    int shots = 0;
    for (int i = 0; i < static_cast<int>(secs * 120); ++i) shots += w.update(1.0f / 120.0f, hold);
    return shots;
}

int main()
{
    WeaponDatabase db = WeaponDatabase::fromFile(WEAPONS_JSON);
    // full auto: rate follows rpm (AK 600 rpm = 10 shots/s) and stops at empty
    WeaponState ak(*db.find("ak47"));
    int s = simulate(ak, 1.0f, true);
    assert(s >= 9 && s <= 11);
    // semi auto: holding the trigger fires only once
    WeaponState m9(*db.find("m9"));
    assert(simulate(m9, 2.0f, true) == 1 && m9.magazine() == m9.def().magazine - 1);
    // burst: one press = burstCount shots
    WeaponState m16(*db.find("m16"));
    int b = 0;
    b += m16.update(1.0f / 120, true);
    b += simulate(m16, 1.0f, false);
    assert(b == m16.def().burstCount);
    // magazine empties, then reload refills from reserve
    WeaponState uzi(*db.find("uzi"));
    simulate(uzi, 5.0f, true);
    assert(uzi.magazine() == 0 || uzi.reloading());
    int total = uzi.def().magazine + uzi.def().reserve;
    simulate(uzi, uzi.def().reloadSec + 0.5f, false);
    assert(!uzi.reloading() && uzi.magazine() > 0);
    // ammo is conserved or spent, never created
    assert(uzi.magazine() + uzi.reserve() <= total);
    // reload does nothing when full or when reserve is empty
    WeaponState dg(*db.find("deagle"));
    assert(!dg.startReload());
    simulate(dg, 0.5f, true);
    assert(dg.startReload() && dg.reloading());
    // no firing while reloading; cancel aborts the reload
    assert(dg.update(0.01f, true) == 0);
    dg.cancel();
    assert(!dg.reloading());
    // knife has no reserve and still fires
    WeaponState k(*db.find("knife"));
    assert(simulate(k, 0.1f, true) == 1);
    std::printf("weapon state ok\n");
    return 0;
}
