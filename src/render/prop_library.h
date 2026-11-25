#pragma once
#include <map>
#include <string>
#include <vector>

// Loads static glb props (CPU only) and bakes instances into a flat vertex list
// (position, normal, colour) that the outpost renderer draws in one call.
class PropLibrary
{
public:
    // Appends one instance. The model is scaled to `height` metres tall, centred on x/z with its
    // base at world y, and rotated by yaw (radians) about Y. The colour is multiplied by (tr, tg, tb)
    // so palette-textured models without their texture file still read as the right material. Returns false if the file is missing.
    bool bake(const std::string &glbPath, float height, float x, float y, float z, float yaw, std::vector<float> &out,
              float tr = 1.0f, float tg = 1.0f, float tb = 1.0f);

private:
    std::map<std::string, std::vector<float>> cache_; // key: path + height
};
