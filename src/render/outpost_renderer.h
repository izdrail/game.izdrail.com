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
    void init(const std::vector<OutpostBox> &boxes);
    void draw(const glm::mat4 &view, const glm::mat4 &projection, const TerrainStyle &style);
    size_t boxCount() const { return boxCount_; }

private:
    Shader shader_;
    GLuint vao_ = 0, vbo_ = 0;
    GLsizei count_ = 0;
    size_t boxCount_ = 0;
};
