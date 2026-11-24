#include "terrain_renderer.h"
#include <cmath>

void TerrainRenderer::init(const Terrain &terrain)
{
    shader_ = Shader(ASSET_PATH "/shaders/terrain.vert", ASSET_PATH "/shaders/terrain.frag");
    const TerrainConfig &c = terrain.config();
    const int n = c.chunkCells + 1;
    const int per = terrain.chunksPerSide();
    const float span = c.chunkCells * c.cellSize;

    // index buffer is identical for every chunk
    std::vector<unsigned int> idx;
    idx.reserve(static_cast<size_t>(c.chunkCells) * c.chunkCells * 6);
    for (int j = 0; j < c.chunkCells; ++j)
        for (int i = 0; i < c.chunkCells; ++i)
        {
            unsigned int a = j * n + i, b = a + 1, d = a + n, e = d + 1;
            idx.insert(idx.end(), {a, d, b, b, d, e});
        }

    std::vector<float> heights, verts;
    for (int cz = 0; cz < per; ++cz)
        for (int cx = 0; cx < per; ++cx)
        {
            terrain.buildChunkHeights(cx, cz, heights);
            const float ox = -c.worldSize * 0.5f + cx * span, oz = -c.worldSize * 0.5f + cz * span;
            verts.clear();
            float lo = 1e9f, hi = -1e9f;
            for (int j = 0; j < n; ++j)
                for (int i = 0; i < n; ++i)
                {
                    float x = ox + i * c.cellSize, z = oz + j * c.cellSize, h = heights[j * n + i];
                    // normals from the global height function => identical across chunk borders
                    float hx = terrain.heightAt(x + c.cellSize, z) - terrain.heightAt(x - c.cellSize, z);
                    float hz = terrain.heightAt(x, z + c.cellSize) - terrain.heightAt(x, z - c.cellSize);
                    glm::vec3 nrm = glm::normalize(glm::vec3(-hx, 2.0f * c.cellSize, -hz));
                    verts.insert(verts.end(), {x, h, z, nrm.x, nrm.y, nrm.z});
                    lo = std::fmin(lo, h); hi = std::fmax(hi, h);
                }
            Chunk ch;
            glGenVertexArrays(1, &ch.vao); glGenBuffers(1, &ch.vbo); glGenBuffers(1, &ch.ebo);
            glBindVertexArray(ch.vao);
            glBindBuffer(GL_ARRAY_BUFFER, ch.vbo);
            glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ch.ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
            glBindVertexArray(0);
            ch.indexCount = static_cast<GLsizei>(idx.size());
            ch.center = glm::vec3(ox + span * 0.5f, (lo + hi) * 0.5f, oz + span * 0.5f);
            ch.radius = std::sqrt(span * span * 0.5f + ((hi - lo) * 0.5f) * ((hi - lo) * 0.5f)) + 1.0f;
            chunks_.push_back(ch);
        }
}

void TerrainRenderer::draw(const glm::mat4 &view, const glm::mat4 &projection, unsigned int groundTexture,
                           const glm::vec3 &fogColor, float fogDensity)
{
    // frustum planes (Gribb/Hartmann) for sphere culling
    glm::mat4 m = projection * view;
    glm::vec4 pl[6];
    for (int i = 0; i < 3; ++i)
    {
        pl[i * 2] = glm::vec4(m[0][3] + m[0][i], m[1][3] + m[1][i], m[2][3] + m[2][i], m[3][3] + m[3][i]);
        pl[i * 2 + 1] = glm::vec4(m[0][3] - m[0][i], m[1][3] - m[1][i], m[2][3] - m[2][i], m[3][3] - m[3][i]);
    }
    for (auto &p : pl) p /= glm::length(glm::vec3(p));

    shader_.use();
    shader_.setMat4("projection", projection);
    shader_.setMat4("view", view);
    shader_.setVec3("cameraPos", glm::vec3(glm::inverse(view)[3]));
    shader_.setVec3("fogColor", fogColor);
    shader_.setFloat("fogDensity", fogDensity);
    shader_.setInt("groundTex", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, groundTexture);

    lastDrawn_ = 0;
    for (const Chunk &c : chunks_)
    {
        bool visible = true;
        for (const auto &p : pl)
            if (glm::dot(glm::vec3(p), c.center) + p.w < -c.radius) { visible = false; break; }
        if (!visible) continue;
        glBindVertexArray(c.vao);
        glDrawElements(GL_TRIANGLES, c.indexCount, GL_UNSIGNED_INT, 0);
        ++lastDrawn_;
    }
    glBindVertexArray(0);
}
