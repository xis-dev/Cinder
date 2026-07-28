#version 330 core

out vec4 FragColor;


in vec2 v_UV;

// TODO: Use depth and remove position buffer
uniform sampler2D t_GDepth;
uniform sampler2D t_GNormal;

uniform mat4 m_View;
uniform mat4 m_InvView;
uniform mat4 m_InvProjection;
uniform mat4 m_Projection;
uniform float u_SSAOStr;
uniform vec3 u_SSAOSamples[64];

vec3 depthToViewPos(vec2 texCoord, float depth) {
    vec4 viewPos = vec4(1.0);

    viewPos.z = depth * 2.0 - 1.0;
    viewPos.y = texCoord.y * 2.0 - 1.0;
    viewPos.x = texCoord.x * 2.0 - 1.0;

    viewPos = m_InvProjection * viewPos;
    return viewPos.xyz /= viewPos.w;

}

void main() {
    float depth = texture(t_GDepth, v_UV).r;
    vec3 currentPos_View = depthToViewPos(v_UV, depth);

    vec3 pureNormal = texture(t_GNormal, v_UV).rgb;
    vec3 currentNormal_View = normalize(m_View * vec4(pureNormal, 1.0)).rgb;
    vec3 otherVec = vec3(1, 0, 0);

    if (abs(dot(pureNormal, otherVec)) > 0.999) {
        otherVec = vec3(0, 1, 0);
    }

    vec3 tangent = otherVec - dot(otherVec, currentNormal_View) * currentNormal_View;
    vec3 bitangent = cross(currentNormal_View, tangent);

    vec3 finalCol = vec3(0.);
    mat3 TBN = mat3(tangent, bitangent, currentNormal_View);
    int occlusionCount = 0;
    for (int i = 0; i < 64; i++) {
        vec3 localSamplePos = normalize((0, 0, 1) + u_SSAOSamples[i]);
        localSamplePos = TBN * localSamplePos;

        vec4 projected = m_Projection * vec4(localSamplePos, 1.0);
        projected /= projected.w;

        float sampleDepth = projected.z * 0.5 + 0.5;
        float currentDepth = texture(t_GDepth, projected.rg * 0.5 + 0.5).r;
        if (currentDepth > sampleDepth) {
            finalCol += vec3(1.0);
            ++occlusionCount;
        }
    }

    finalCol = occlusionCount <= 0 ? vec3(1.0) : vec3(((finalCol.r / occlusionCount) * (1 / u_SSAOStr)));

    FragColor = vec4(finalCol, 1.0);
}