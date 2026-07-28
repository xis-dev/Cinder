#version 330 core

out vec4 FragColor;

in vec2 v_UV;

uniform sampler2D t_TextureToBlur;
uniform vec2 u_TexelSize;
uniform bool horizontal;

// Sample coordinates from: https://community.khronos.org/t/downsampling-a-texture/53403/5
float sampleCoords[5] = float[5](9.4, 7.4, 5.4, 3.4, 1.4);
float sampleWeights[5] = float[5](0.01, 0.04, 0.08, 0.15, 0.3);
void main() {
    vec4 result = texture(t_TextureToBlur, v_UV) * sampleWeights[4]; // Start with center and its weight
    if (horizontal) {
        for (int i = 0; i < 5; i++) {
            result += (texture(t_TextureToBlur, v_UV + vec2(u_TexelSize.x * sampleCoords[i], 0))) * sampleWeights[i];
            result += (texture(t_TextureToBlur, v_UV - vec2(u_TexelSize.x * sampleCoords[i], 0))) * sampleWeights[i];
        }
    }
            // Vertical blur
    else {
        for (int i = 0; i < 5; i++) {
            result += (texture(t_TextureToBlur, v_UV + vec2(0, u_TexelSize.y * sampleCoords[i]))) * sampleWeights[i];
            result += (texture(t_TextureToBlur, v_UV - vec2(0, u_TexelSize.y * sampleCoords[i]))) * sampleWeights[i];
        }
    }

    FragColor = result;
}