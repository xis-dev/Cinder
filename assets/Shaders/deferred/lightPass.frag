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
    float radius;
};

out vec4 FragColor;

in vec2 v_UV;

const float kPI = 3.14159265;


#define MAX_POINT_LIGHTS 20
#define MAX_SPOT_LIGHTS 1
#define MAX_DIR_LIGHTS 1


uniform int u_DirLightCount;
uniform int u_PointLightCount;

uniform DirectionalLight u_DirectionalLights[MAX_DIR_LIGHTS];
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];

uniform sampler2D u_GDepth;
uniform sampler2D u_GColorSpec;
uniform sampler2D u_GNormal;
uniform sampler2D u_GMaterial;
uniform sampler2D u_SSAO;

uniform mat4 m_InvView;
uniform mat4 m_InvProjection;

uniform vec3 u_CameraPosition;
uniform mat4 m_LightSpace;

uniform bool u_Blinn;
uniform bool u_SSAOActive;

uniform float u_NearPlane;
uniform float u_FarPlane;

uniform sampler2D u_ShadowMap;
uniform samplerCube[MAX_POINT_LIGHTS] t_PointMaps;

vec3 sampleOffsetDirections[20] = vec3[]
(
vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex, samplerCube shadowMap);


vec3 projectedDepthToWorld(vec2 texCoord, float depth) {
  // float z = reconstituteViewZ(texture(u_GDepth, v_UV).r);

    float z = depth * 2.0 - 1.0;

    float x = (texCoord.x * 2.0 - 1.0);
    float y = (texCoord.y * 2.0 - 1.0);

    vec4 clipPos = vec4(x, y, z, 1.0);
    vec4 viewPos = m_InvProjection * clipPos;
    viewPos /= viewPos.w;

    return vec3(m_InvView * viewPos);

}
float calcLuminance(vec3 colour) {
    return 0.2126 * colour.r + 0.7152 * colour.g + 0.0722 * colour.b;
}

float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * u_NearPlane * u_FarPlane) / (u_FarPlane + u_NearPlane - z * (u_FarPlane - u_NearPlane));
}

vec4 lightSpacePos;
vec3 worldPos;


void main(){

    int numberOfDirLights = clamp(u_DirLightCount, 0, MAX_DIR_LIGHTS);
    int numberOfPointLights = clamp(u_PointLightCount, 0, MAX_POINT_LIGHTS);

    worldPos = projectedDepthToWorld(v_UV, texture(u_GDepth, v_UV).r);


    vec3 norm = texture(u_GNormal, v_UV).rgb * 2.0 - 1.0;
    vec3 viewDir = normalize(u_CameraPosition - worldPos);

    vec3 diffuseTex = texture(u_GColorSpec, v_UV).rgb;
    vec3 specularTex = vec3(texture(u_GColorSpec, v_UV).a);


    float ambientOcclusion = u_SSAOActive ? texture(u_SSAO, v_UV).r : 1.0;
    vec3 ambientCol = vec3(0.05, 0.05, 0.08);
    vec3 ambient = texture(u_GMaterial, v_UV).r * ambientCol * diffuseTex * ambientOcclusion;
    vec3 result = ambient;

    lightSpacePos = (m_LightSpace * vec4(worldPos, 1.0));

    for (int i = 0; i < numberOfDirLights; i++) {
        result += calcDirLight(u_DirectionalLights[i], norm, viewDir, diffuseTex, specularTex);
    }


    for (int i = 0; i < numberOfPointLights; i++) {
        result += calcPointLight(u_PointLights[i], norm, viewDir, diffuseTex, specularTex, t_PointMaps[i]);
    }

//    BloomColor = vec4(0.0);
//    if (calcLuminance(result) > 1.0) {
//        BloomColor = vec4(result, 1.0);
//    }

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
             reflectedDir = normalize(reflect(-lightDir,normal));
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

float pointShadowCalc(vec3 lightPos, vec3 fragPos, float bias, float diskRadius, samplerCube shadowMap, float lightRadius) {
    vec3 fragToLight = vec3(fragPos - lightPos);

        float closestDepth = texture(shadowMap, fragToLight).r;

        closestDepth *= lightRadius;
        float currentDepth = length(fragToLight);
        int samples = 20;
        float shadow = 0.0;
        for (int i = 0; i < samples; ++i) {
            float closestDepth = texture(shadowMap, fragToLight + (sampleOffsetDirections[i] * diskRadius)).r;
            closestDepth *= lightRadius;
            shadow += currentDepth - bias > closestDepth ? 1.0: 0.0;
        }
    return shadow /= samples;
}



vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 diffuseTex, vec3 specularTex, samplerCube shadowMap) {

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
    float diskRadius = (1.0 + (length(u_CameraPosition - worldPos) / light.radius)) / 25.0;
    float shadow = pointShadowCalc(light.position,worldPos, bias, diskRadius, shadowMap, light.radius);
    vec3 result = ((1.0 - shadow) * (diffuse + specular)) * light.intensity;


    return result;

}