#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include "../world/terrain.h"

// Chunked terrain mesh. Meshes are built once (never per frame). Chunks outside the
// camera frustum are skipped. Shared chunk edges use identical heights/normals (no cracks).
class TerrainRenderer
{
public:
    TerrainRenderer() = default;
    void init(const Terrain &terrain);
    void draw(const glm::mat4 &view, const glm::mat4 &projection, unsigned int groundTexture,
              const glm::vec3 &fogColor, float fogDensity);
    size_t chunkCount() const { return chunks_.size(); }
    size_t lastDrawn() const { return lastDrawn_; }

private:
    struct Chunk { GLuint vao = 0, vbo = 0, ebo = 0; GLsizei indexCount = 0; glm::vec3 center; float radius = 0; };
    std::vector<Chunk> chunks_;
    Shader shader_;
    size_t lastDrawn_ = 0;
};
