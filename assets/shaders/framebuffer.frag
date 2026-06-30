#version 460 core

out vec4 FragColor;
in vec2  TexCoords;

uniform sampler2D u_ColorAttachment;

void main()
{
    FragColor = texture(u_ColorAttachment, TexCoords);
}
