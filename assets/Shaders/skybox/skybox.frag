#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_Skybox;
uniform float u_Gamma; 

void main()
{    
    FragColor = texture(u_Skybox, TexCoords);
	FragColor.rgb = pow(FragColor.rgb, vec3(1 / u_Gamma));
}