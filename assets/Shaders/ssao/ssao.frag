#version 330 core

out float FragColor;

in vec2 v_UV;

uniform sampler2D t_GDepth;
uniform sampler2D t_GNormal;

uniform sampler2D t_Noise;

uniform mat4 m_View;
uniform mat4 m_InvProjection;
uniform mat4 m_Projection;

uniform vec2 u_WindowSize;
uniform vec2 u_NoiseSize;

uniform float u_SampleRadius;
uniform float u_SampleBias;
uniform float u_Strength;

uniform vec3 u_SampleKernel[64];

const int kernelSize = 64;

vec3 depthToViewPos(vec2 uv, float depth) {

    // Ensure w is 1
    vec4 viewPos = vec4(1.0);
    // Back to ndc
    viewPos.x = uv.x * 2.0 - 1.0;
    viewPos.y = uv.y * 2.0 - 1.0;
    viewPos.z = depth * 2.0 - 1.0;

    // Undo projection & divide by correct w for view
    viewPos = m_InvProjection * viewPos;

    return viewPos.xyz/viewPos.w;
}

void main() {
    // Noise scale for render based on window


    vec2 noiseScale = u_WindowSize/u_NoiseSize;

    vec3 fragPos_View = depthToViewPos(v_UV, texture(t_GDepth, v_UV).r);


    vec3 normal_View = normalize(mat3(m_View) * (texture(t_GNormal, v_UV).rgb * 2.0 - 1.0));
    vec3 randomVec = texture(t_Noise, v_UV * noiseScale).rgb;

    vec3 tangent = normalize(randomVec - dot(randomVec, normal_View) * normal_View);
    vec3 bitangent = cross(normal_View, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal_View);

    float occlusion = 0.0;
    for (int i = 0; i < 64; i++) {
        vec3 samplePos = TBN * u_SampleKernel[i];
        samplePos = fragPos_View + samplePos * u_SampleRadius;

        vec4 screenPos = m_Projection * vec4(samplePos, 1.0);
        screenPos /= screenPos.w;
        screenPos.xyz = screenPos.xyz * 0.5 + 0.5;

        // TODO: Switch depth so its depth from view not world

        vec3 sampleDepthPos = depthToViewPos(screenPos.xy, texture(t_GDepth, screenPos.xy).r);

        float rangeCheck = smoothstep(0.0, 1.0, u_SampleRadius / abs(fragPos_View.z - sampleDepthPos.z));
        occlusion += (sampleDepthPos.z >= samplePos.z + u_SampleBias ? 1.0 : 0.0) * rangeCheck;
    }

  //  occlusion = occlusion > 0.0 ? 1.0 : 0.0;
    occlusion = 1.0 - (occlusion / kernelSize);
    occlusion = pow(occlusion, u_Strength);
    FragColor = occlusion;
}