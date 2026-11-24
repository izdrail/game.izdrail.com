#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>
#include "../src/world/terrain.h"

int main()
{
    TerrainConfig c;
    Terrain a(c), b(c);
    // determinism: same seed, same heights
    for (int i = 0; i < 1000; ++i) { float x = i * 0.73f - 300, z = i * 1.31f - 300; assert(a.heightAt(x, z) == b.heightAt(x, z)); }
    // different seed differs
    TerrainConfig c2 = c; c2.seed = 42; Terrain d(c2);
    int diff = 0; for (int i = 0; i < 100; ++i) if (a.heightAt(i * 3.f, i * 2.f) != d.heightAt(i * 3.f, i * 2.f)) ++diff;
    assert(diff > 90);
    // chunk boundary: shared edge identical bit-for-bit (no cracks)
    std::vector<float> l, r, up, dn; const int n = c.chunkCells + 1;
    a.buildChunkHeights(1, 1, l); a.buildChunkHeights(2, 1, r);
    for (int j = 0; j < n; ++j) assert(l[j * n + (n - 1)] == r[j * n + 0]);
    a.buildChunkHeights(1, 2, dn);
    for (int i = 0; i < n; ++i) assert(l[(n - 1) * n + i] == dn[i]);
    // bounds & sanity
    float lo = 1e9f, hi = -1e9f;
    for (int i = 0; i < 20000; ++i) { float h = a.heightAt((i % 141) * 3.6f - 250, (i / 141) * 3.6f - 250); assert(std::isfinite(h)); lo = std::fmin(lo, h); hi = std::fmax(hi, h); }
    assert(hi - lo > 5.0f && hi < c.baseHeight + c.heightScale * 2 && lo > c.baseHeight - c.heightScale * 2);
    assert(a.slopeDegAt(0, 0) >= 0 && a.slopeDegAt(0, 0) < 90);
    std::printf("terrain ok: height range %.1f..%.1f, chunks/side %d\n", lo, hi, a.chunksPerSide());
    return 0;
}
