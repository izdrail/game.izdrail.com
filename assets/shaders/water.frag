#version 330 core
out vec4 FragColor;
in vec3 WorldPos;
uniform vec3 cameraPos;
uniform vec3 fogColor;
uniform float fogDensity;
uniform float time;
uniform vec3 sunColor;
void main()
{
    // animated ripple normal
    vec2 p = WorldPos.xz;
    float a = sin(p.x * 0.35 + time * 1.3) + sin(p.y * 0.41 - time * 1.1) + sin((p.x + p.y) * 0.17 + time * 0.7);
    float b = cos(p.x * 0.29 - time * 0.9) + cos(p.y * 0.33 + time * 1.4);
    vec3 n = normalize(vec3(a * 0.05, 1.0, b * 0.05));
    vec3 v = normalize(cameraPos - WorldPos);
    float fres = pow(1.0 - max(dot(n, v), 0.0), 3.0);
    vec3 deep = vec3(0.04, 0.17, 0.26);
    vec3 sky = vec3(0.55, 0.68, 0.80);
    vec3 col = mix(deep, sky, 0.25 + 0.65 * fres);
    vec3 sun = normalize(vec3(0.45, 0.8, 0.3));
    col += sunColor * pow(max(dot(reflect(-sun, n), v), 0.0), 80.0) * 0.8;
    float dist = length(WorldPos - cameraPos);
    float fog = 1.0 - exp(-pow(dist * fogDensity, 1.6));
    FragColor = vec4(mix(col, fogColor, clamp(fog, 0.0, 1.0)), 0.78 + 0.2 * fres);
}
