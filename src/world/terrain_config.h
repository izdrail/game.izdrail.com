#pragma once
#include <cstdint>

// Configuration for deterministic procedural terrain. Same config => identical output.
struct TerrainConfig
{
    uint64_t seed = 1337;
    float worldSize = 512.0f;    // metres, square, centred on (0,0)
    int chunkCells = 64;         // quads per chunk edge
    float cellSize = 2.0f;       // metres per quad
    float baseHeight = -1.8f;    // matches World::GROUND_Y
    float heightScale = 40.0f;   // max amplitude
    float frequency = 0.004f;    // base noise frequency (1/m)
    int octaves = 5;
    float lacunarity = 2.0f;
    float gain = 0.5f;
    float warpStrength = 30.0f;  // domain warp (m)
    float mountainBlend = 0.6f;  // 0 = rolling hills only, 1 = strong ridged mountains
    float waterLevel = -4.0f;    // height below which water is placed
    float flatRadius = 18.0f;    // fully flat around the origin (player start)
    float flatBlend = 60.0f;     // distance over which terrain rises to full height
};
