#pragma once
#include <string>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

// First-person weapon mesh loaded from glTF/GLB (via assimp). Materials are flat colours.
// The mesh is normalised: barrel along +X (muzzle at +X), up is +Y, longest side = `length`,
// origin near the grip. This matches the orientation of the gun matrix used by the player.
class WeaponModel
{
public:
    WeaponModel() = default;
    // Returns false when the file cannot be loaded.
    bool load(const std::string &path, float length);
    void draw(Shader &shader, const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection) const;
    bool valid() const { return count_ > 0; }

    // Pure geometry helper (testable, no GL): normalises interleaved pos(3) normal(3) colour(3) vertices.
    static void normalise(std::vector<float> &verts, float length);

private:
    GLuint vao_ = 0, vbo_ = 0;
    GLsizei count_ = 0;
};
