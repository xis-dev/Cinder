#version 330 core

vec3 positions[4] = vec3[4]( vec3(-0.5,  0.5, 0.0), 
						   vec3(-0.5, -0.5, 0.0),
						   vec3( 0.5, -0.5, 0.0),
						   vec3( 0.5,  0.5, 0.0));

vec2 texCoords[4] = vec2[4](vec2(0.0, 1.0), vec2(0.0, 0.0),
							vec2(1.0, 0.0), vec2(1.0, 1.0));

int indices[6] = int[6](0, 1, 2, 2, 3, 0);

uniform mat4 u_ProjectionMatrix;
uniform mat4 u_ViewMatrix;

uniform vec3 u_CameraRight_WorldSpace;
uniform vec3 u_CameraUp_WorldSpace;	
uniform vec3 u_ObjectPosition;

out vec2 v_UV;


void main()	 
{
	int index = indices[gl_VertexID];
	vec3 pos = positions[index];
	v_UV = texCoords[index];

	vec3 vertexPosition = u_ObjectPosition + u_CameraRight_WorldSpace * pos.x + u_CameraUp_WorldSpace * pos.y; 

	gl_Position = u_ProjectionMatrix * u_ViewMatrix * vec4(vertexPosition, 1.0);

}