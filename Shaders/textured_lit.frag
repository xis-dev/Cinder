#version 330 core


struct Material {
	vec3 albedo;
	float ambient;
	float diffuse;
	float specular;
	float shininess;
};


struct DirectionalLight{
	float intensity;
	vec3 direction;
	vec3 color;
};

struct PointLight{
	float intensity;
	vec3 position;
	vec3 color;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight {
	float intensity;
	vec3 position;
	vec3 direction;
	vec3 color;

	float innerCutoff;
	float outerCutoff;
};


in vec2 v_UV;
in vec3 v_WorldNormal;
in vec3 v_WorldPos;

uniform vec3 u_CameraPosition;
uniform vec3 u_ViewDirection;

uniform Material u_Material;

out vec4 FragColor;



float near = 0.1;
float far = 10000.0;

#define MAX_POINT_LIGHTS 5
#define MAX_SPOT_LIGHTS 1
#define MAX_DIR_LIGHTS 1

#define MAX_MAPS_SPECULAR 3
#define MAX_MAPS_DIFFUSE 3

uniform int u_DirLightCount;
uniform int u_PointLightCount;
uniform int u_SpotLightCount;

uniform int u_DiffuseMapCount;
uniform int u_SpecularMapCount;



uniform sampler2D t_Specular[MAX_MAPS_SPECULAR];
uniform sampler2D t_Diffuse[MAX_MAPS_DIFFUSE];


uniform DirectionalLight u_DirectionalLights[MAX_DIR_LIGHTS];
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight u_SpotLights[MAX_SPOT_LIGHTS];

uniform bool u_cullBackface;


vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex);


float linearizeDepth(float depth);

void main() {

	int numberOfDirLights = clamp(u_DirLightCount, 0, MAX_DIR_LIGHTS);
	int numberOfPointLights = clamp(u_PointLightCount, 0, MAX_POINT_LIGHTS);
	int numberOfSpotLights = clamp(u_SpotLightCount, 0, MAX_SPOT_LIGHTS);

	int numberOfDiffuseMaps = clamp(u_DiffuseMapCount, 0, MAX_MAPS_DIFFUSE);
	int numberOfSpecularMaps = clamp(u_SpecularMapCount, 0, MAX_MAPS_SPECULAR);

	vec3 norm = normalize(v_WorldNormal);
	vec3 viewDir = normalize(u_CameraPosition - v_WorldPos);

	if (u_cullBackface && dot(norm, viewDir) >= 0.0) {
		discard;
	}

	vec3 diffuseTex = vec3(0.0);
	for (int i = 0; i < numberOfDiffuseMaps; i++) {
		diffuseTex += vec3(texture(t_Diffuse[i], v_UV));
	}

	vec3 specularTex = vec3(0.0);
	for (int i = 0; i < numberOfSpecularMaps; i++) {
		specularTex += vec3(texture(t_Specular[i], v_UV));
	}

	vec3 result = vec3(0,0,0);

	 for (int i = 0; i < numberOfDirLights; i++) {
		result += calcDirLight(u_DirectionalLights[i], norm, viewDir, diffuseTex, specularTex);
	}
	for (int i = 0; i < numberOfPointLights; i++) {
		result += calcPointLight(u_PointLights[i], norm, viewDir, diffuseTex, specularTex);
	}

	vec3 depth = vec3(linearizeDepth(gl_FragCoord.z)/far);
	FragColor = vec4(result * u_Material.albedo, 1.0) ;
	return;
}

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex) {

	float diff = max(dot(normalize(-light.direction), normal), 0.0) * u_Material.diffuse;
	vec3 diffuse = diffuseTex * diff * light.color;

	vec3 ambient = u_Material.ambient * light.color * diffuseTex;

	vec3 reflectedDir = normalize(reflect(normalize(light.direction), normal));
	float spec = pow(max(dot(viewDir, reflectedDir), 0.0), u_Material.shininess);
	vec3 specular = specularTex * spec * u_Material.specular * light.color;

	return (ambient + diffuse + specular ) * light.intensity;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex) {

	vec3 lightDir = normalize(light.position - v_WorldPos);

	float diff = max(dot(normal, lightDir), 0.0);
	
	vec3 reflectDir = normalize(reflect(-lightDir, normal));
	vec3 fragToViewDirection = normalize(u_CameraPosition - v_WorldPos);

	float dist = length(light.position - v_WorldPos);

	float spec = pow(max(dot(fragToViewDirection, reflectDir), 0.0), u_Material.shininess);
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
						
	vec3 diffuse  = u_Material.diffuse  * diff * diffuseTex * light.color;
	vec3 specular = u_Material.specular * spec * specularTex * light.color;

	diffuse  *= attenuation;
	specular *= attenuation;

	vec3 result = (diffuse + specular ) * light.intensity;
	return result;

}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex) {



	vec3 lightDir = normalize(light.position - v_WorldPos);
	float diff = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diffuseTex * u_Material.diffuse * diff * light.color;

	vec3 reflectedDir = normalize(reflect(-lightDir, normal));
	float spec = pow(max(dot(reflectedDir, viewDir), 0.0), u_Material.shininess);
	vec3 specular = spec * u_Material.specular * specularTex * light.color;

	float theta = max(dot(normalize(light.direction), -lightDir), 0.0);
	float epsilon = light.innerCutoff - light.outerCutoff;
	float attenuation = clamp(((theta - light.outerCutoff)/epsilon), 0.0, 1.0);

	diffuse *= attenuation;
	specular *= attenuation;

	return   (diffuse + specular ) * light.intensity;
}

float linearizeDepth(float depth) 
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

