#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;

uniform sampler2D groundTex;
uniform vec3 cameraPos;
uniform vec3 fogColor;
uniform float fogDensity;

void main()
{
    vec3 n = normalize(Normal);
    // base ground texture, tiled; second sample at another scale hides repetition
    vec3 base = texture(groundTex, WorldPos.xz * 0.25).rgb * 0.6 + texture(groundTex, WorldPos.xz * 0.043).rgb * 0.4;
    // slope -> bare rock; height -> lighter, drier tone
    float slope = 1.0 - n.y;
    float rockMix = smoothstep(0.18, 0.38, slope);
    vec3 rock = vec3(0.36, 0.34, 0.31) * (0.75 + 0.5 * texture(groundTex, WorldPos.xz * 0.9).r);
    float high = smoothstep(8.0, 22.0, WorldPos.y);
    vec3 col = mix(base, rock, rockMix);
    col = mix(col, vec3(0.72, 0.72, 0.70), high * (1.0 - rockMix * 0.5) * 0.55);
    vec3 sunDir = normalize(vec3(0.45, 0.8, 0.3));
    float diff = max(dot(n, sunDir), 0.0);
    col *= 0.38 + 0.78 * diff;
    float dist = length(WorldPos - cameraPos);
    float fog = 1.0 - exp(-pow(dist * fogDensity, 1.6));
    FragColor = vec4(mix(col, fogColor, clamp(fog, 0.0, 1.0)), 1.0);
}
