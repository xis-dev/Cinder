#version 330 core

out vec4 FragColor;

in vec2 v_UV;

uniform sampler2D u_Pos;
uniform sampler2D u_ColSpec;
uniform sampler2D u_Norm;


const float offset = 1.0/300.0;

void main()	{

//    vec2 offsets[9] = vec2[](
//    vec2(-offset,  offset), // top-left
//    vec2( 0.0f,    offset), // top-center
//    vec2( offset,  offset), // top-right
//    vec2(-offset,  0.0f),   // center-left
//    vec2( 0.0f,    0.0f),   // center-center
//    vec2( offset,  0.0f),   // center-right
//    vec2(-offset, -offset), // bottom-left
//    vec2( 0.0f,   -offset), // bottom-center
//    vec2( offset, -offset)  // bottom-right
//    );
//
//    float kernel[9] = float[](
//    1.0, 1.0, 1.0,
//    1.0,  -8.0, 1.0,
//    1.0, 1.0, 1.0
//    );
//
//    vec3 sampleTex[9];
//    for (int i = 0; i < 9; i++) {
//        sampleTex[i] = vec3(texture(u_ScreenTexture, v_UV.st + offsets[i]));
//    }
//
//    vec3 col = vec3(0.0);
//    for (int i = 0; i < 9; i++) {
//        col += sampleTex[i] * kernel[i];
//    }


    if (v_UV.x < 0.5 && v_UV.y > 0.5) {
        FragColor = texture(u_Pos, fract(v_UV * 2.0));
        return;
    }

if (v_UV.x < 0.5 && v_UV.y < 0.5) {
    FragColor = texture(u_Norm, fract(v_UV * 2.0));
    return;
}

if (v_UV.x > 0.5 && v_UV.y > 0.5) {
    FragColor = vec4(texture(u_ColSpec, fract(v_UV * 2.0)).rgb, 1.0);
    return;
}

if (v_UV.x > 0.5 && v_UV.y < 0.5) {
    FragColor = vec4(vec3(texture(u_ColSpec, fract(v_UV * 2.0)).a), 1.0);
    return;
}
//
//    vec3 col = texture(u_ScreenTexture, v_UV).rgb;
//    FragColor = vec4(col, 1.0);


}