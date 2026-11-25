#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec3 Normal;
out vec3 Color;
out vec3 ViewDir;
void main()
{
    vec4 wp = model * vec4(aPos, 1.0);
    Normal = mat3(model) * aNormal;
    Color = aColor;
    ViewDir = normalize(-(view * wp).xyz);
    gl_Position = projection * view * wp;
}
