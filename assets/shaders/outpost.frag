#version 330 core
out vec4 FragColor;
in vec3 WorldPos;
in vec3 Normal;
in vec3 Color;
uniform vec3 cameraPos, fogColor, sunColor, skyAmbient, groundAmbient;
uniform float fogDensity, exposure;
void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(vec3(0.45, 0.8, 0.3));
    float dist = length(WorldPos - cameraPos);
    vec3 hemi = mix(groundAmbient, skyAmbient, N.y * 0.5 + 0.5);
    float wrap = max((dot(N, L) + 0.3) / 1.3, 0.0);
    // subtle grime: darker toward the ground
    float grime = mix(0.75, 1.0, smoothstep(0.0, 2.5, fract(WorldPos.y * 0.37) + 0.5));
    vec3 col = Color * grime * (hemi + sunColor * wrap * 1.15);
    float fog = 1.0 - exp(-pow(dist * fogDensity, 1.5));
    col = mix(col, fogColor, clamp(fog, 0.0, 1.0));
    col *= exposure;
    col = (col * (2.51 * col + 0.03)) / (col * (2.43 * col + 0.59) + 0.14);
    FragColor = vec4(pow(clamp(col, 0.0, 1.0), vec3(1.0 / 2.2)), 1.0);
}
