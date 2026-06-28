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

uniform float u_Near;
uniform float u_Far;
uniform float u_FogDensity;
uniform vec3  u_FogColor;
uniform bool  u_ViewDepthBuffer;
uniform bool  u_SquareFog;

float LinearizeDepth(float depth)
{
    float ndc = depth * 2.0 - 1.0;
    // Equation derived from the projection matrix as we convert the normalized device coords to a
    // linear depth
    float linear_depth = (2.0 * u_Near * u_Far) / (u_Far + u_Near - ndc * (u_Far - u_Near));
    return linear_depth;
}

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

vec4 GetObjectColor(vec3 normal)
{
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
    vec4 emission_texture = texture(u_Material.Emission.Image, TexCoords);

    float emission_pulse = (sin(u_Time) * 0.25 + 0.75);

    vec3 diffuse  = diffuse_light * diffuse_texture.rgb * u_Material.Diffuse.Color;
    vec3 specular = specular_light * specular_texture.rgb * u_Material.Specular.Color;
    vec3 emission = emission_texture.rgb * emission_pulse * u_Material.Emission.Color;
    vec3 ambient  = ambient_light * diffuse_texture.rgb;

    return vec4(diffuse + specular + emission + ambient, diffuse_texture.a);
}

void main()
{
    if (u_ViewDepthBuffer)
    {
        float depth = LinearizeDepth(gl_FragCoord.z) / u_Far;
        FragColor   = vec4(vec3(depth), 1.0);
        return;
    }

    vec3 normal = normalize(Normal);
    vec4 color  = GetObjectColor(normal);

    float linear_depth = LinearizeDepth(gl_FragCoord.z);
    float fog_factor;
    if (u_SquareFog)
        fog_factor = exp(-pow(u_FogDensity * linear_depth, 2.0));
    else
        fog_factor = exp(-u_FogDensity * linear_depth);
    fog_factor = clamp(fog_factor, 0.0, 1.0);

    FragColor = vec4(mix(u_FogColor, color.rgb, fog_factor), color.a);
}
