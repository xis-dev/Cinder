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

const float kPI = 3.14159265;


in vec2 v_UV;
in vec3 v_WorldNormal;
in vec3 v_WorldPos;
in vec4 v_LightSpacePos;
in mat3 v_TBN;

uniform vec3 u_CameraPosition;

uniform Material u_Material;

uniform float u_Gamma;
uniform float u_FarPlane;

out vec4 FragColor;



float near = 0.1;
float far = 1000.0;

#define MAX_POINT_LIGHTS 5
#define MAX_SPOT_LIGHTS 1
#define MAX_DIR_LIGHTS 1

#define MAX_MAPS_SPECULAR 10
#define MAX_MAPS_DIFFUSE 10
#define MAX_MAPS_NORMAL 10

uniform int u_DirLightCount;
uniform int u_PointLightCount;
uniform int u_SpotLightCount;

uniform int u_DiffuseMapCount;
uniform int u_SpecularMapCount;
uniform int u_NormalMapCount;

uniform sampler2D t_Specular[MAX_MAPS_SPECULAR];
uniform sampler2D t_Diffuse[MAX_MAPS_DIFFUSE];
uniform sampler2D t_Normal;


uniform DirectionalLight u_DirectionalLights[MAX_DIR_LIGHTS];
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform SpotLight u_SpotLights[MAX_SPOT_LIGHTS];

uniform bool u_cullBackface;
uniform bool u_Blinn;

uniform sampler2D u_ShadowMap;
uniform samplerCube u_PointMap;


vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex);

float dirShadowCalc(vec4 lightSpacePos, float bias);

float linearizeDepth(float depth);

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
); 

void main() {

	vec3 viewDir = normalize(u_CameraPosition - v_WorldPos);


	int numberOfDirLights = clamp(u_DirLightCount, 0, MAX_DIR_LIGHTS);
	int numberOfPointLights = clamp(u_PointLightCount, 0, MAX_POINT_LIGHTS);
	int numberOfSpotLights = clamp(u_SpotLightCount, 0, MAX_SPOT_LIGHTS);

	int numberOfDiffuseMaps = clamp(u_DiffuseMapCount, 0, MAX_MAPS_DIFFUSE);
	int numberOfSpecularMaps = clamp(u_SpecularMapCount, 0, MAX_MAPS_SPECULAR);
	int numberOfNormalMaps = clamp(u_NormalMapCount, 0, MAX_MAPS_NORMAL);



	vec3 norm = normalize(v_WorldNormal);


	if (numberOfNormalMaps > 0) {

	norm = (texture(t_Normal, v_UV)).rgb;
	norm = normalize(norm * 2.0 - 1.0);
	norm = normalize((v_TBN) * norm);

	}
	else {

		norm = normalize(v_WorldNormal);
	}
		
		

	

	vec3 diffuseTex = vec3(0.0);
	if (numberOfDiffuseMaps > 0) {
		for (int i = 0; i < numberOfDiffuseMaps; i++) {
		diffuseTex += vec3(texture(t_Diffuse[i], v_UV));
	}
	}
	else {
	diffuseTex = vec3(1.0);	
	}
	diffuseTex /= max(numberOfDiffuseMaps, 1);
	

	vec3 specularTex = vec3(0.0);
	if (numberOfSpecularMaps > 0) {
		for (int i = 0; i < numberOfSpecularMaps; i++) {
		specularTex += vec3(texture(t_Specular[i], v_UV));
	}
	}
	else {
		specularTex = vec3(1.0);
	}
	specularTex /= max(numberOfSpecularMaps, 1);

vec3 ambientCol = vec3(0.05, 0.05, 0.08);
	vec3 ambient = u_Material.ambient * ambientCol * diffuseTex;
	vec3 result = ambient;


	 for (int i = 0; i < numberOfDirLights; i++) {
		result += calcDirLight(u_DirectionalLights[i], norm, viewDir, diffuseTex, specularTex);
	}


	for (int i = 0; i < numberOfPointLights; i++) {
		result += calcPointLight(u_PointLights[i], norm, viewDir, diffuseTex, specularTex);
	}

				FragColor = vec4(result * u_Material.albedo, 1.0) ;
	FragColor.rgb = pow(FragColor.rgb, vec3(1.0/u_Gamma));
	return;




}

float dirShadowCalc(vec4 lightSpacePos, float bias) {
	vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
	projCoords = projCoords * 0.5 + 0.5;

	float closestDepth = texture(u_ShadowMap, projCoords.xy).r;

	float currentDepth = projCoords.z;

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_ShadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
    for(int y = -1; y <= 1; ++y)
    {
        float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
    }    
}
shadow /= 9.0;

	 if(projCoords.z > 1.0) shadow = 0.0;

	return shadow;
}

float pointShadowCalc(vec3 lightPos, vec3 fragPos, float bias, float diskRadius) {
	vec3 fragToLight = vec3(fragPos - lightPos);
	    float closestDepth = texture(u_PointMap, fragToLight).r;

		closestDepth *= u_FarPlane;
		float currentDepth = length(fragToLight);  
		int samples = 20; 
		float shadow = 0.0;
		for (int i = 0; i < samples; ++i) {
			float closestDepth = texture(u_PointMap, fragToLight + (sampleOffsetDirections[i] * diskRadius)).r;
			  closestDepth *= u_FarPlane;
			shadow += currentDepth - bias > closestDepth ? 1.0: 0.0;
		}
		return shadow /= samples;
} 


vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex) {


	vec3 lightDir = normalize(-light.direction);
	float diff = max(dot(lightDir, normal), 0.0) * u_Material.diffuse;
	vec3 diffuse = diffuseTex * diff * light.color;


	float spec = 0.0;

	if (u_Blinn) {

	float energyConserv = ( 8.0 + u_Material.shininess ) / ( 8.0 * kPI ); 
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess * 4.0);

	}
	else {
	float energyConserv = ( 2.0 + u_Material.shininess ) / ( 2.0 * kPI ); 

	vec3 reflectedDir = normalize(reflect(-lightDir	, normal));
	spec = pow(max(dot(viewDir, reflectedDir), 0.0), u_Material.shininess);
	}

	float NdotV = max(dot(normal, viewDir), 0.0);
	float NdotL = max(dot(normal, lightDir), 0.0);

	float specMask = step(0.0, NdotV) * step(0.0, NdotL);
	spec *= specMask;
	vec3 specular = specularTex * spec * u_Material.specular * light.color;

	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);  
	float shadow = dirShadowCalc(v_LightSpacePos, bias);

	return ((1.0 - shadow) * (diffuse + specular)) * light.intensity;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex) {

	vec3 lightDir = normalize(light.position - v_WorldPos);
	float diff = max(dot(normal, lightDir), 0.0);
	

	float dist = length(light.position - v_WorldPos);

	float spec = 0.0;
	if (u_Blinn) {

	float energyConserv = ( 8.0 + u_Material.shininess ) / ( 8.0 * kPI ); 
	vec3 halfwayDir = normalize(lightDir + viewDir);
	spec =  pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess * 4.0);

	}
	else {
	float energyConserv = ( 2.0 + u_Material.shininess ) / ( 2.0 * kPI ); 

	vec3 reflectedDir = normalize(reflect(-lightDir	, normal));
	spec = pow(max(dot(viewDir, reflectedDir), 0.0), u_Material.shininess);
	}

	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
						
	float NdotV = max(dot(normal, viewDir), 0.0);
	float NdotL = max(dot(normal, lightDir), 0.0);

	float specMask = step(0.0, NdotV) * step(0.0, NdotL);
	spec *= specMask;
	vec3 diffuse  = u_Material.diffuse  * diff * diffuseTex * light.color;
	vec3 specular = u_Material.specular * spec * specularTex * light.color;


	diffuse  *= attenuation;
	specular *= attenuation;
	float bias = max(0.05 * (1.0 - max(dot(normal, lightDir), 0.0)), 0.005);
	float diskRadius = (1.0 + (length(u_CameraPosition - v_WorldPos) / u_FarPlane)) / 25.0;  
	float shadow = pointShadowCalc(light.position,v_WorldPos, bias, diskRadius);
	vec3 result = ((1.0 - shadow) * (diffuse + specular)) * light.intensity;

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

