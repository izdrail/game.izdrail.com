#include "json_mini.h"
#include <cctype>
#include <cstdlib>

namespace
{
struct P
{
    const std::string &s;
    size_t i = 0;
    [[noreturn]] void fail(const char *m) { throw std::runtime_error(std::string("json: ") + m + " at " + std::to_string(i)); }
    void ws() { while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i; }
    bool eat(char c) { ws(); if (i < s.size() && s[i] == c) { ++i; return true; } return false; }
    std::string str()
    {
        if (!eat('"')) fail("expected string");
        std::string r;
        while (i < s.size() && s[i] != '"')
        {
            char c = s[i++];
            if (c == '\\' && i < s.size())
            {
                char e = s[i++];
                switch (e) { case 'n': r += '\n'; break; case 't': r += '\t'; break; case 'u': i += 4; r += '?'; break; default: r += e; }
            }
            else r += c;
        }
        if (i >= s.size()) fail("unterminated string");
        ++i;
        return r;
    }
    Json val()
    {
        ws();
        if (i >= s.size()) fail("unexpected end");
        Json j;
        char c = s[i];
        if (c == '{')
        {
            ++i; j.type = Json::Object;
            if (eat('}')) return j;
            do { ws(); std::string k = str(); if (!eat(':')) fail("expected ':'"); j.obj[k] = val(); } while (eat(','));
            if (!eat('}')) fail("expected '}'");
        }
        else if (c == '[')
        {
            ++i; j.type = Json::Array;
            if (eat(']')) return j;
            do j.arr.push_back(val()); while (eat(','));
            if (!eat(']')) fail("expected ']'");
        }
        else if (c == '"') { j.type = Json::String; j.str = str(); }
        else if (s.compare(i, 4, "true") == 0) { i += 4; j.type = Json::Bool; j.b = true; }
        else if (s.compare(i, 5, "false") == 0) { i += 5; j.type = Json::Bool; }
        else if (s.compare(i, 4, "null") == 0) { i += 4; }
        else
        {
            char *end = nullptr;
            j.num = std::strtod(s.c_str() + i, &end);
            if (end == s.c_str() + i) fail("bad value");
            i = static_cast<size_t>(end - s.c_str());
            j.type = Json::Number;
        }
        return j;
    }
};
} // namespace

Json Json::parse(const std::string &text)
{
    P p{text};
    Json j = p.val();
    p.ws();
    if (p.i != text.size()) p.fail("trailing data");
    return j;
}
