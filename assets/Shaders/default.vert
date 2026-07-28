#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;
layout (location = 3) in vec3 a_Tangent;
layout (location = 4) in vec3 a_BiTangent;

uniform mat4 m_Model;
uniform mat4 m_View;
uniform mat4 m_MVP;
uniform mat4 m_MV;

uniform mat4 m_LightSpace;
uniform vec3 u_CameraPosition;



out vec2 v_UV;
out vec3 v_WorldPos;
out vec3 v_WorldNormal;
out vec4 v_LightSpacePos;
out mat3 v_TBN;
out vec3 v_TangentCameraPos;
out vec3 v_TangentFragPos;

void main() {

	gl_Position = m_MVP * vec4(a_Position, 1.0);
	v_UV = a_TexCoords;
	v_WorldPos = vec3(m_Model * vec4(a_Position, 1.0));
    mat3 modelNoScale = mat3(transpose(inverse(m_Model)));
	v_LightSpacePos = m_LightSpace * vec4(v_WorldPos, 1.0);

    v_WorldNormal = normalize(modelNoScale * a_Normal);
    vec3 T = normalize(modelNoScale * a_Tangent);
    vec3 B = normalize(modelNoScale * a_BiTangent);

   mat3 TBN = transpose(mat3(T, B, v_WorldNormal));

   v_TangentCameraPos = TBN * u_CameraPosition;
   v_TangentFragPos = TBN * v_WorldPos;
   v_TBN = transpose(TBN);
}																			 