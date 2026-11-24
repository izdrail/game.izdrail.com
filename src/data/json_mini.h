#pragma once
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// Minimal JSON reader for game data files (objects, arrays, strings, numbers, bool, null).
struct Json
{
    enum Type { Null, Bool, Number, String, Array, Object } type = Null;
    bool b = false;
    double num = 0;
    std::string str;
    std::vector<Json> arr;
    std::map<std::string, Json> obj;

    bool has(const std::string &k) const { return type == Object && obj.count(k); }
    const Json &at(const std::string &k) const
    {
        static const Json none;
        auto it = obj.find(k);
        return it == obj.end() ? none : it->second;
    }
    double number(const std::string &k, double def) const { return has(k) && at(k).type == Number ? at(k).num : def; }
    std::string string(const std::string &k, const std::string &def = "") const { return has(k) && at(k).type == String ? at(k).str : def; }
    bool boolean(const std::string &k, bool def) const { return has(k) && at(k).type == Bool ? at(k).b : def; }

    // Throws std::runtime_error on malformed input.
    static Json parse(const std::string &text);
};
