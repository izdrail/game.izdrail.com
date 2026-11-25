#include "weapon_model.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

void WeaponModel::normalise(std::vector<float> &v, float length)
{
    const size_t n = v.size() / 9;
    if (!n) return;
    glm::vec3 lo(1e30f), hi(-1e30f);
    for (size_t i = 0; i < n; ++i)
        for (int a = 0; a < 3; ++a) { lo[a] = std::min(lo[a], v[i * 9 + a]); hi[a] = std::max(hi[a], v[i * 9 + a]); }
    glm::vec3 ext = hi - lo;
    // barrel axis = longest side
    int ax = 0;
    if (ext.y > ext[ax]) ax = 1;
    if (ext.z > ext[ax]) ax = 2;
    // up axis: the shorter of the other two that is larger in extent is usually height; guns are
    // taller than they are wide, so pick the larger remaining extent as up
    int o1 = (ax + 1) % 3, o2 = (ax + 2) % 3;
    int up = ext[o1] >= ext[o2] ? o1 : o2;
    int side = (up == o1) ? o2 : o1;
    // grip and magazine hang below the barrel: find which end of the barrel axis the low-mass is near
    float midUp = 0.5f * (lo[up] + hi[up]);
    float lowCenter = 0; int lowCount = 0;
    for (size_t i = 0; i < n; ++i)
        if (v[i * 9 + up] < midUp - 0.15f * ext[up]) { lowCenter += v[i * 9 + ax]; ++lowCount; }
    lowCenter = lowCount ? lowCenter / lowCount : 0.5f * (lo[ax] + hi[ax]);
    // muzzle is on the opposite side of the grip: flip so the grip is at -X
    float flip = (lowCenter > 0.5f * (lo[ax] + hi[ax])) ? -1.0f : 1.0f;
    float scale = length / ext[ax];
    // should the up axis be flipped? grips hang down, so the heavier lower region has smaller up values
    // (assumes source models use +Y or +Z up, which holds for glTF and the packs used)
    for (size_t i = 0; i < n; ++i)
    {
        float *p = &v[i * 9];
        glm::vec3 pos(p[0], p[1], p[2]), nrm(p[3], p[4], p[5]);
        float x = (pos[ax] - 0.5f * (lo[ax] + hi[ax])) * scale * flip;
        float y = (pos[up] - lo[up]) * scale;
        float z = (pos[side] - 0.5f * (lo[side] + hi[side])) * scale * flip;
        float nx = nrm[ax] * flip, ny = nrm[up], nz = nrm[side] * flip;
        // grip region sits at the origin: shift so the rear 30% of the barrel length is behind it
        x += 0.2f * length;
        p[0] = x; p[1] = y; p[2] = z;
        p[3] = nx; p[4] = ny; p[5] = nz;
    }
}

bool WeaponModel::load(const std::string &path, float length)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);
    if (!scene || !scene->mRootNode)
    {
        std::cerr << "weapon model: cannot load " << path << ": " << importer.GetErrorString() << std::endl;
        return false;
    }
    std::vector<float> v;
    for (unsigned m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh *mesh = scene->mMeshes[m];
        aiColor4D diff(0.6f, 0.6f, 0.6f, 1.0f);
        if (mesh->mMaterialIndex < scene->mNumMaterials) { aiMaterial *mt = scene->mMaterials[mesh->mMaterialIndex]; aiColor4D c; if (mt->Get(AI_MATKEY_BASE_COLOR, c) == AI_SUCCESS) diff = c; else if (mt->Get(AI_MATKEY_COLOR_DIFFUSE, c) == AI_SUCCESS && (c.r + c.g + c.b) > 0.01f) diff = c; }
        for (unsigned f = 0; f < mesh->mNumFaces; ++f)
        {
            const aiFace &face = mesh->mFaces[f];
            if (face.mNumIndices != 3) continue;
            for (int k = 0; k < 3; ++k)
            {
                unsigned i = face.mIndices[k];
                aiVector3D p = mesh->mVertices[i], n = mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D(0, 1, 0);
                float r = diff.r, g = diff.g, b = diff.b;
                if (mesh->HasVertexColors(0)) { r *= mesh->mColors[0][i].r; g *= mesh->mColors[0][i].g; b *= mesh->mColors[0][i].b; }
                v.insert(v.end(), {p.x, p.y, p.z, n.x, n.y, n.z, r, g, b});
            }
        }
    }
    if (v.empty()) return false;
    normalise(v, length);
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
    return true;
}

void WeaponModel::draw(Shader &shader, const glm::mat4 &model, const glm::mat4 &view, const glm::mat4 &projection) const
{
    if (!count_) return;
    shader.use();
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, count_);
    glBindVertexArray(0);
}
