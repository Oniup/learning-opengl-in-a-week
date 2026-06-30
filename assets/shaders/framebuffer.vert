#version 460 core

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(a_Position.xy, 0.0, 1.0);
    TexCoords   = a_TexCoord;
}
