#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 projection;
uniform mat4 view;
uniform float level;
out vec3 WorldPos;
void main()
{
    WorldPos = vec3(aPos.x, level, aPos.z);
    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
