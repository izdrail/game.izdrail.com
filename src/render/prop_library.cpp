#include "prop_library.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "stb_image.h"

static bool loadNormalised(const std::string &path, float height, std::vector<float> &v)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices);
    if (!scene || !scene->mRootNode)
    {
        std::cerr << "prop: cannot load " << path << ": " << importer.GetErrorString() << std::endl;
        return false;
    }
    for (unsigned m = 0; m < scene->mNumMeshes; ++m)
    {
        const aiMesh *mesh = scene->mMeshes[m];
        aiColor4D diff(0.6f, 0.6f, 0.6f, 1.0f);
        std::vector<unsigned char> pixels;
        int tw = 0, th = 0;
        if (mesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiMaterial *mt = scene->mMaterials[mesh->mMaterialIndex];
            aiColor4D c;
            if (mt->Get(AI_MATKEY_BASE_COLOR, c) == AI_SUCCESS) diff = c;
            else if (mt->Get(AI_MATKEY_COLOR_DIFFUSE, c) == AI_SUCCESS && (c.r + c.g + c.b) > 0.01f) diff = c;
            aiString tp;
            if (mt->GetTexture(aiTextureType_DIFFUSE, 0, &tp) == AI_SUCCESS || mt->GetTexture(aiTextureType_BASE_COLOR, 0, &tp) == AI_SUCCESS)
            {
                const aiTexture *et = scene->GetEmbeddedTexture(tp.C_Str());
                if (et && et->mHeight == 0)
                {
                    int w = 0, h = 0, ch = 0;
                    unsigned char *px = stbi_load_from_memory(reinterpret_cast<const unsigned char *>(et->pcData), static_cast<int>(et->mWidth), &w, &h, &ch, 4);
                    if (px) { pixels.assign(px, px + w * h * 4); tw = w; th = h; stbi_image_free(px); }
                }
            }
        }
        for (unsigned f = 0; f < mesh->mNumFaces; ++f)
        {
            const aiFace &face = mesh->mFaces[f];
            if (face.mNumIndices != 3) continue;
            for (int k = 0; k < 3; ++k)
            {
                unsigned i = face.mIndices[k];
                aiVector3D p = mesh->mVertices[i], n = mesh->HasNormals() ? mesh->mNormals[i] : aiVector3D(0, 1, 0);
                float r = diff.r, g = diff.g, b = diff.b;
                if (!pixels.empty() && mesh->HasTextureCoords(0))
                {
                    float u = mesh->mTextureCoords[0][i].x, vv = mesh->mTextureCoords[0][i].y;
                    u -= std::floor(u); vv -= std::floor(vv);
                    int tx = std::min(tw - 1, static_cast<int>(u * tw)), ty = std::min(th - 1, static_cast<int>(vv * th)); // glTF uv origin is top-left, as stb loads
                    const unsigned char *q = &pixels[(ty * tw + tx) * 4];
                    r = q[0] / 255.0f; g = q[1] / 255.0f; b = q[2] / 255.0f;
                }
                if (mesh->HasVertexColors(0)) { r *= mesh->mColors[0][i].r; g *= mesh->mColors[0][i].g; b *= mesh->mColors[0][i].b; }
                v.insert(v.end(), {p.x, p.y, p.z, n.x, n.y, n.z, r, g, b});
            }
        }
    }
    if (v.empty()) return false;
    const size_t n = v.size() / 9;
    float lo[3] = {1e30f, 1e30f, 1e30f}, hi[3] = {-1e30f, -1e30f, -1e30f};
    for (size_t i = 0; i < n; ++i)
        for (int a = 0; a < 3; ++a) { lo[a] = std::min(lo[a], v[i * 9 + a]); hi[a] = std::max(hi[a], v[i * 9 + a]); }
    float sc = height / std::max(hi[1] - lo[1], 1e-4f);
    for (size_t i = 0; i < n; ++i)
    {
        float *p = &v[i * 9];
        p[0] = (p[0] - 0.5f * (lo[0] + hi[0])) * sc;
        p[1] = (p[1] - lo[1]) * sc;
        p[2] = (p[2] - 0.5f * (lo[2] + hi[2])) * sc;
    }
    return true;
}

bool PropLibrary::bake(const std::string &glbPath, float height, float x, float y, float z, float yaw, std::vector<float> &out, float tr, float tg, float tb)
{
    std::string key = glbPath + "@" + std::to_string(height);
    auto it = cache_.find(key);
    if (it == cache_.end())
    {
        std::vector<float> v;
        if (!loadNormalised(glbPath, height, v)) v.clear();
        it = cache_.emplace(key, std::move(v)).first;
    }
    const std::vector<float> &src = it->second;
    if (src.empty()) return false;
    float c = std::cos(yaw), s = std::sin(yaw);
    for (size_t i = 0; i + 8 < src.size(); i += 9)
    {
        const float *p = &src[i];
        out.insert(out.end(), {x + p[0] * c + p[2] * s, y + p[1], z - p[0] * s + p[2] * c,
                               p[3] * c + p[5] * s, p[4], -p[3] * s + p[5] * c, p[6] * tr, p[7] * tg, p[8] * tb});
    }
    return true;
}
