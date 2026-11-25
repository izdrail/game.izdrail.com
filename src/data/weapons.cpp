#include "weapons.h"
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#include "json_mini.h"

const char *toString(WeaponClass c)
{
    static const char *n[] = {"pistol", "smg", "rifle", "shotgun", "sniper", "lmg", "launcher", "melee"};
    return n[static_cast<int>(c)];
}
const char *toString(FireMode m)
{
    static const char *n[] = {"semi", "auto", "burst", "pump", "bolt"};
    return n[static_cast<int>(m)];
}

namespace
{
WeaponClass parseClass(const std::string &s, const std::string &id)
{
    for (int i = 0; i < 8; ++i) if (s == toString(static_cast<WeaponClass>(i))) return static_cast<WeaponClass>(i);
    throw std::runtime_error("weapon " + id + ": unknown class '" + s + "'");
}
FireMode parseMode(const std::string &s, const std::string &id)
{
    for (int i = 0; i < 5; ++i) if (s == toString(static_cast<FireMode>(i))) return static_cast<FireMode>(i);
    throw std::runtime_error("weapon " + id + ": unknown fire mode '" + s + "'");
}
} // namespace

WeaponDatabase WeaponDatabase::fromJson(const std::string &text)
{
    Json root = Json::parse(text);
    const Json &list = root.at("weapons");
    if (list.type != Json::Array) throw std::runtime_error("weapons.json: missing 'weapons' array");
    WeaponDatabase db;
    std::set<std::string> seen;
    for (const Json &w : list.arr)
    {
        WeaponDef d;
        d.id = w.string("id");
        if (d.id.empty()) throw std::runtime_error("weapon without id");
        if (!seen.insert(d.id).second) throw std::runtime_error("duplicate weapon id " + d.id);
        d.name = w.string("name", d.id);
        d.cls = parseClass(w.string("class", "rifle"), d.id);
        d.mode = parseMode(w.string("mode", "semi"), d.id);
        d.damage = static_cast<float>(w.number("damage", d.damage));
        d.pellets = static_cast<int>(w.number("pellets", 1));
        d.roundsPerMinute = static_cast<float>(w.number("rpm", d.roundsPerMinute));
        d.burstCount = static_cast<int>(w.number("burst", 1));
        d.magazine = static_cast<int>(w.number("magazine", d.magazine));
        d.reserve = static_cast<int>(w.number("reserve", d.reserve));
        d.reloadSec = static_cast<float>(w.number("reload", d.reloadSec));
        d.spreadDeg = static_cast<float>(w.number("spread", d.spreadDeg));
        d.adsSpreadDeg = static_cast<float>(w.number("adsSpread", d.adsSpreadDeg));
        d.recoilKick = static_cast<float>(w.number("recoil", d.recoilKick));
        d.range = static_cast<float>(w.number("range", d.range));
        d.minFalloff = static_cast<float>(w.number("minFalloff", d.minFalloff));
        d.muzzleVelocity = static_cast<float>(w.number("velocity", d.muzzleVelocity));
        d.adsZoom = static_cast<float>(w.number("zoom", d.adsZoom));
        d.moveSpeedMul = static_cast<float>(w.number("moveSpeed", d.moveSpeedMul));
        d.weightKg = static_cast<float>(w.number("weight", d.weightKg));
        d.model = w.string("model");
        d.sound = w.string("sound");
        d.modelScale = static_cast<float>(w.number("scale", 1.0));
        d.modelLength = static_cast<float>(w.number("modelLength", d.modelLength));
        if (d.damage <= 0 || d.pellets < 1 || d.roundsPerMinute <= 0 || d.magazine < 1 || d.reloadSec <= 0 || d.range <= 0 ||
            d.minFalloff < 0 || d.minFalloff > 1 || d.adsZoom < 1.0f || d.moveSpeedMul <= 0 || d.reserve < 0)
            throw std::runtime_error("weapon " + d.id + ": value out of range");
        db.weapons_.push_back(d);
    }
    return db;
}

WeaponDatabase WeaponDatabase::fromFile(const std::string &path)
{
    std::ifstream f(path);
    if (!f) throw std::runtime_error("cannot open " + path);
    std::stringstream ss;
    ss << f.rdbuf();
    return fromJson(ss.str());
}

const WeaponDef *WeaponDatabase::find(const std::string &id) const
{
    for (const auto &w : weapons_) if (w.id == id) return &w;
    return nullptr;
}
