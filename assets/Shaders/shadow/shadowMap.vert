#version 330 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_Tex;

uniform mat4 u_LightProjView;
uniform mat4 u_Model;

out vec2 texCoords;

void main() {
texCoords = a_Tex;
	gl_Position = u_LightProjView * u_Model * vec4(a_Position, 1.0);
}
