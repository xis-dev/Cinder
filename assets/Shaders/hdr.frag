#version 330 core

out vec4 FragColor;

in vec2 v_UV;

uniform sampler2D u_HDRTexture;
uniform float u_HDRExposure;
uniform float u_Gamma;


void main()	{
    vec3 hdrColor = texture(u_HDRTexture, v_UV).rgb;
  
    vec3 mapped = vec3(1.0) - exp(-hdrColor * u_HDRExposure);
    mapped = pow(mapped, vec3(1.0/u_Gamma));
    FragColor = vec4(mapped, 1.0);
}