#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"
#include "../world/terrain.h"

// Chunked terrain mesh. Meshes are built once (never per frame). Chunks outside the
// camera frustum are skipped. Shared chunk edges use identical heights/normals (no cracks).
// Per-environment look of the terrain (lighting, fog, snow line, dryness).
struct TerrainStyle
{
    glm::vec3 fogColor{0.62f, 0.70f, 0.78f};
    float fogDensity = 0.0022f;
    float snowLine = 55.0f;        // world y above which snow appears
    float dryness = 0.0f;          // 0 lush grass .. 1 dry dirt/sand
    glm::vec3 sunColor{1.0f, 0.93f, 0.80f};
    glm::vec3 skyAmbient{0.34f, 0.42f, 0.55f};
    glm::vec3 groundAmbient{0.20f, 0.17f, 0.12f};
    float exposure = 1.0f;
    float waterLevel = -10.8f;
};

class TerrainRenderer
{
public:
    TerrainRenderer() = default;
    void init(const Terrain &terrain);
    void draw(const glm::mat4 &view, const glm::mat4 &projection, const TerrainStyle &style);
    void drawWater(const glm::mat4 &view, const glm::mat4 &projection, const TerrainStyle &style, float time);
    size_t chunkCount() const { return chunks_.size(); }
    size_t lastDrawn() const { return lastDrawn_; }

private:
    struct Chunk { GLuint vao = 0, vbo = 0, ebo = 0; GLsizei indexCount = 0; glm::vec3 center; float radius = 0; };
    std::vector<Chunk> chunks_;
    Shader shader_, waterShader_;
    GLuint waterVao_ = 0, waterVbo_ = 0;
    float half_ = 0;
    GLuint tex_[12] = {0};
    size_t lastDrawn_ = 0;
};
