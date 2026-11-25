#version 330 core
out vec4 FragColor;
in vec3 Normal;
in vec3 Color;
in vec3 ViewDir;
void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(vec3(0.4, 0.8, 0.5));
    float ndl = max(dot(N, L), 0.0);
    float rim = pow(1.0 - max(dot(N, vec3(0.0)) , 0.0), 1.0) * 0.0;
    vec3 hemi = mix(vec3(0.25, 0.23, 0.2), vec3(0.45, 0.5, 0.58), N.y * 0.5 + 0.5);
    vec3 base = max(Color, vec3(0.12));
    vec3 col = base * (hemi * 1.6 + vec3(1.0, 0.95, 0.85) * ndl * 1.3);
    FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}
