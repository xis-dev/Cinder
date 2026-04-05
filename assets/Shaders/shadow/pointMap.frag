#version 330 core
in vec4 FragPos;

uniform vec3 u_LightPos;
uniform float u_FarPlane;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - u_LightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / u_FarPlane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}  