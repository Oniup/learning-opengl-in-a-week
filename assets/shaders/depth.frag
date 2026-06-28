#version 460 core

out vec4 FragColor;

float Near = 0.1;
float Far  = 100.0;

float LiNearizeDepth(float depth)
{
    float ndc = depth * 2.0 - 1.0;
    // Equation derived from the projection matrix as we convert the normalized device coords to a
    // liNear depth
    float liNear_depth = (2.0 * Near * Far) / (Far + Near - ndc * (Far - Near));
    return liNear_depth;
}

void main()
{
    float depth = LiNearizeDepth(gl_FragCoord.z) / Far; // Divide by Far for demonstration
    FragColor   = vec4(vec3(depth), 1.0);
}
