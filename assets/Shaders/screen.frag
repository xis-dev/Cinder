#version 330 core

out vec4 FragColor;

in vec2 v_UV;

uniform sampler2D t_FinalTexture;

void main()	{
    FragColor = vec4(texture(t_FinalTexture, v_UV).rgb, 1.0);
}
