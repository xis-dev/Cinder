#version 330 core
out vec4 FragColor;
  
in vec2 v_UV;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposure;
uniform float u_Gamma;

void main()
{             
    vec3 hdrColor = texture(scene, v_UV).rgb;      
    vec3 bloomColor = texture(bloomBlur, v_UV).rgb;
    hdrColor += bloomColor;
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    result = pow(result, vec3(1.0 / u_Gamma));
    FragColor = vec4(result, 1.0);
} 