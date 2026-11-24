#include "terrain_renderer.h"
#include <cmath>
#include "texture_loading.h"

void TerrainRenderer::init(const Terrain &terrain)
{
    shader_ = Shader(ASSET_PATH "/shaders/terrain.vert", ASSET_PATH "/shaders/terrain.frag");
    const TerrainConfig &c = terrain.config();
    // PBR-style material set: color (sRGB), normal (GL), roughness for grass, dirt, rock, snow
    const char *mats[4] = {"grass", "dirt", "rock", "snow"};
    for (int m = 0; m < 4; ++m)
    {
        std::string base = mats[m];
        tex_[m * 3 + 0] = TextureFromFile((base + "_color.jpg").c_str(), ASSET_PATH "/resources/terrain", false, true);
        tex_[m * 3 + 1] = TextureFromFile((base + "_normal.jpg").c_str(), ASSET_PATH "/resources/terrain", false, false);
        tex_[m * 3 + 2] = TextureFromFile((base + "_rough.jpg").c_str(), ASSET_PATH "/resources/terrain", false, false);
        for (int k = 0; k < 3; ++k)
        {
            glBindTexture(GL_TEXTURE_2D, tex_[m * 3 + k]);
            // anisotropic filtering left at driver default
        }
    }
    waterShader_ = Shader(ASSET_PATH "/shaders/water.vert", ASSET_PATH "/shaders/water.frag");
    half_ = c.worldSize * 0.5f;
    float q[] = {-half_, 0, -half_, half_, 0, -half_, half_, 0, half_, -half_, 0, -half_, half_, 0, half_, -half_, 0, half_};
    glGenVertexArrays(1, &waterVao_); glGenBuffers(1, &waterVbo_);
    glBindVertexArray(waterVao_); glBindBuffer(GL_ARRAY_BUFFER, waterVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(q), q, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glBindVertexArray(0);
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
                    verts.insert(verts.end(), {x, h, z, nrm.x, nrm.y, nrm.z, terrain.roadMaskAt(x, z)});
                    lo = std::fmin(lo, h); hi = std::fmax(hi, h);
                }
            Chunk ch;
            glGenVertexArrays(1, &ch.vao); glGenBuffers(1, &ch.vbo); glGenBuffers(1, &ch.ebo);
            glBindVertexArray(ch.vao);
            glBindBuffer(GL_ARRAY_BUFFER, ch.vbo);
            glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ch.ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(2); glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void *)(6 * sizeof(float)));
            glBindVertexArray(0);
            ch.indexCount = static_cast<GLsizei>(idx.size());
            ch.center = glm::vec3(ox + span * 0.5f, (lo + hi) * 0.5f, oz + span * 0.5f);
            ch.radius = std::sqrt(span * span * 0.5f + ((hi - lo) * 0.5f) * ((hi - lo) * 0.5f)) + 1.0f;
            chunks_.push_back(ch);
        }
}

void TerrainRenderer::draw(const glm::mat4 &view, const glm::mat4 &projection, const TerrainStyle &st)
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
    shader_.setVec3("fogColor", st.fogColor);
    shader_.setFloat("fogDensity", st.fogDensity);
    shader_.setFloat("snowLine", st.snowLine);
    shader_.setFloat("dryness", st.dryness);
    shader_.setFloat("waterLevel", st.waterLevel);
    shader_.setFloat("exposure", st.exposure);
    shader_.setVec3("sunColor", st.sunColor);
    shader_.setVec3("skyAmbient", st.skyAmbient);
    shader_.setVec3("groundAmbient", st.groundAmbient);
    static const char *names[12] = {"grassC", "grassN", "grassR", "dirtC", "dirtN", "dirtR", "rockC", "rockN", "rockR", "snowC", "snowN", "snowR"};
    for (int i = 0; i < 12; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, tex_[i]);
        shader_.setInt(names[i], i);
    }

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
    glActiveTexture(GL_TEXTURE0);
}

void TerrainRenderer::drawWater(const glm::mat4 &view, const glm::mat4 &projection, const TerrainStyle &st, float time)
{
    waterShader_.use();
    waterShader_.setMat4("projection", projection);
    waterShader_.setMat4("view", view);
    waterShader_.setFloat("level", st.waterLevel);
    waterShader_.setFloat("time", time);
    waterShader_.setVec3("cameraPos", glm::vec3(glm::inverse(view)[3]));
    waterShader_.setVec3("fogColor", st.fogColor);
    waterShader_.setFloat("fogDensity", st.fogDensity);
    waterShader_.setVec3("sunColor", st.sunColor);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(waterVao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
}
