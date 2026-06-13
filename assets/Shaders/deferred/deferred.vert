#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_BitTangent;

uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
uniform mat4 u_MVPMatrix;
uniform mat4 u_LightSpaceMatrix;
uniform vec3 u_CameraPosition;



out vec2 v_UV;
out vec3 v_ViewPos;
out vec3 v_ViewNormal;
out vec4 v_LightSpacePos;
out mat3 v_TBN;
out vec3 v_TangentCameraPos;
out vec3 v_TangentFragPos;

void main() {

    mat4 mvMat = u_ViewMatrix * u_ModelMatrix;
    gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
    v_UV = a_TexCoords;
    v_ViewPos = vec3(mvMat * vec4(a_Position, 1.0));
    vec3 worldPos = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
    vec3 modelNorm = mat3(transpose(inverse(u_ModelMatrix))) * a_Normal;
    v_ViewNormal = mat3(u_ViewMatrix) * (modelNorm);
    v_LightSpacePos = u_LightSpaceMatrix * vec4(worldPos, 1.0);

    vec3 T = normalize(vec3(mvMat * vec4(a_Tangent, 0.0)));
    vec3 B = normalize(vec3(mvMat * vec4(a_BitTangent, 0.0)));
    vec3 N = normalize(vec3(mvMat * vec4(a_Normal,  0.0)));
    mat3 TBN = transpose(mat3(T, B, N));

    v_TangentCameraPos = vec3(0.0);
    v_TangentFragPos = TBN * v_ViewPos;
    v_TBN = transpose(TBN);
}