#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoords;

uniform mat4 u_ModelMatrix;
uniform mat4 u_MVPMatrix;



out vec2 v_UV;
out vec3 v_WorldPos;
out vec3 v_WorldNormal;

void main() {

	gl_Position = u_MVPMatrix * vec4(a_Position, 1.0);
	v_UV = a_TexCoords;
	v_WorldPos = vec3(u_ModelMatrix * vec4(a_Position, 1.0));
	v_WorldNormal = mat3(transpose(inverse(u_ModelMatrix))) * a_Normal;
}