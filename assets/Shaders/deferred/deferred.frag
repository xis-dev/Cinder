#version 330 core


layout (location = 0) out vec4 g_Position;
layout (location = 1) out vec4 g_ColorSpec;
layout (location = 2) out vec4 g_Normal;
layout (location = 3) out vec4 g_Material;

in vec2 v_UV;
in vec3 v_WorldNormal;
in vec3 v_WorldPos;
in vec4 v_LightSpacePos;
in mat3 v_TBN;
in vec3 v_TangentCameraPos;
in vec3 v_TangentFragPos;

#define MAX_MAPS_DIFFUSE 10
#define MAX_MAPS_SPECULAR 10
#define MAX_MAPS_NORMAL 10
#define MAX_MAPS_HEIGHT 10

struct Material {
    vec3 albedo;
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

uniform Material u_Material;

uniform sampler2D t_Specular[MAX_MAPS_SPECULAR];
uniform sampler2D t_Diffuse[MAX_MAPS_DIFFUSE];
uniform sampler2D t_Normal[MAX_MAPS_NORMAL];
uniform sampler2D t_Height[MAX_MAPS_HEIGHT];

uniform int u_DiffuseMapCount;
uniform int u_SpecularMapCount;
uniform int u_NormalMapCount;
uniform int u_HeightMapCount;

uniform float u_ParallaxHeightScale;


vec2 parallaxMapping(vec2 uv, vec3 viewDir, int heightMapCount);




void main() {

    int numberOfDiffuseMaps = clamp(u_DiffuseMapCount, 0, MAX_MAPS_DIFFUSE);
    int numberOfSpecularMaps = clamp(u_SpecularMapCount, 0, MAX_MAPS_SPECULAR);
    int numberOfNormalMaps = clamp(u_NormalMapCount, 0, MAX_MAPS_NORMAL);
    int numberOfHeightMaps = clamp(u_HeightMapCount, 0, MAX_MAPS_HEIGHT);

    g_Position = vec4(v_WorldPos, 1.0);

    vec3 tangentViewDir = normalize(v_TangentCameraPos - v_TangentFragPos);

    vec2 texCoords = v_UV;

    if (numberOfHeightMaps > 0) {
        texCoords = parallaxMapping(v_UV, tangentViewDir, numberOfHeightMaps);
        texCoords = vec2
        (
        texCoords.x - floor(texCoords.x),
        texCoords.y - floor(texCoords.y)
        );
    }

    vec3 diffuseColor = numberOfDiffuseMaps > 0 ? vec3(0.0) : vec3(1.0);
    for (int i = 0; i < numberOfDiffuseMaps; i++) {
        diffuseColor += texture(t_Diffuse[i], texCoords).rgb;
    }

    diffuseColor /= max(numberOfDiffuseMaps, 1);
    //diffuseColor *= u_Material.albedo;

    vec3 specularColor = numberOfSpecularMaps > 0 ? vec3(0.0) : vec3(0.0);
    for (int i = 0; i < numberOfSpecularMaps; i++) {
        specularColor += texture(t_Specular[i], texCoords).r;
    }
    specularColor /= max(numberOfSpecularMaps, 1);

    g_ColorSpec.rgb = diffuseColor;
    g_ColorSpec.a = specularColor.r;

    vec3 normalColor = normalize(v_WorldNormal);
    if (numberOfNormalMaps > 0) {
        normalColor = vec3(0.0);
        for (int i = 0; i < numberOfNormalMaps; ++i) {
            normalColor += (texture(t_Normal[i], texCoords)).rgb;
        }
        normalColor /= numberOfNormalMaps;

        normalColor = normalize(normalColor * 2.0 - 1.0);
        normalColor = normalize((v_TBN) * normalColor);

    }

    g_Normal = vec4(normalColor * 0.5 + 0.5, 1.0);
    g_Material = vec4(u_Material.ambient, u_Material.diffuse, u_Material.specular, u_Material.shininess);

}

vec2 parallaxMapping(vec2 uv, vec3 viewDir, int heightMapCount) {

    // number of depth layers
    const float minLayers = 8.0;
    const float maxLayers = 32.0;

    vec2 finalTexCoords = vec2(0.0);
    for (int i = 0; i < heightMapCount; i++) {
        float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));
        // calculate the size of each layer
        float layerDepth = 1.0 / numLayers;
        // depth of current layer
        float currentLayerDepth = 0.0;

        float height = texture(t_Height[i], uv).r;

        vec2 p = vec2(0.0);
        p = viewDir.xy * (height * u_ParallaxHeightScale);


        vec2 deltaTexCoords = p / numLayers;
        vec2  currentTexCoords  = uv;
        float currentDepthMapValue = texture(t_Height[i], currentTexCoords).r;

        while(currentLayerDepth < currentDepthMapValue)
        {
            // shift texture coordinates along direction of P
            currentTexCoords -= deltaTexCoords;
            // get depthmap value at current texture coordinates
            currentDepthMapValue = texture(t_Height[i], currentTexCoords).r;
            // get depth of next layer
            currentLayerDepth += layerDepth;
        }
        vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

        // get depth after and before collision for linear interpolation
        float afterDepth  = currentDepthMapValue - currentLayerDepth;
        float beforeDepth = texture(t_Height[i], prevTexCoords).r - currentLayerDepth + layerDepth;

        // interpolation of texture coordinates
        float weight = afterDepth / (afterDepth - beforeDepth);
        finalTexCoords += prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    }
    finalTexCoords /= heightMapCount;

    return finalTexCoords;

}