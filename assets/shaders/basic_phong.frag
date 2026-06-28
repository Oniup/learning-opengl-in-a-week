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

    float Constant;
    float Linear;
    float Quadratic;
    float SpotCutOff;
    float SpotOuterCutOff;

    vec3 Color;
    vec3 Ambient;
    vec3 Specular;
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
uniform vec3  u_GlobalAmbientLight;

// Other
uniform Material u_Material;
uniform float    u_Time;

// From Vertex
in vec3 Color;
in vec3 Normal;
in vec2 TexCoords;
in vec3 FragPosition;

vec3 CalculateDiffuseLight(Light light, vec3 light_direction, vec3 normal)
{
    float diffuse = max(dot(normal, light_direction), 0.0f);
    return diffuse * light.Color;
}

vec3 CalculateSpecularLight(Light light, vec3 light_direction, vec3 normal)
{
    vec3  camera_direction  = normalize(u_CameraPosition - FragPosition);
    vec3  reflect_direction = reflect(-light_direction, normal);
    float specular          = pow(max(dot(camera_direction, reflect_direction), 0.0f), 32);
    return specular * light.Specular;
}

float CalculateAttenuation(Light light)
{
    float distance = length(light.Position - FragPosition);
    float denominator =
        (light.Quadratic * distance * distance) + (light.Linear * distance) + light.Constant;
    return 1.0 / denominator;
}

void main()
{
    vec3 normal = normalize(Normal);

    vec3 diffuse_light  = vec3(0.0f);
    vec3 specular_light = vec3(0.0f);
    vec3 ambient_light  = u_GlobalAmbientLight;
    for (int i = 0; i < u_LightCount; i++)
    {
        Light light = u_Lights[i];

        vec3  direction   = vec3(0.0f);
        float attenuation = 0.0f;

        switch (light.Type)
        {
        case LIGHT_TYPE_POINT: {
            direction   = normalize(light.Position - FragPosition);
            attenuation = light.Intensity * CalculateAttenuation(light);
        }
        break;
        case LIGHT_TYPE_SPOT: {
            direction = normalize(light.Position - FragPosition);

            float theta     = dot(direction, normalize(-light.Direction));
            float epsilon   = light.SpotCutOff - light.SpotOuterCutOff;
            float intensity = clamp((theta - light.SpotOuterCutOff) / epsilon, 0.0, 1.0);

            attenuation = light.Intensity * intensity * CalculateAttenuation(light);
        }
        break;
        case LIGHT_TYPE_DIRECTIONAL: {
            direction   = normalize(-light.Direction);
            attenuation = light.Intensity;
        }
        break;
        }

        diffuse_light  += CalculateDiffuseLight(light, direction, normal) * attenuation;
        specular_light += CalculateSpecularLight(light, direction, normal) * attenuation;
        ambient_light  += light.Ambient * attenuation;
    }

    vec4 diffuse_texture  = texture(u_Material.Diffuse.Image, TexCoords);
    vec4 specular_texture = texture(u_Material.Specular.Image, TexCoords);
    vec4 emission_texture = texture(u_Material.Emission.Image, TexCoords + vec2(0.0, 0.3 * u_Time));

    float emission_pulse = (sin(u_Time) * 0.5 + 0.5);

    vec3 diffuse  = diffuse_light * diffuse_texture.rgb * u_Material.Diffuse.Color;
    vec3 specular = specular_light * specular_texture.rgb * u_Material.Specular.Color;
    vec3 emission = emission_texture.rgb * u_Material.Emission.Color * emission_pulse;
    vec3 ambient  = ambient_light * diffuse_texture.rgb;

    if (specular_texture.rgb != vec3(0.0))
        emission = vec3(0.0);

    FragColor = vec4(diffuse + specular + ambient + emission, diffuse_texture.a);
}
