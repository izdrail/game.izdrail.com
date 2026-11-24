#include "terrain.h"
#include "roads.h"
#include <cmath>

namespace
{
inline uint64_t mix(uint64_t x)
{
    x += 0x9E3779B97F4A7C15ull;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
    return x ^ (x >> 31);
}
inline float lattice(uint64_t seed, int ix, int iz)
{
    uint64_t h = mix(seed ^ mix(static_cast<uint64_t>(static_cast<uint32_t>(ix)) << 32 | static_cast<uint32_t>(iz)));
    return static_cast<float>(h >> 40) / 8388608.0f - 1.0f; // [-1,1)
}
inline float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
float valueNoise(uint64_t seed, float x, float z)
{
    int x0 = static_cast<int>(std::floor(x)), z0 = static_cast<int>(std::floor(z));
    float fx = fade(x - x0), fz = fade(z - z0);
    float a = lattice(seed, x0, z0), b = lattice(seed, x0 + 1, z0);
    float c = lattice(seed, x0, z0 + 1), d = lattice(seed, x0 + 1, z0 + 1);
    float u = a + (b - a) * fx, v = c + (d - c) * fx;
    return u + (v - u) * fz;
}
float fbm(const TerrainConfig &c, uint64_t seed, float x, float z, bool ridged)
{
    float amp = 1, freq = c.frequency, sum = 0, norm = 0;
    for (int i = 0; i < c.octaves; ++i)
    {
        float n = valueNoise(seed + i * 101, x * freq, z * freq);
        if (ridged) n = 1.0f - 2.0f * std::fabs(n);
        sum += n * amp;
        norm += amp;
        amp *= c.gain;
        freq *= c.lacunarity;
    }
    return sum / norm;
}
} // namespace

float Terrain::heightAt(float x, float z) const
{
    float raw = rawHeightAt(x, z);
    if (!roads_) return raw;
    float target;
    float w = roads_->carve(x, z, target);
    return w > 0 ? raw + (target - raw) * w : raw;
}

float Terrain::roadMaskAt(float x, float z) const { return roads_ ? roads_->surfaceMask(x, z) : 0.0f; }

float Terrain::rawHeightAt(float x, float z) const
{
    const TerrainConfig &c = cfg_;
    float wx = x + c.warpStrength * fbm(c, c.seed ^ 0xA1, x + 31.7f, z - 17.3f, false);
    float wz = z + c.warpStrength * fbm(c, c.seed ^ 0xB2, x - 53.1f, z + 9.9f, false);
    float hills = fbm(c, c.seed, wx, wz, false);
    float ridges = fbm(c, c.seed ^ 0xC3, wx, wz, true);
    // mountains only where a low-frequency mask is high
    float mask = 0.5f + 0.5f * valueNoise(c.seed ^ 0xD4, wx * c.frequency * 0.35f, wz * c.frequency * 0.35f);
    float m = mask * mask * c.mountainBlend;
    float h = hills * (1.0f - m) + (ridges * 1.4f - 0.2f) * m;
    float hgt = h * c.heightScale;
    // keep the spawn area flat so the player starts on level ground
    float d = std::sqrt(x * x + z * z);
    float t = (d - c.flatRadius) / c.flatBlend;
    t = t < 0 ? 0 : (t > 1 ? 1 : t);
    hgt *= t * t * (3 - 2 * t);
    return c.baseHeight + hgt;
}

float Terrain::moistureAt(float x, float z) const
{
    float m = 0.5f + 0.5f * fbm(cfg_, cfg_.seed ^ 0xE5, x * 1.7f + 200.f, z * 1.7f - 90.f, false) * 1.6f;
    return m < 0 ? 0 : (m > 1 ? 1 : m);
}

float Terrain::slopeDegAt(float x, float z) const
{
    const float e = cfg_.cellSize;
    float dx = (heightAt(x + e, z) - heightAt(x - e, z)) / (2 * e);
    float dz = (heightAt(x, z + e) - heightAt(x, z - e)) / (2 * e);
    return std::atan(std::sqrt(dx * dx + dz * dz)) * 57.29578f;
}

int Terrain::chunksPerSide() const
{
    return static_cast<int>(std::ceil(cfg_.worldSize / (cfg_.chunkCells * cfg_.cellSize)));
}

void Terrain::buildChunkHeights(int cx, int cz, std::vector<float> &out) const
{
    const int n = cfg_.chunkCells + 1;
    out.resize(static_cast<size_t>(n) * n);
    const float span = cfg_.chunkCells * cfg_.cellSize;
    const float ox = -cfg_.worldSize * 0.5f + cx * span, oz = -cfg_.worldSize * 0.5f + cz * span;
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i)
            out[static_cast<size_t>(j) * n + i] = heightAt(ox + i * cfg_.cellSize, oz + j * cfg_.cellSize);
}
