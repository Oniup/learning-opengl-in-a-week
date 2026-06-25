#version 460 core

#define LIGHT_TYPE_POINT 0
#define LIGHT_TYPE_SPOT 1
#define LIGHT_TYPE_DIRECTIONAL 2

out vec4 FragColor;

struct Light
{
    int   Type;
    vec3  Position;
    vec3  Direction;
    float Intensity;

    vec3  Color;
    vec3  Specular;
    float SpecularStrength;
    int   SpecularShininess;
};

struct MaterialColorInput
{
    vec3      Color;
    sampler2D Image;
};

struct Material
{
    MaterialColorInput Diffuse;
    MaterialColorInput Specular;
    MaterialColorInput Emission;
    float              Shininess;
};

// Lights
uniform vec3  u_CameraPosition;
uniform int   u_LightCount;
uniform Light u_Lights[10];
uniform vec3  u_AmbientColor;

// Other
uniform Material u_Material;
uniform float    u_Time;

// From Vertex
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

float LengthSq(vec3 vec)
{
    return dot(vec, vec);
}

void main()
{
    vec3 normal = normalize(Normal);

    vec3 diffuse_light  = vec3(0.0f);
    vec3 specular_light = vec3(0.0f);
    for (int i = 0; i < u_LightCount; i++)
    {
        Light light = u_Lights[i];

        vec3 light_dir = vec3(0.0f);
        switch (light.Type)
        {
        case LIGHT_TYPE_POINT:       light_dir = normalize(light.Position - FragPosition); break;
        case LIGHT_TYPE_SPOT:
        case LIGHT_TYPE_DIRECTIONAL: light_dir = normalize(-light.Direction); break;
        }

        diffuse_light += CalculateDiffuseLight(light, light_dir, normal) * light.Intensity;
        specular_light += CalculateSpecularLight(light, light_dir, normal) * light.Intensity;
    }

    vec4 diffuse_texture  = texture(u_Material.Diffuse.Image, TexCoords);
    vec4 specular_texture = texture(u_Material.Specular.Image, TexCoords);
    vec4 emission_texture = texture(u_Material.Emission.Image, TexCoords + vec2(0.0, 0.3 * u_Time));

    float emission_pulse = (sin(u_Time) * 0.5 + 0.5);

    vec3 diffuse  = diffuse_light * diffuse_texture.rgb * u_Material.Diffuse.Color;
    vec3 specular = specular_light * specular_texture.rgb * u_Material.Specular.Color;
    vec3 emission = emission_texture.rgb * u_Material.Emission.Color * emission_pulse;
    vec3 ambient  = u_AmbientColor * diffuse_texture.rgb;

    if (specular_texture.rgb != vec3(0.0))
        emission = vec3(0.0);

    FragColor = vec4(diffuse + specular + ambient + emission, diffuse_texture.a);
}
