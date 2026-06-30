#version 460 core

#define POST_PROCESSING_MODE_DEFAULT 0
#define POST_PROCESSING_MODE_INVERT 1
#define POST_PROCESSING_MODE_AVERAGE_GRAYSCALE 2
#define POST_PROCESSING_MODE_WEIGHTED_GRAYSCALE 3
#define POST_PROCESSING_MODE_KERNEL 4

#define KERNEL_SHARPEN 0
#define KERNEL_BLUR 1
#define KERNEL_EDGE_DETECTION 2

out vec4 FragColor;
in vec2  TexCoords;

uniform sampler2D u_ColorAttachment;

uniform int u_PostProcessingMode;
uniform int u_PostProcessingKernelMode;

vec3 GrayScale(vec3 color)
{
    float average = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    return vec3(average);
}

vec3 ApplyKernelEffects()
{
    const float offset     = 1.0 / 300.0;
    const vec2  offsets[9] = vec2[](vec2(-offset, offset),  // Top-left
                                    vec2(0.0f, offset),     // Top-center
                                    vec2(offset, offset),   // Top-right
                                    vec2(-offset, 0.0f),    // Center-left
                                    vec2(0.0f, 0.0f),       // Center-center
                                    vec2(offset, 0.0f),     // Center-right
                                    vec2(-offset, -offset), // Bottom-left
                                    vec2(0.0f, -offset),    // Bottom-center
                                    vec2(offset, -offset)); // Bottom-right

    float kernel[9];
    switch (u_PostProcessingKernelMode)
    {
    case KERNEL_SHARPEN:
        // clang-format off
        kernel = float[9]( 0.0, -1.0,  0.0,
                          -1.0,  5.0, -1.0,
                           0.0, -1.0,  0.0);
        // clang-format on
        break;
    case KERNEL_BLUR:
        // clang-format off
        kernel = float[9](1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0,
                          2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0,
                          1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0);
        // clang-format on
        break;
    case KERNEL_EDGE_DETECTION:
        // clang-format off
        kernel = float[9](-1.0, -1.0,  -1.0,
                          -1.0,  8.0,  -1.0,
                          -1.0, -1.0,  -1.0);
        // clang-format on
        break;
    }

    vec3 color = vec3(0.0);
    for (int i = 0; i < 9; i++)
    {
        vec3 texture_sample  = vec3(texture(u_ColorAttachment, TexCoords + offsets[i]));
        color               += GrayScale(texture_sample) * kernel[i];
    }
    return color;
}

void main()
{
    vec3 color = texture(u_ColorAttachment, TexCoords).rgb;
    switch (u_PostProcessingMode)
    {
    case POST_PROCESSING_MODE_DEFAULT:           FragColor = vec4(color, 1.0); break;
    case POST_PROCESSING_MODE_INVERT:            FragColor = vec4(1.0 - color, 1.0); break;
    case POST_PROCESSING_MODE_AVERAGE_GRAYSCALE: {
        float average = (color.r + color.g + color.b) / 3.0;
        FragColor     = vec4(average, average, average, 1.0);
    }
    break;
    case POST_PROCESSING_MODE_WEIGHTED_GRAYSCALE: FragColor = vec4(GrayScale(color), 1.0); break;
    case POST_PROCESSING_MODE_KERNEL:             FragColor = vec4(ApplyKernelEffects(), 1.0); break;
    }
}
