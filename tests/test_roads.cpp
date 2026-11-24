#include <cassert>
#include <cmath>
#include <cstdio>
#include "../src/world/terrain.h"
#include "../src/world/roads.h"
#include "../src/world/outpost_layout.h"

static void cfgFor(TerrainConfig &c)
{
    c.worldSize = 1200; c.cellSize = 3; c.chunkCells = 40; c.heightScale = 95; c.frequency = 0.0028f;
    c.mountainBlend = 0.8f; c.warpStrength = 60; c.flatRadius = 25; c.flatBlend = 140; c.waterLevel = -1.8f - 9.0f; c.baseHeight = -1.8f;
}

int main()
{
    int totalRoads = 0, totalOutposts = 0;
    for (uint64_t seed : {1337ull, 2024ull, 7ull})
    {
        TerrainConfig c; cfgFor(c); c.seed = seed;
        Terrain t(c);
        RoadNetwork a, b;
        a.build(t, 4);
        b.build(t, 4);
        // determinism
        assert(a.outposts().size() == b.outposts().size() && a.roads().size() == b.roads().size());
        for (size_t i = 0; i < a.outposts().size(); ++i) assert(a.outposts()[i].x == b.outposts()[i].x && a.outposts()[i].z == b.outposts()[i].z);
        assert(a.outposts().size() >= 2);
        assert(!a.roads().empty());
        t.attachRoads(&a);
        for (const Outpost &o : a.outposts())
        {
            // outposts stay dry and pads are level
            assert(!t.isWater(o.x, o.z));
            for (int s = 0; s < 12; ++s)
            {
                float an = s * 0.5236f, x = o.x + 0.8f * o.padRadius * std::cos(an), z = o.z + 0.8f * o.padRadius * std::sin(an);
                assert(std::fabs(t.heightAt(x, z) - o.padHeight) < 0.01f);
            }
            std::vector<OutpostBox> boxes; buildOutpostBoxes(o, boxes);
            assert(boxes.size() > 40);
            for (auto &bx : boxes) assert(std::isfinite(bx.cx) && std::isfinite(bx.cy) && bx.hx > 0);
        }
        // roads are dry, graded and masked as road surface
        for (const auto &r : a.roads())
        {
            assert(r.size() > 5);
            for (size_t i = 0; i < r.size(); ++i)
            {
                assert(!t.isWater(r[i].x, r[i].z));
                assert(a.surfaceMask(r[i].x, r[i].z) > 0.99f);
                assert(a.near(r[i].x, r[i].z, 0.0f));
                if (i + 1 < r.size())
                {
                    float run = std::hypot(r[i + 1].x - r[i].x, r[i + 1].z - r[i].z);
                    assert(std::fabs(r[i + 1].y - r[i].y) / run < 0.45f); // grade limit
                }
                // carved surface sits on the road profile
                assert(std::fabs(t.heightAt(r[i].x, r[i].z) - r[i].y) < 0.5f);
            }
            ++totalRoads;
        }
        totalOutposts += static_cast<int>(a.outposts().size());
        // far from everything: untouched
        assert(t.heightAt(-590, 590) == t.rawHeightAt(-590, 590) || a.near(-590, 590, 30));
    }
    std::printf("roads ok: %d roads, %d outposts over 3 seeds\n", totalRoads, totalOutposts);
    return 0;
}
