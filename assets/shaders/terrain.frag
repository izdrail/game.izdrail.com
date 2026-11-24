#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;
in float Road;

uniform sampler2D grassC, grassN, grassR;
uniform sampler2D dirtC, dirtN, dirtR;
uniform sampler2D rockC, rockN, rockR;
uniform sampler2D snowC, snowN, snowR;

uniform vec3 cameraPos;
uniform vec3 fogColor;
uniform float fogDensity;
uniform float snowLine;
uniform float dryness;
uniform float waterLevel;
uniform float exposure;
uniform vec3 sunColor;
uniform vec3 skyAmbient;
uniform vec3 groundAmbient;

// cheap smooth noise from a texture's low frequency content (breaks up material boundaries)
float macro(vec2 p) { return texture(dirtR, p * 0.011).r * 0.6 + texture(grassR, p * 0.0037 + 0.3).r * 0.4; }

// planar sample with two scales blended (hides tiling)
vec3 planarC(sampler2D t, vec2 p, float s) { return mix(texture(t, p * s).rgb, texture(t, p * s * 0.23 + 0.37).rgb, 0.4); }
vec3 planarN(sampler2D t, vec2 p, float s) { return mix(texture(t, p * s).rgb, texture(t, p * s * 0.23 + 0.37).rgb, 0.4) * 2.0 - 1.0; }
float planarR(sampler2D t, vec2 p, float s) { return mix(texture(t, p * s).r, texture(t, p * s * 0.23 + 0.37).r, 0.4); }

void main()
{
    vec3 N = normalize(Normal);
    float dist = length(WorldPos - cameraPos);
    vec3 b = pow(abs(N), vec3(4.0)); b /= (b.x + b.y + b.z);   // triplanar weights
    vec2 pxz = WorldPos.xz, pxy = WorldPos.xy, pzy = WorldPos.zy;

    // --- material weights: slope, height, wetness, macro noise
    float m = macro(pxz);
    float slope = 1.0 - N.y;
    float wRock = smoothstep(0.20 + 0.05 * (m - 0.5), 0.36, slope);
    float wSnow = smoothstep(snowLine - 6.0, snowLine + 10.0, WorldPos.y + (m - 0.5) * 22.0) * (1.0 - smoothstep(0.30, 0.5, slope));
    float wet = 1.0 - smoothstep(waterLevel + 0.4, waterLevel + 3.5, WorldPos.y);
    float wDirt = clamp(smoothstep(0.52, 0.72, m) * 0.85 + wet * 1.2 + dryness, 0.0, 1.0) * (1.0 - wRock);
    float wGrass = (1.0 - wRock) * (1.0 - wDirt) * (1.0 - wSnow);
    wSnow *= (1.0 - wRock * 0.7);
    float sum = wRock + wDirt + wGrass + wSnow + 1e-4;
    wRock /= sum; wDirt /= sum; wGrass /= sum; wSnow /= sum;

    // --- albedo / roughness / detail normals per material
    vec3 cG = planarC(grassC, pxz, 0.35), cD = planarC(dirtC, pxz, 0.30), cS = planarC(snowC, pxz, 0.25);
    // rock uses triplanar so cliffs are not stretched
    vec3 cR = texture(rockC, pzy * 0.16).rgb * b.x + texture(rockC, pxz * 0.16).rgb * b.y + texture(rockC, pxy * 0.16).rgb * b.z;
    vec3 nR = (texture(rockN, pzy * 0.16).rgb * b.x + texture(rockN, pxz * 0.16).rgb * b.y + texture(rockN, pxy * 0.16).rgb * b.z) * 2.0 - 1.0;
    float rR = texture(rockR, pxz * 0.16).r;

    vec3 nG = planarN(grassN, pxz, 0.35), nD = planarN(dirtN, pxz, 0.30), nS = planarN(snowN, pxz, 0.25);
    vec3 albedo = cG * wGrass + cD * wDirt + cR * wRock + cS * wSnow;
    // roads and outpost pads: packed gravel with wheel-rut streaks along the surface
    float road = smoothstep(0.35, 0.85, Road);
    vec3 gravel = cD * vec3(0.78, 0.74, 0.68) * (0.8 + 0.4 * macro(pxz * 3.0));
    albedo = mix(albedo, gravel, road);
    // dry look: bleach grass towards sand; wet ground is darker
    albedo = mix(albedo, albedo * vec3(1.25, 1.1, 0.8), dryness * 0.5);
    albedo *= mix(1.0, 0.55, wet);
    float rough = planarR(grassR, pxz, 0.35) * wGrass + planarR(dirtR, pxz, 0.30) * wDirt + rR * wRock + planarR(snowR, pxz, 0.25) * wSnow;
    rough = mix(rough, 0.25, wet * 0.6);
    rough = mix(rough, 0.9, road);

    // detail normal perturbation (UDN blend), fades with distance
    vec2 d = (nG.xy * wGrass + nD.xy * wDirt + nS.xy * wSnow + nR.xy * wRock) * (1.0 - smoothstep(60.0, 260.0, dist));
    // build tangent frame for the planar case
    vec3 T = normalize(cross(N, vec3(0.0, 0.0, 1.0)) + 1e-5);
    vec3 B = cross(N, T);
    vec3 Nd = normalize(N + (T * d.x + B * d.y) * 0.9);

    // --- lighting (linear space)
    vec3 L = normalize(vec3(0.45, 0.8, 0.3));
    vec3 V = normalize(cameraPos - WorldPos);
    vec3 H = normalize(L + V);
    float ndl = max(dot(Nd, L), 0.0);
    float wrap = max((dot(Nd, L) + 0.3) / 1.3, 0.0);
    vec3 hemi = mix(groundAmbient, skyAmbient, Nd.y * 0.5 + 0.5);
    float shin = mix(180.0, 6.0, clamp(rough, 0.0, 1.0));
    float spec = pow(max(dot(Nd, H), 0.0), shin) * (1.0 - rough) * 0.6 * ndl;
    // cheap ambient occlusion from slope and low ground
    float ao = mix(1.0, 0.72, smoothstep(0.25, 0.6, slope)) * (0.85 + 0.15 * smoothstep(waterLevel, waterLevel + 8.0, WorldPos.y));
    vec3 col = albedo * (hemi * ao + sunColor * wrap * 1.15) + sunColor * spec;

    // --- aerial perspective: fog toward horizon colour, thicker in low ground
    float hf = exp(-max(WorldPos.y - waterLevel, 0.0) * 0.012);
    float fog = 1.0 - exp(-pow(dist * fogDensity, 1.5) * (0.8 + 0.6 * hf));
    col = mix(col, fogColor, clamp(fog, 0.0, 1.0));

    // tonemap (ACES fit) + gamma
    col *= exposure;
    col = (col * (2.51 * col + 0.03)) / (col * (2.43 * col + 0.59) + 0.14);
    FragColor = vec4(pow(clamp(col, 0.0, 1.0), vec3(1.0 / 2.2)), 1.0);
}
