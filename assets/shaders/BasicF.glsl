#version 460 core

out vec4 FragColor;

in vec3 Color;
in vec2 TexCoords;

uniform int u_TextureCount;
uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;

void main()
{
    vec4 texture0 = texture(u_Texture0, TexCoords);

    if (u_TextureCount > 1)
    {
        vec4 texture1 = texture(u_Texture1, TexCoords);
        FragColor = mix(texture0, texture1, texture1.a);
    }
    else
        FragColor = texture(u_Texture0, TexCoords);
}
