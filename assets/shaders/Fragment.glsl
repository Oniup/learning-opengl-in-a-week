#version 460 core

out vec4 FragColor;

in vec3 Color;
in vec2 TexCoords;

uniform int u_TextureCount;
uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;

void main()
{
    vec4 bg_texture = texture(u_Texture0, TexCoords);
    vec4 fg_texture = texture(u_Texture1, TexCoords);

    FragColor = bg_texture * (1 - fg_texture.a) + fg_texture * fg_texture.a;

    FragColor = texture(u_Texture0, TexCoords);
}
