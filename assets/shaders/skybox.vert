#version 460 core

layout(location = 0) in vec3 a_Position;

out vec3 TexCoords;

uniform mat4 u_Projection;
uniform mat4 u_View;

void main()
{
    vec4 vertex_position = u_Projection * u_View * vec4(a_Position, 1.0);
    gl_Position          = vertex_position.xyww; // Set the depth to always be 1.0, farthest away
    TexCoords            = a_Position;
}
