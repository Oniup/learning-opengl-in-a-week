#version 460 core

#define LIGHT_TYPE_POINT 0
#define LIGHT_TYPE_DIRECTIONAL 1
#define LIGHT_TYPE_SPOT 2

struct Light
{
    int   Type;
    vec3  Position;
    float Intensity;

    vec3  Color;
    vec3  Specular;
    float SpecularStrength;
    int   SpecularShininess;
};

// struct Material
// {
//     vec3      Ambient;
//     sampler2D Diffuse;
//     sampler2D Specular;
//     float     Shininess;
// };

out vec4 FragColor;

in vec3 Color;
in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPosition;

uniform int       u_TextureCount;
uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;

uniform vec3  u_CameraPosition;
// uniform Material u_Material;
uniform int   u_LightCount;
uniform Light u_Lights[10];
uniform vec3  u_AmbientColor;

vec3 CalculateDiffuseLight(Light light, vec3 light_dir, vec3 normal)
{
    float diffuse = max(dot(normal, light_dir), 0.0f);
    return diffuse * light.Color;
}

vec3 CalculateSpecularLight(Light light, vec3 light_dir, vec3 normal)
{
    vec3  camera_dir  = normalize(u_CameraPosition - FragPosition);
    vec3  reflect_dir = reflect(-light_dir, normal);
    float specular    = pow(max(dot(camera_dir, reflect_dir), 0.0f), light.SpecularShininess);
    return light.SpecularStrength * specular * light.Specular;
}

void main()
{
    vec4 texture_color = texture(u_Texture0, TexCoords);
    vec3 normal        = normalize(Normal);

    if (u_TextureCount > 1)
    {
        vec4 fg_texture = texture(u_Texture1, TexCoords);
        texture_color   = mix(texture_color, fg_texture, fg_texture.a);
    }

    vec3 light_color = vec3(0.0f);
    for (int i = 0; i < u_LightCount; i++)
    {
        vec3 light_dir = normalize(u_Lights[i].Position - FragPosition);

        vec3 diffuse  = CalculateDiffuseLight(u_Lights[i], light_dir, normal);
        vec3 specular = CalculateSpecularLight(u_Lights[i], light_dir, normal);

        light_color += (diffuse + specular) * u_Lights[i].Intensity;
    }

    FragColor = texture_color * vec4(light_color + u_AmbientColor, 1.0);
}
