#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
layout(location = 2) in vec2 a_TexCoords;

out vec3 Color;
out vec2 TexCoords;

void main()
{
    gl_Position = vec4(a_Position.xyz, 1.0);

    Color = a_Color;
    TexCoords = a_TexCoords;
}
