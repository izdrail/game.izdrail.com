#pragma once
#include <vector>
#include "terrain_config.h"

class RoadNetwork;

// Pure CPU, GL-free, thread-safe (const) height field. Single source of truth for
// rendering, collision, vegetation placement and AI/navigation queries.
class Terrain
{
public:
    explicit Terrain(const TerrainConfig &cfg) : cfg_(cfg) {}
    const TerrainConfig &config() const { return cfg_; }

    // Height at world (x,z). Pure function of (config, x, z); chunk-independent,
    // so neighbouring chunks agree exactly on shared edges (no cracks).
    // heightAt includes road/outpost carving when a RoadNetwork is attached; rawHeightAt does not.
    float heightAt(float x, float z) const;
    float rawHeightAt(float x, float z) const;
    void attachRoads(const RoadNetwork *r) { roads_ = r; }
    const RoadNetwork *roads() const { return roads_; }
    // 0..1 road / outpost-pad surface mask (0 when no roads attached).
    float roadMaskAt(float x, float z) const;
    // Surface slope in degrees (0 flat .. 90).
    float slopeDegAt(float x, float z) const;
    // 0..1 vegetation density driver (low frequency, clumps forests/meadows).
    float moistureAt(float x, float z) const;
    bool isWater(float x, float z) const { return heightAt(x, z) < cfg_.waterLevel; }

    // Fills (chunkCells+1)^2 heights for chunk (cx,cz); chunk 0,0 starts at -worldSize/2.
    void buildChunkHeights(int cx, int cz, std::vector<float> &out) const;
    int chunksPerSide() const;

private:
    TerrainConfig cfg_;
    const RoadNetwork *roads_ = nullptr;
};
