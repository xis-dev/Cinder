#version 330 core 

in vec2 v_UV;

uniform sampler2D u_iconImage;

out vec4 FragColor;

void main()	 
{

	FragColor = texture(u_iconImage, v_UV);;
}