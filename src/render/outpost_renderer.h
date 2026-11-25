#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include "terrain_renderer.h"
#include "../world/outpost_layout.h"

// Draws outpost placeholder geometry as one static mesh (position, normal, colour).
class OutpostRenderer
{
public:
    // extra pre-baked triangles (position, normal, colour) drawn together with the boxes
    void init(const std::vector<OutpostBox> &boxes, const std::vector<float> &extra = {});
    void draw(const glm::mat4 &view, const glm::mat4 &projection, const TerrainStyle &style);
    size_t boxCount() const { return boxCount_; }

private:
    Shader shader_;
    GLuint vao_ = 0, vbo_ = 0;
    GLsizei count_ = 0;
    size_t boxCount_ = 0;
};
