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




in vec3 v_WorldNormal;
in vec3 v_WorldPos;


#define MAX_POINT_LIGHTS 1
#define MAX_SPOT_LIGHTS 1
#define MAX_DIR_LIGHTS 1

#define MAX_MAPS_SPECULAR 3
#define MAX_MAPS_DIFFUSE 3

uniform int u_DirLightCount;
uniform int u_PointLightCount;
uniform int u_SpotLightCount;



uniform sampler2D t_Specular[MAX_MAPS_SPECULAR];
uniform sampler2D t_Diffuse[MAX_MAPS_DIFFUSE];


uniform DirectionalLight u_DirectionalLights[MAX_DIR_LIGHTS];
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight u_SpotLights[MAX_SPOT_LIGHTS];

uniform bool u_cullBackface;


uniform vec3 u_CameraPosition;


uniform vec3 u_ViewDirection;

Material u_Material = Material(vec3(1.0, 1.0, 1.0), 1.0, 1.0, 0.5, 32);

out vec4 FragColor;


vec3 calcDirLights(DirectionalLight light, Material mat, vec3 normal, vec3 cameraPosition, vec3 fragPosition);
vec3 calcPointLights(PointLight light, Material mat, vec3 normal, vec3 cameraPosition, vec3 fragPosition);
vec3 calcSpotLights(SpotLight light, Material mat, vec3 normal, vec3 cameraPosition, vec3 fragPosition);




void main()
{

	int numberOfDirLights = clamp(u_DirLightCount, 0, MAX_DIR_LIGHTS);
	int numberOfPointLights = clamp(u_PointLightCount, 0, MAX_POINT_LIGHTS);
	int numberOfSpotLights = clamp(u_SpotLightCount, 0, MAX_SPOT_LIGHTS);


	vec3 norm = normalize(v_WorldNormal);
	vec3 viewDir = u_CameraPosition - v_WorldPos;
	float distanceToViewer = length(viewDir);
	viewDir = normalize(viewDir);

	if (u_cullBackface && dot(norm, viewDir) <= 0.0) {
		discard;
	}

	vec3 result = vec3(0,0,0);

	 for (int i = 0; i < numberOfDirLights; i++) {
		result += calcDirLights(u_DirectionalLights[i], u_Material , norm, viewDir, v_WorldPos);
	}

	for (int i = 0; i < numberOfPointLights; i++) {
		result += calcPointLights(u_PointLights[i], u_Material, norm, viewDir, v_WorldPos);
	}


	result *= u_Material.albedo;
	FragColor = vec4(result, 1.0);
	return;
}


vec3 calcDirLights(DirectionalLight light, Material mat, vec3 normal, vec3 fragToViewDirection, vec3 fragPosition) {
	vec3 ambient = mat.ambient * light.color;

	vec3 lightDir = normalize(light.direction);
	vec3 diffuse = max(dot(normal, -lightDir), 0.0) * mat.diffuse * light.color;

	vec3 reflectedDir = normalize(reflect(-lightDir, normal));
	vec3 specular = pow(max(dot(reflectedDir, fragToViewDirection), 0.0), mat.shininess) * mat.specular * light.color;

	return (ambient + diffuse + specular) * light.intensity;

}

vec3 calcPointLights(PointLight light, Material mat, vec3 normal, vec3 fragToViewDirection, vec3 fragPosition) {

	vec3 ambient = mat.ambient * light.color;

	vec3 fragDirectionToLight = normalize(vec3(light.position - fragPosition));
	float distanceToLight = length(light.position - fragPosition);
	vec3 diffuse = mat.diffuse * max(dot(fragDirectionToLight, normal), 0.0) * light.color;

	vec3 reflectedDir = normalize(reflect(fragDirectionToLight, normal));

	vec3 specular = pow(max(dot(reflectedDir, fragToViewDirection), 0.0), mat.shininess) * mat.specular * light.color;

	float attenuatedIntensity = 1 / (light.constant + (light.linear * distanceToLight) + (light.quadratic * (distanceToLight * distanceToLight)));

	ambient *= attenuatedIntensity;
	diffuse *= attenuatedIntensity;
	specular *= attenuatedIntensity;

	return (ambient + diffuse + specular) * light.intensity;
}

vec3 calcSpotLights(SpotLight light, Material mat, vec3 normal, vec3 fragToViewDirection, vec3 fragPosition) {

	vec3 ambient = mat.ambient * light.color;

	vec3 fragDirectionToLight = normalize(vec3(light.position - fragPosition));
	vec3 diffuse = mat.diffuse * max(dot(fragDirectionToLight, normal), 0.0) * light.color;

	vec3 reflectedDir = normalize(reflect(fragDirectionToLight, normal));

	vec3 specular = pow(max(dot(reflectedDir, normalize(fragToViewDirection)), 0.0), mat.shininess) * mat.specular * light.color;

	float theta = max(dot(normalize(light.direction), -fragDirectionToLight), 0.0);
	float epsilon = light.innerCutoff - light.outerCutoff;
	float attenuatedIntensity = clamp(((theta - light.outerCutoff) / epsilon), 0.0, 1.0);

	ambient *= attenuatedIntensity;
	diffuse *= attenuatedIntensity;
	specular *= attenuatedIntensity;

	return (ambient + diffuse + specular) * light.intensity;

}