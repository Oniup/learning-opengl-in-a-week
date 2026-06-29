#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Color;
layout(location = 3) in vec2 a_TexCoords;

out vec3 Color;
out vec3 Normal;
out vec2 TexCoords;
out vec3 FragPosition;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

uniform vec2 u_Material_TilingFactor;

void main()
{
    vec4 position = vec4(a_Position, 1.0);
    gl_Position   = u_Projection * u_View * u_Model * position;

    Color        = a_Color;
    Normal       = mat3(transpose(inverse(u_Model))) * a_Normal;
    TexCoords    = a_TexCoords * u_Material_TilingFactor;
    FragPosition = vec3(u_Model * position);
}
