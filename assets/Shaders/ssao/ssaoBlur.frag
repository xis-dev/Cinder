#version 330 core

out float FragColor;
in vec2 v_UV;

uniform sampler2D t_SSAO;
void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(t_SSAO, 0));
    float value = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(x * texelSize.x, y * texelSize.y);
            value += texture(t_SSAO, v_UV + offset).r;
        }
    }

    FragColor = value / (4.0 * 4.0);
}