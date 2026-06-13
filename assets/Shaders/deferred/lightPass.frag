#version 330 core


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

out vec4 FragColor;

in vec2 v_UV;

const float kPI = 3.14159265;

uniform float u_HDRExposure;

#define MAX_POINT_LIGHTS 18
#define MAX_SPOT_LIGHTS 1
#define MAX_DIR_LIGHTS 1

uniform int u_DirLightCount;
uniform int u_PointLightCount;

uniform DirectionalLight u_DirectionalLights[MAX_DIR_LIGHTS];
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];

uniform sampler2D u_GPosition;
uniform sampler2D u_GColorSpec;
uniform sampler2D u_GNormal;
uniform sampler2D u_GMaterial;
uniform sampler2D u_SSAO;

uniform vec3 u_CameraPosition;
uniform mat4 u_LightSpaceMatrix;

uniform float u_Gamma;
uniform float u_FarPlane;
uniform bool u_Blinn;
uniform bool u_SSAOActive;

uniform sampler2D u_ShadowMap;
uniform samplerCube u_PointMap;

vec3 sampleOffsetDirections[20] = vec3[]
(
vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex);

vec4 lightSpacePos;
vec3 worldPos;

vec3 myReflect(vec3 v, vec3 a) {
    vec3 vNorm = normalize(v);
    vec3 aNorm = normalize(a);

    vec3 vPar = dot(vNorm, aNorm) * aNorm;
    vec3 vPerp = vNorm - vPar;
    vPar *= -1;

    // reduced form but didnt want to use the pregiven one
    //return (vNorm - (2 * (dot(vNorm, aNorm) * aNorm))) * length(v);
    return (vPerp + vPar) * length(v);
}

void main(){

    int numberOfDirLights = clamp(u_DirLightCount, 0, MAX_DIR_LIGHTS);
    int numberOfPointLights = clamp(u_PointLightCount, 0, MAX_POINT_LIGHTS);

    vec3 norm = texture(u_GNormal, v_UV).rgb * 2.0 - 1.0;
    vec3 viewDir = normalize(u_CameraPosition - texture(u_GPosition, v_UV).rgb);

    vec3 diffuseTex = texture(u_GColorSpec, v_UV).rgb;
    vec3 specularTex = vec3(texture(u_GColorSpec, v_UV).a);


    float ambientOcclusion = u_SSAOActive ? texture(u_SSAO, v_UV).r : 1.0;
    vec3 ambientCol = vec3(0.05, 0.05, 0.08);
    vec3 ambient = texture(u_GMaterial, v_UV).r * ambientCol * diffuseTex * ambientOcclusion;
    vec3 result = ambient;

    worldPos = texture(u_GPosition, v_UV).rgb;
    lightSpacePos = (u_LightSpaceMatrix * vec4(worldPos, 1.0));

    for (int i = 0; i < numberOfDirLights; i++) {
        result += calcDirLight(u_DirectionalLights[i], norm, viewDir, diffuseTex, specularTex);
    }


    for (int i = 0; i < numberOfPointLights; i++) {
        result += calcPointLight(u_PointLights[i], norm, viewDir, diffuseTex, specularTex);
    }

//
//    vec3 mapped = vec3(1.0) - exp(-result * u_HDRExposure);
//    mapped = pow(mapped, vec3(1.0/u_Gamma));
    FragColor = vec4(result, 1.0);
}

float dirShadowCalc(vec4 lightSpacePos, float bias) {
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(u_ShadowMap, projCoords.xy).r;

    float currentDepth = projCoords.z;

    float shadow = 0.0;
    vec2 texelSize = (1.0 / textureSize(u_ShadowMap, 0)).rg;
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

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex) {


    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(lightDir, normal), 0.0) * texture(u_GMaterial, v_UV).g;
    vec3 diffuse = diffuseTex * diff * light.color;


    float spec = 0.0;

    if (u_Blinn) {

        float energyConserv = ( 8.0 + texture(u_GMaterial, v_UV).a ) / ( 8.0 * kPI );
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), texture(u_GMaterial, v_UV).a * 4.0);

    }
    else {
        float energyConserv = ( 2.0 + texture(u_GMaterial, v_UV).a ) / ( 2.0 * kPI );
        vec3 reflectedDir = vec3(0.0);
        if (u_SSAOActive) {
             reflectedDir = normalize(myReflect(-lightDir,normal));
        }
        else {
            reflectedDir = normalize(reflect(-lightDir,normal));

        }
        spec = pow(max(dot(viewDir, reflectedDir), 0.0), texture(u_GMaterial, v_UV).a);
    }

    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);

    float specMask = step(0.0, NdotV) * step(0.0, NdotL);
    spec *= specMask;
    vec3 specular = specularTex * spec * texture(u_GMaterial, v_UV).b * light.color;

    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = dirShadowCalc(lightSpacePos, bias);

    return ((1.0 - shadow) * (diffuse + specular)) * light.intensity;
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



vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex) {

    vec3 lightDir = normalize(light.position - worldPos);
    float diff = max(dot(normal, lightDir), 0.0);


    float dist = length(light.position - worldPos);

    float spec = 0.0;
    if (u_Blinn) {

        float energyConserv = ( 8.0 + texture(u_GMaterial, v_UV).a ) / ( 8.0 * kPI );
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec =  pow(max(dot(normal, halfwayDir), 0.0), texture(u_GMaterial, v_UV).a * 4.0);

    }
    else {
        float energyConserv = ( 2.0 + texture(u_GMaterial, v_UV).a ) / ( 2.0 * kPI );

        vec3 reflectedDir = normalize(reflect(-lightDir	, normal));
        spec = pow(max(dot(viewDir, reflectedDir), 0.0), texture(u_GMaterial, v_UV).a);
    }

    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));

    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);

    float specMask = step(0.0, NdotV) * step(0.0, NdotL);
    spec *= specMask;
    vec3 diffuse  = texture(u_GMaterial, v_UV).g  * diff * diffuseTex * light.color;
    vec3 specular = texture(u_GMaterial, v_UV).b * spec * specularTex * light.color;


    diffuse  *= attenuation;
    specular *= attenuation;
    float bias = max(0.05 * (1.0 - max(dot(normal, lightDir), 0.0)), 0.005);
    float diskRadius = (1.0 + (length(u_CameraPosition - worldPos) / u_FarPlane)) / 25.0;
    float shadow = pointShadowCalc(light.position,worldPos, bias, diskRadius);
    vec3 result = ((1.0 - shadow) * (diffuse + specular)) * light.intensity;

    return result;

}