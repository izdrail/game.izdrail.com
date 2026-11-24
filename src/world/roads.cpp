#include "roads.h"
#include "terrain.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <random>

namespace
{
inline float sstep(float a, float b, float x)
{
    float t = (x - a) / (b - a);
    t = t < 0 ? 0 : (t > 1 ? 1 : t);
    return t * t * (3 - 2 * t);
}
} // namespace

bool RoadNetwork::findPath(const Terrain &t, float ax, float az, float bx, float bz, std::vector<RoadPoint> &out) const
{
    const float step = 8.0f;
    const float half = t.config().worldSize * 0.5f - 20.0f;
    const int n = static_cast<int>(std::floor(2 * half / step)) + 1;
    auto wx = [&](int i) { return -half + i * step; };
    auto toI = [&](float v) { return std::clamp(static_cast<int>(std::lround((v + half) / step)), 0, n - 1); };
    std::vector<float> h(static_cast<size_t>(n) * n);
    for (int j = 0; j < n; ++j)
        for (int i = 0; i < n; ++i) h[j * n + i] = t.rawHeightAt(wx(i), wx(j));
    const float wl = t.config().waterLevel + 1.0f;
    int si = toI(ax), sj = toI(az), gi = toI(bx), gj = toI(bz);
    std::vector<float> g(h.size(), 1e30f);
    std::vector<int> prev(h.size(), -1);
    using Q = std::pair<float, int>;
    std::priority_queue<Q, std::vector<Q>, std::greater<Q>> open;
    auto heur = [&](int i, int j) { return std::hypot(float(i - gi), float(j - gj)) * step; };
    g[sj * n + si] = 0;
    open.push({heur(si, sj), sj * n + si});
    while (!open.empty())
    {
        auto [f, cur] = open.top();
        open.pop();
        int ci = cur % n, cj = cur / n;
        if (ci == gi && cj == gj) break;
        if (f > g[cur] + heur(ci, cj) + 1e-3f) continue;
        for (int dj = -1; dj <= 1; ++dj)
            for (int di = -1; di <= 1; ++di)
            {
                if (!di && !dj) continue;
                int ni = ci + di, nj = cj + dj;
                if (ni < 0 || nj < 0 || ni >= n || nj >= n) continue;
                int nid = nj * n + ni;
                if (h[nid] < wl) continue; // never route through water
                float len = std::hypot(float(di), float(dj)) * step;
                float s = std::fabs(h[nid] - h[cur]) / len;
                if (s > 0.55f) continue;
                float cost = len * (1.0f + 14.0f * s * s) + (h[nid] < wl + 3.0f ? len * 1.5f : 0.0f);
                if (g[cur] + cost < g[nid])
                {
                    g[nid] = g[cur] + cost;
                    prev[nid] = cur;
                    open.push({g[nid] + heur(ni, nj), nid});
                }
            }
    }
    if (prev[gj * n + gi] < 0 && !(si == gi && sj == gj)) return false;
    std::vector<std::pair<float, float>> pts;
    for (int c = gj * n + gi; c >= 0; c = prev[c]) pts.push_back({wx(c % n), wx(c / n)});
    std::reverse(pts.begin(), pts.end());
    pts.front() = {ax, az};
    pts.back() = {bx, bz};
    // smooth the staircase of grid cells (keeps ends fixed)
    for (int pass = 0; pass < 4; ++pass)
        for (size_t i = 1; i + 1 < pts.size(); ++i)
        {
            pts[i].first = 0.25f * pts[i - 1].first + 0.5f * pts[i].first + 0.25f * pts[i + 1].first;
            pts[i].second = 0.25f * pts[i - 1].second + 0.5f * pts[i].second + 0.25f * pts[i + 1].second;
        }
    // resample to ~6 m spacing
    std::vector<RoadPoint> res;
    for (size_t i = 0; i + 1 < pts.size(); ++i)
    {
        float dx = pts[i + 1].first - pts[i].first, dz = pts[i + 1].second - pts[i].second;
        int k = std::max(1, static_cast<int>(std::ceil(std::hypot(dx, dz) / 6.0f)));
        for (int m = 0; m < k; ++m)
            res.push_back({pts[i].first + dx * m / k, pts[i].second + dz * m / k, 0});
    }
    res.push_back({pts.back().first, pts.back().second, 0});
    for (auto &p : res) p.y = t.rawHeightAt(p.x, p.z);
    // smooth the road profile so grades are gentle (cuts and fills of a few metres)
    for (int pass = 0; pass < 30; ++pass)
        for (size_t i = 1; i + 1 < res.size(); ++i) res[i].y = 0.25f * res[i - 1].y + 0.5f * res[i].y + 0.25f * res[i + 1].y;
    out = std::move(res);
    return true;
}

void RoadNetwork::build(const Terrain &t, int count)
{
    outposts_.clear();
    roads_.clear();
    half_ = t.config().worldSize * 0.5f;
    std::mt19937_64 gen(t.config().seed ^ 0x0A57ull);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    const float wl = t.config().waterLevel;
    for (int k = 0; k < count; ++k)
    {
        float baseAng = (k + 0.3f * U(gen)) * 6.2831853f / count;
        float bestScore = 1e30f;
        Outpost best;
        bool found = false;
        for (int tries = 0; tries < 60; ++tries)
        {
            float r = 160.0f + 320.0f * U(gen);
            float a = baseAng + (U(gen) - 0.5f) * 0.7f;
            float x = std::cos(a) * r, z = std::sin(a) * r;
            if (std::fabs(x) > half_ - 80 || std::fabs(z) > half_ - 80) continue;
            float hc = t.rawHeightAt(x, z);
            if (hc < wl + 6.0f) continue;
            float rough = 0;
            bool wet = false;
            for (int s = 0; s < 12 && !wet; ++s)
            {
                float aa = s * 0.523599f;
                for (float rr : {30.0f, 48.0f})
                {
                    float hh = t.rawHeightAt(x + rr * std::cos(aa), z + rr * std::sin(aa));
                    if (hh < wl + 2.0f) wet = true; // keep lakes well away from the pad
                    rough = std::max(rough, std::fabs(hh - hc));
                }
            }
            if (wet) continue;
            bool tooClose = false;
            for (auto &o : outposts_) if (std::hypot(o.x - x, o.z - z) < 170) tooClose = true;
            if (tooClose) continue;
            float score = rough + 0.05f * r;
            if (score < bestScore) { bestScore = score; best.x = x; best.z = z; best.padHeight = hc; best.yaw = std::atan2(-z, -x); found = true; }
        }
        if (found) outposts_.push_back(best);
    }
    // outpost pad heights follow the surrounding average so cuts and fills stay small
    for (auto &o : outposts_)
    {
        float sum = t.rawHeightAt(o.x, o.z);
        for (int s = 0; s < 8; ++s)
            sum += t.rawHeightAt(o.x + 18 * std::cos(s * 0.785398f), o.z + 18 * std::sin(s * 0.785398f));
        o.padHeight = sum / 9.0f;
    }
    // hub (spawn) to each outpost, plus links between neighbours around the ring
    std::vector<std::pair<int, int>> links;
    for (size_t i = 0; i < outposts_.size(); ++i) links.push_back({-1, static_cast<int>(i)});
    for (size_t i = 0; i + 1 < outposts_.size(); ++i) links.push_back({static_cast<int>(i), static_cast<int>(i + 1)});
    for (auto [a, b] : links)
    {
        float ax = a < 0 ? 0.0f : outposts_[a].x, az = a < 0 ? 0.0f : outposts_[a].z;
        std::vector<RoadPoint> path;
        if (findPath(t, ax, az, outposts_[b].x, outposts_[b].z, path)) roads_.push_back(std::move(path));
    }
    indexSegments();
}

void RoadNetwork::indexSegments()
{
    segs_.clear();
    gridN_ = static_cast<int>(std::ceil(2 * half_ / kCell)) + 1;
    grid_.assign(static_cast<size_t>(gridN_) * gridN_, {});
    for (auto &r : roads_)
        for (size_t i = 0; i + 1 < r.size(); ++i)
        {
            int id = static_cast<int>(segs_.size());
            segs_.push_back({r[i], r[i + 1]});
            float minx = std::min(r[i].x, r[i + 1].x), maxx = std::max(r[i].x, r[i + 1].x);
            float minz = std::min(r[i].z, r[i + 1].z), maxz = std::max(r[i].z, r[i + 1].z);
            auto ci = [&](float v) { return std::clamp(static_cast<int>((v + half_) / kCell), 0, gridN_ - 1); };
            for (int cj = ci(minz); cj <= ci(maxz); ++cj)
                for (int cx = ci(minx); cx <= ci(maxx); ++cx) grid_[cj * gridN_ + cx].push_back(id);
        }
}

bool RoadNetwork::nearestRoad(float x, float z, float range, float &dist, float &h) const
{
    if (segs_.empty()) return false;
    int cx0 = static_cast<int>((x - range + half_) / kCell), cx1 = static_cast<int>((x + range + half_) / kCell);
    int cz0 = static_cast<int>((z - range + half_) / kCell), cz1 = static_cast<int>((z + range + half_) / kCell);
    cx0 = std::max(cx0, 0); cz0 = std::max(cz0, 0); cx1 = std::min(cx1, gridN_ - 1); cz1 = std::min(cz1, gridN_ - 1);
    float best = 1e30f, bh = 0;
    for (int j = cz0; j <= cz1; ++j)
        for (int i = cx0; i <= cx1; ++i)
            for (int id : grid_[j * gridN_ + i])
            {
                const Seg &s = segs_[id];
                float dx = s.b.x - s.a.x, dz = s.b.z - s.a.z;
                float l2 = dx * dx + dz * dz;
                float u = l2 > 0 ? ((x - s.a.x) * dx + (z - s.a.z) * dz) / l2 : 0;
                u = u < 0 ? 0 : (u > 1 ? 1 : u);
                float px = s.a.x + dx * u, pz = s.a.z + dz * u;
                float d = std::hypot(x - px, z - pz);
                if (d < best) { best = d; bh = s.a.y + (s.b.y - s.a.y) * u; }
            }
    if (best > range) return false;
    dist = best;
    h = bh;
    return true;
}

float RoadNetwork::carve(float x, float z, float &outHeight) const
{
    float wBest = 0, hBest = 0;
    float d, h;
    if (nearestRoad(x, z, kHalfWidth + kShoulder, d, h))
    {
        float w = 1.0f - sstep(kHalfWidth, kHalfWidth + kShoulder, d);
        if (w > wBest) { wBest = w; hBest = h; }
    }
    for (const auto &o : outposts_)
    {
        float dd = std::hypot(x - o.x, z - o.z);
        if (dd > o.padRadius + kPadBlend) continue;
        float w = 1.0f - sstep(o.padRadius, o.padRadius + kPadBlend, dd);
        if (w > wBest) { wBest = w; hBest = o.padHeight; }
    }
    outHeight = hBest;
    return wBest;
}

float RoadNetwork::surfaceMask(float x, float z) const
{
    float m = 0, d, h;
    if (nearestRoad(x, z, kHalfWidth + 2.0f, d, h)) m = 1.0f - sstep(kHalfWidth - 0.5f, kHalfWidth + 2.0f, d);
    for (const auto &o : outposts_)
    {
        float dd = std::hypot(x - o.x, z - o.z);
        if (dd < o.padRadius + 4.0f) m = std::max(m, 1.0f - sstep(o.padRadius - 2.0f, o.padRadius + 4.0f, dd));
    }
    return m;
}

bool RoadNetwork::near(float x, float z, float margin) const
{
    float d, h;
    if (nearestRoad(x, z, kHalfWidth + margin, d, h)) return true;
    for (const auto &o : outposts_)
        if (std::hypot(x - o.x, z - o.z) < o.padRadius + margin) return true;
    return false;
}
