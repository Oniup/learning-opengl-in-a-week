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

struct Material
{
    vec3      Tint;
    sampler2D Diffuse;
    sampler2D Specular;
    float     Shininess;
};

uniform vec3  u_CameraPosition;
uniform int   u_LightCount;
uniform Light u_Lights[10];
uniform vec3  u_AmbientColor;

uniform Material u_Material;

out vec4 FragColor;

in vec3 Color;
in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPosition;

vec3 CalculateDiffuseLight(Light light, vec3 light_dir, vec3 normal)
{
    float diffuse = max(dot(normal, light_dir), 0.0f);
    return diffuse * light.Color;
}

vec3 CalculateSpecularLight(Light light, vec3 light_dir, vec3 normal)
{
    vec3  camera_dir  = normalize(u_CameraPosition - FragPosition);
    vec3  reflect_dir = reflect(-light_dir, normal);
    float specular    = pow(max(dot(camera_dir, reflect_dir), 0.0f), 32);
    return light.SpecularStrength * specular * light.Specular;
}

void main()
{
    vec3 normal = normalize(Normal);

    vec3 diffuse  = vec3(0.0f);
    vec3 specular = vec3(0.0f);
    for (int i = 0; i < u_LightCount; i++)
    {
        vec3 light_dir = normalize(u_Lights[i].Position - FragPosition);

        diffuse += CalculateDiffuseLight(u_Lights[i], light_dir, normal) * u_Lights[i].Intensity;
        specular += CalculateSpecularLight(u_Lights[i], light_dir, normal) * u_Lights[i].Intensity;
    }

    vec4 diffuse_texture  = texture(u_Material.Diffuse, TexCoords);
    vec4 specular_texture = texture(u_Material.Specular, TexCoords);

    vec3 final_diffuse  = diffuse * diffuse_texture.rgb;
    vec3 final_specular = specular * specular_texture.rgb;
    vec3 final_ambient  = u_AmbientColor * diffuse_texture.rgb;

    FragColor = vec4(final_diffuse + final_specular + final_ambient, diffuse_texture.a);
}
