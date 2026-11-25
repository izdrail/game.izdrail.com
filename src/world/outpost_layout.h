#pragma once
#include <vector>
#include "roads.h"

// Oriented box used for placeholder outpost geometry (walls, towers, bunkers, crates).
struct OutpostBox
{
    float cx, cy, cz;     // centre (world)
    float hx, hy, hz;     // half extents (local axes)
    float yaw;            // rotation about Y (radians)
    float r, g, b;        // linear-ish albedo
};

// Deterministic outpost layout around an outpost pad: perimeter wall with a gate, four watch
// towers, command bunker, barracks, sandbag cover and supply crates. Placeholder geometry until
// glTF assets are imported (same layout will place those models).
void buildOutpostBoxes(const Outpost &o, std::vector<OutpostBox> &out);

// glb props (file under assets/resources/props, height in metres, world position, yaw) for an outpost
struct OutpostProp { const char *file; float height, x, y, z, yaw; float r, g, b; };
void buildOutpostProps(const Outpost &o, std::vector<OutpostProp> &out);
