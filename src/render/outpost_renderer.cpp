#include "outpost_renderer.h"
#include <cmath>

void OutpostRenderer::init(const std::vector<OutpostBox> &boxes)
{
    shader_ = Shader(ASSET_PATH "/shaders/outpost.vert", ASSET_PATH "/shaders/outpost.frag");
    boxCount_ = boxes.size();
    std::vector<float> v;
    v.reserve(boxes.size() * 36 * 9);
    static const float N[6][3] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};
    for (const OutpostBox &b : boxes)
    {
        float c = std::cos(b.yaw), s = std::sin(b.yaw);
        auto rot = [&](float x, float z, float &ox, float &oz) { ox = x * c - z * s; oz = x * s + z * c; };
        for (int f = 0; f < 6; ++f)
        {
            int ax = f / 2;
            int u = (ax + 1) % 3, w = (ax + 2) % 3;
            float sign = (f % 2 == 0) ? 1.0f : -1.0f;
            float h[3] = {b.hx, b.hy, b.hz};
            float corner[4][3];
            int k = 0;
            for (int du = -1; du <= 1; du += 2)
                for (int dw = -1; dw <= 1; dw += 2)
                {
                    corner[k][ax] = sign * h[ax];
                    corner[k][u] = du * h[u];
                    corner[k][w] = dw * h[w];
                    ++k;
                }
            int order[6] = {0, 1, 3, 0, 3, 2};
            float nx, nz;
            rot(N[f][0], N[f][2], nx, nz);
            // shade the faces slightly differently so boxes read as solid shapes
            float shade = f == 2 ? 1.0f : (f == 3 ? 0.5f : 0.82f);
            for (int q = 0; q < 6; ++q)
            {
                float *p = corner[order[q]];
                float px, pz;
                rot(p[0], p[2], px, pz);
                v.insert(v.end(), {b.cx + px, b.cy + p[1], b.cz + pz, nx, N[f][1], nz, b.r * shade, b.g * shade, b.b * shade});
            }
        }
    }
    count_ = static_cast<GLsizei>(v.size() / 9);
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(float), v.data(), GL_STATIC_DRAW);
    for (int i = 0; i < 3; ++i)
    {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(i * 3 * sizeof(float)));
    }
    glBindVertexArray(0);
}

void OutpostRenderer::draw(const glm::mat4 &view, const glm::mat4 &projection, const TerrainStyle &st)
{
    if (!count_) return;
    shader_.use();
    shader_.setMat4("projection", projection);
    shader_.setMat4("view", view);
    shader_.setVec3("cameraPos", glm::vec3(glm::inverse(view)[3]));
    shader_.setVec3("fogColor", st.fogColor);
    shader_.setFloat("fogDensity", st.fogDensity);
    shader_.setVec3("sunColor", st.sunColor);
    shader_.setVec3("skyAmbient", st.skyAmbient);
    shader_.setVec3("groundAmbient", st.groundAmbient);
    shader_.setFloat("exposure", st.exposure);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, count_);
    glBindVertexArray(0);
}
