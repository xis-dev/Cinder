#version 330 core

#define MAX_MAPS_DIFFUSE 16
#define MAX_MAPS_SPECULAR 16

struct Material {
	vec3 albedo;
};


in vec2 v_UV;
in vec3 v_WorldPos;

uniform Material u_Material;

uniform int u_DiffuseMapCount;
uniform int u_SpecularMapCount;

uniform sampler2D diffuseMaps[MAX_MAPS_DIFFUSE];
uniform sampler2D specularMaps[MAX_MAPS_SPECULAR];

out vec4 FragColor;
void main()	{
	

	int numberOfDiffuseMaps = clamp(u_DiffuseMapCount, 0, MAX_MAPS_DIFFUSE);
	int numberOfSpecularMaps = clamp(u_SpecularMapCount, 0, MAX_MAPS_SPECULAR);

	vec4 result = vec4(vec3(0.0), 1.0);
	for (int i = 0; i < numberOfDiffuseMaps; i++)  {  
		 result.xyz += u_Material.albedo * texture(diffuseMaps[i], v_UV).xyz;
		 result.w *= texture(diffuseMaps[i], v_UV).w;
	}

	for (int i = 0; i < numberOfSpecularMaps; i++)  {  
		 result.xyz += u_Material.albedo * texture(specularMaps[i], v_UV).xyz;
		 result.w *= texture(specularMaps[i], v_UV).w;
	}

	FragColor = result;
}