#version 330 core



uniform vec3 u_CameraPosition;
uniform float gridSize = 100.0;
uniform mat4 u_VPMatrix;

out vec3 v_WorldPos;
const vec3 Pos[4] = vec3[4](
    vec3(-1.0, 0.0, -1.0),      // bottom left
    vec3( 1.0, 0.0, -1.0),      // bottom right
    vec3( 1.0, 0.0,  1.0),      // top right
    vec3(-1.0, 0.0,  1.0)       // top left
);

const int Indices[6] = int[6](0, 2, 1, 2, 0, 3);




void main() {
			
 int Index = Indices[gl_VertexID];
 vec3 vPos = Pos[Index] * gridSize;

//
vPos.x += u_CameraPosition.x;
vPos.z += u_CameraPosition.z;

vPos.y = -0.1;

v_WorldPos =	vPos;

gl_Position = u_VPMatrix * vec4(vPos, 1.0);

}