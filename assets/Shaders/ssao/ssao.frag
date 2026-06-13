#version 330 core

out float FragColor;

in vec2 v_UV;

uniform sampler2D u_GPosition;
uniform sampler2D u_GColorSpec;
uniform sampler2D u_GNormal;

uniform sampler2D t_Noise;

uniform vec3 samples[64];

uniform mat4 u_ViewMatrix;
uniform mat4 u_Projection;

uniform float u_SsaoPow;
const int kernelSize = 64;
const float sampleRad = 0.5;
const float bias = 0.025;

const vec2 noiseScale = vec2(1600.0/4.0, 900.0/4.0);

void main() {
    vec3 fragPos = texture(u_GPosition, v_UV).xyz;
    fragPos = (u_ViewMatrix * vec4(fragPos, 1.0)).xyz;
    vec3 color = texture(u_GColorSpec, v_UV).rgb;

    vec3 normal = (texture(u_GNormal, v_UV).rgb * 2.0 - 1.0);
    normal = normalize(mat3(u_ViewMatrix) * normal);

    vec3 randomVec = texture(t_Noise, v_UV * noiseScale).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernelSize; ++i) {
        vec3 samplePos = TBN * samples[i];
        samplePos = fragPos + samplePos * sampleRad;

        vec4 offset = u_Projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        vec3 sampleWorldPos = texture(u_GPosition, offset.xy).xyz;
        float sampleDepth = (u_ViewMatrix * vec4(sampleWorldPos, 1.0)).z;

        float rangeCheck = smoothstep(0.0, 1.0, sampleRad / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(kernelSize));
    occlusion = pow(occlusion, u_SsaoPow);
    FragColor = occlusion;
}
