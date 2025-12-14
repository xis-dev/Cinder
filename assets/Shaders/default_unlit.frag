#version 330 core 
	
struct Material {
	vec3 albedo;
};

uniform Material u_Material;

out vec4 FragColor;


void main() {

	FragColor = vec4(u_Material.albedo, 1.0);
}