#version 330 core

layout (location = 0) in vec2 a_Position;
layout (location = 1) in vec2 a_TexCoords;

uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

uniform vec3 u_CameraRight_WorldSpace;
uniform vec3 u_CameraUp_WorldSpace;	
uniform vec3 u_ObjectPosition;

out vec2 v_UV;


void main()	 
{
	vec3 pos = vec3(a_Position, 0.0);
	v_UV = a_TexCoords;

	vec3 vertexPosition = u_ObjectPosition + u_CameraRight_WorldSpace * pos.x + u_CameraUp_WorldSpace * pos.y; 

	gl_Position = u_ProjectionMatrix * u_ViewMatrix * vec4(vertexPosition, 1.0);

}