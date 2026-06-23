#version 460 core

out vec4 FragColor;

in vec3 Color;
in vec2 TexCoords;

uniform int u_TextureCount;
uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;

struct Light
{
    vec3 Position;
    vec3 Color;
};

uniform int u_LightCount;
uniform Light u_Lights[10];

void main()
{
    vec4 texture_color = texture(u_Texture0, TexCoords);

    if (u_TextureCount > 1)
    {
        vec4 fg_texture = texture(u_Texture1, TexCoords);
        texture_color = mix(texture_color, fg_texture, fg_texture.a);
    }

    vec3 light_color = vec3(0.0f);
    for (int i = 0; i < u_LightCount; i++)
    {
        light_color += u_Lights[i].Color;
    }

    FragColor = texture_color * vec4(light_color.rgb, 1.0);
}
