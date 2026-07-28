#version 330 core

out vec4 FragColor;

in vec2 v_UV;

// Full size bloom texture
uniform sampler2D t_BloomTexture;
// Original texel size for downsample
uniform vec2 u_OriginalTexelSize;
float calcLuminance(vec3 colour) {
    return 0.2126 * colour.r + 0.7152 * colour.g + 0.0722 * colour.b;
}
void main() {
    vec2 offset = u_OriginalTexelSize * 0.5;

    vec3 col1 = texture(t_BloomTexture, v_UV + vec2(offset.x, offset.y)).rgb;
    vec3 col2 = texture(t_BloomTexture, v_UV + vec2(-offset.x, -offset.y)).rgb;
    vec3 col3 = texture(t_BloomTexture, v_UV + vec2(-offset.x, offset.y)).rgb;
    vec3 col4 = texture(t_BloomTexture, v_UV + vec2(offset.x, -offset.y)).rgb;

    vec3 finalCol = (col1 + col2 + col3 + col4) * 0.25;
    if (calcLuminance(finalCol) < 1.0) {
        finalCol = vec3(0.0);
    }
    FragColor = vec4(finalCol, 1.0);
}