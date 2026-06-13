#version 330 core

out float FragColor;

in vec2 v_UV;

uniform sampler2D t_SsaoInput;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(t_SsaoInput, 0));
    float result = 0.0;

    for (int x = -2; x < 2; ++x) {
        for (int y = -2; y < 2; ++y) {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(t_SsaoInput, v_UV + offset).r;
        }
}

    FragColor = result / (4.0 * 4.0);
}