#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <string>
#include "../src/data/json_mini.h"
#include "../src/data/weapons.h"

#ifndef WEAPONS_JSON
#define WEAPONS_JSON "assets/data/weapons.json"
#endif

static bool throws(const std::string &t)
{
    try { WeaponDatabase::fromJson(t); } catch (const std::runtime_error &) { return true; }
    return false;
}

int main()
{
    Json j = Json::parse("{\"a\":[1,2.5,-3e1],\"b\":{\"c\":\"x\\\"y\",\"d\":true,\"e\":null}}");
    assert(j.at("a").arr.size() == 3 && j.at("a").arr[2].num == -30.0);
    assert(j.at("b").string("c") == "x\"y" && j.at("b").boolean("d", false));

    WeaponDatabase db = WeaponDatabase::fromFile(WEAPONS_JSON);
    assert(db.size() >= 20);
    int classes[8] = {0};
    for (const auto &w : db.all())
    {
        assert(w.damage > 0 && w.magazine >= 1 && w.secondsBetweenShots() > 0);
        assert(w.falloff(0) == 1.0f && w.falloff(w.range * 2) == w.minFalloff);
        assert(w.falloff(w.range * 0.5f) < 1.0f + 1e-6f && w.falloff(w.range * 0.5f) >= w.minFalloff);
        if (w.mode == FireMode::Burst) assert(w.burstCount > 1);
        if (w.cls == WeaponClass::Shotgun) assert(w.pellets > 1);
        ++classes[static_cast<int>(w.cls)];
    }
    for (int c = 0; c < 8; ++c) assert(classes[c] > 0);
    const WeaponDef *ak = db.find("ak47");
    assert(ak && ak->cls == WeaponClass::Rifle && ak->mode == FireMode::Auto);
    assert(db.find("nope") == nullptr);

    assert(throws("{\"weapons\":[{\"id\":\"a\"},{\"id\":\"a\"}]}"));          // duplicate id
    assert(throws("{\"weapons\":[{\"id\":\"a\",\"class\":\"laser\"}]}"));    // bad class
    assert(throws("{\"weapons\":[{\"id\":\"a\",\"damage\":-5}]}"));          // bad value
    assert(throws("{\"weapons\":[{\"id\":\"a\""));                           // malformed
    assert(throws("{}"));                                                    // missing list
    std::printf("weapons ok: %zu weapons\n", db.size());
    return 0;
}
