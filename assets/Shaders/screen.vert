#version 330 core



const vec2 Pos[4] = vec2[4](
    vec2(-1.0, -1.0),      // bottom left
    vec2( 1.0, -1.0),      // bottom right
    vec2( 1.0,  1.0),      // top right
    vec2(-1.0,  1.0)       // top left
);

const vec2 TexCoords[4] = vec2[4](
    vec2(0.0, 0.0),
    vec2(1.0, 0.0),
    vec2(1.0, 1.0),
    vec2(0.0, 1.0)
);

const int Indices[6] = int[6](0, 2, 1, 2, 0, 3);

out vec2 v_UV;
void main() {
    int index = Indices[gl_VertexID];
    vec2 position = Pos[index];
	gl_Position = vec4(position.x, position.y, 0.0, 1.0);
	v_UV = TexCoords[index];
}