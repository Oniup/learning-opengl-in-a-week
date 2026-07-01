#version 460 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_SkyBox;

void main()
{
    FragColor = texture(u_SkyBox, TexCoords);
}
