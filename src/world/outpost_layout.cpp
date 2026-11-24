#include "outpost_layout.h"
#include <cmath>

namespace
{
struct Frame
{
    const Outpost &o;
    std::vector<OutpostBox> &out;
    float c, s;
    // local coords: +x forward (toward the gate), +z right
    void box(float lx, float baseY, float lz, float sx, float sy, float sz, float localYaw, float r, float g, float b)
    {
        float wx = o.x + lx * c - lz * s, wz = o.z + lx * s + lz * c;
        out.push_back({wx, o.padHeight + baseY + sy * 0.5f, wz, sx * 0.5f, sy * 0.5f, sz * 0.5f, o.yaw + localYaw, r, g, b});
    }
};
} // namespace

void buildOutpostBoxes(const Outpost &o, std::vector<OutpostBox> &out)
{
    Frame f{o, out, std::cos(o.yaw), std::sin(o.yaw)};
    const float R = o.padRadius - 4.0f;
    const int N = 18;
    // perimeter wall of concrete barrier blocks with a gate facing +x
    for (int i = 0; i < N; ++i)
    {
        float a = (i + 0.5f) * 6.2831853f / N;
        if (std::fabs(std::remainder(a, 6.2831853f)) < 0.42f) continue; // gate gap
        float seg = 2.0f * R * std::sin(3.1415926f / N) * 1.04f;
        f.box(R * std::cos(a), 0, R * std::sin(a), 0.9f, 1.5f, seg, a, 0.50f, 0.50f, 0.47f);
        f.box(R * std::cos(a), 1.5f, R * std::sin(a), 1.0f, 0.35f, seg * 0.92f, a, 0.30f, 0.27f, 0.20f); // sandbag cap
    }
    // watch towers on the diagonals
    for (int k = 0; k < 4; ++k)
    {
        float a = 0.785398f + k * 1.570796f;
        float tx = (R + 0.5f) * std::cos(a), tz = (R + 0.5f) * std::sin(a);
        for (int lx = -1; lx <= 1; lx += 2)
            for (int lz = -1; lz <= 1; lz += 2)
                f.box(tx + lx * 1.1f, 0, tz + lz * 1.1f, 0.35f, 6.5f, 0.35f, 0, 0.22f, 0.24f, 0.19f);
        f.box(tx, 6.2f, tz, 3.4f, 0.25f, 3.4f, a, 0.28f, 0.27f, 0.22f);
        f.box(tx, 6.45f, tz, 3.0f, 1.6f, 3.0f, a, 0.25f, 0.30f, 0.22f);
        f.box(tx, 8.05f, tz, 3.8f, 0.2f, 3.8f, a, 0.18f, 0.20f, 0.16f);
    }
    // command bunker with roof slab and door recess
    f.box(-5.0f, 0, 0, 9.0f, 3.2f, 6.5f, 0, 0.45f, 0.45f, 0.42f);
    f.box(-5.0f, 3.2f, 0, 9.6f, 0.4f, 7.1f, 0, 0.36f, 0.36f, 0.33f);
    f.box(-0.4f, 0, 0, 0.3f, 2.2f, 1.6f, 0, 0.08f, 0.08f, 0.07f);
    // barracks huts
    f.box(-3.0f, 0, -11.0f, 11.0f, 3.0f, 4.5f, 0, 0.27f, 0.32f, 0.22f);
    f.box(-3.0f, 3.0f, -11.0f, 11.6f, 0.3f, 5.0f, 0, 0.20f, 0.22f, 0.17f);
    f.box(-3.0f, 0, 11.0f, 11.0f, 3.0f, 4.5f, 0, 0.27f, 0.32f, 0.22f);
    f.box(-3.0f, 3.0f, 11.0f, 11.6f, 0.3f, 5.0f, 0, 0.20f, 0.22f, 0.17f);
    // sandbag cover behind the gate, supply crates, fuel drums
    f.box(9.0f, 0, -3.5f, 1.0f, 1.1f, 4.5f, 0.2f, 0.45f, 0.38f, 0.26f);
    f.box(9.0f, 0, 3.5f, 1.0f, 1.1f, 4.5f, -0.2f, 0.45f, 0.38f, 0.26f);
    f.box(4.0f, 0, 6.0f, 1.4f, 1.0f, 1.4f, 0.3f, 0.35f, 0.28f, 0.16f);
    f.box(5.6f, 0, 6.4f, 1.2f, 0.8f, 1.2f, -0.2f, 0.33f, 0.26f, 0.15f);
    f.box(4.5f, 0.8f, 6.1f, 1.0f, 0.7f, 1.0f, 0.1f, 0.36f, 0.30f, 0.18f);
    f.box(3.0f, 0, -6.5f, 0.7f, 1.0f, 0.7f, 0, 0.25f, 0.12f, 0.08f);
    f.box(3.9f, 0, -6.3f, 0.7f, 1.0f, 0.7f, 0, 0.12f, 0.20f, 0.12f);
    // flag pole
    f.box(2.0f, 0, 0, 0.18f, 9.0f, 0.18f, 0, 0.55f, 0.55f, 0.55f);
    f.box(2.0f, 7.6f, 0.9f, 0.05f, 1.1f, 1.7f, 0, 0.55f, 0.08f, 0.06f);
}
