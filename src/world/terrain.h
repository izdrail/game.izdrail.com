#pragma once
#include <vector>
#include "terrain_config.h"

// Pure CPU, GL-free, thread-safe (const) height field. Single source of truth for
// rendering, collision, vegetation placement and AI/navigation queries.
class Terrain
{
public:
    explicit Terrain(const TerrainConfig &cfg) : cfg_(cfg) {}
    const TerrainConfig &config() const { return cfg_; }

    // Height at world (x,z). Pure function of (config, x, z); chunk-independent,
    // so neighbouring chunks agree exactly on shared edges (no cracks).
    float heightAt(float x, float z) const;
    // Surface slope in degrees (0 flat .. 90).
    float slopeDegAt(float x, float z) const;
    bool isWater(float x, float z) const { return heightAt(x, z) < cfg_.waterLevel; }

    // Fills (chunkCells+1)^2 heights for chunk (cx,cz); chunk 0,0 starts at -worldSize/2.
    void buildChunkHeights(int cx, int cz, std::vector<float> &out) const;
    int chunksPerSide() const;

private:
    TerrainConfig cfg_;
};
