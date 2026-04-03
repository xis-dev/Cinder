#version 330 core 

in vec2 v_UV;

uniform sampler2D u_iconImage;
uniform float u_Gamma;
out vec4 FragColor;

void main()	 
{

	FragColor = texture(u_iconImage, v_UV);
	FragColor.rgb = pow(FragColor.rgb, vec3(1 / u_Gamma));
}