#pragma once
#include <vector>
#include <cstdint>

class Terrain;

// Military outposts and the roads joining them. Pure CPU, deterministic for a given terrain.
// Roads follow A* paths over a slope-weighted grid and are carved into the height field
// (flattened corridor + shoulder). Outposts sit on levelled pads.
struct Outpost
{
    float x = 0, z = 0, yaw = 0;
    float padRadius = 24.0f; // fully level radius
    float padHeight = 0;     // world y of the pad
};

struct RoadPoint { float x, z, y; };

class RoadNetwork
{
public:
    static constexpr float kHalfWidth = 3.5f; // road surface half width (m)
    static constexpr float kShoulder = 6.0f;  // blend distance to natural terrain (m)
    static constexpr float kPadBlend = 14.0f;

    // Builds outposts + roads using the terrain's uncarved height.
    void build(const Terrain &terrain, int outpostCount = 4);

    const std::vector<Outpost> &outposts() const { return outposts_; }
    const std::vector<std::vector<RoadPoint>> &roads() const { return roads_; }

    // 0..1 how much carving applies at (x,z) and the height to carve toward.
    // Returns weight; outHeight is valid when weight > 0.
    float carve(float x, float z, float &outHeight) const;
    // 0..1 surface mask: 1 on the road / outpost pad, fading to 0 at the verges.
    float surfaceMask(float x, float z) const;
    // True when (x,z) is within margin metres of a road surface or an outpost pad.
    bool near(float x, float z, float margin) const;

private:
    bool findPath(const Terrain &t, float ax, float az, float bx, float bz, std::vector<RoadPoint> &out) const;
    void indexSegments();
    // nearest road distance and interpolated road height; false when none within range
    bool nearestRoad(float x, float z, float range, float &dist, float &h) const;

    std::vector<Outpost> outposts_;
    std::vector<std::vector<RoadPoint>> roads_;
    float half_ = 0;
    static constexpr float kCell = 32.0f;
    int gridN_ = 0;
    struct Seg { RoadPoint a, b; };
    std::vector<Seg> segs_;
    std::vector<std::vector<int>> grid_;
};
