#version 330 core

in vec3 v_WorldPos;

uniform vec3 u_CameraPosition;

uniform float gridSize = 100.0;
uniform float gridCellSize = 0.225;
uniform vec4 gridColorThin = vec4(0.5, 0.5, 0.5, 0.5);
uniform vec4 gridColorThick = vec4(0.8, 0.8, 0.8, 1.0);
uniform float minPixelsBetweenCell = 2.0;
out vec4 FragColor;


float log10(float x)
{
    float f = log(x) / log(10.0);
    return f;
}


float satf(float x)
{
    float f = clamp(x, 0.0, 1.0);
    return f;
}


vec2 satv(vec2 x)
{
    vec2 v = clamp(x, vec2(0.0), vec2(1.0));
    return v;
}


float max2(vec2 v)
{
    float f = max(v.x, v.y);
    return f;
}

void main() 
{

	vec2 dvx = vec2(dFdx(v_WorldPos.x), dFdy(v_WorldPos.x));
	vec2 dvy =  vec2(dFdx(v_WorldPos.z), dFdy(v_WorldPos.z));

	vec2 dudv = vec2(length(dvx), length(dvy));

	float l = length(dudv);

	float LOD = max(0.0, log10(l * minPixelsBetweenCell / gridCellSize) + 1.0);

	float gridCellSizeLod0 = gridCellSize * pow(10.0, floor(LOD));
	float gridCellSizeLod1 = gridCellSizeLod0 * 10.0;
	float gridCellSizeLod2 = gridCellSizeLod1 * 10.0;



	dudv *= 4.0;
	vec2 mod_div_dudv = mod(v_WorldPos.xz, gridCellSizeLod0) / dudv;
	float lod0a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)));
	
	 mod_div_dudv = mod(v_WorldPos.xz, gridCellSizeLod1) / dudv;
	float lod1a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)));

	 mod_div_dudv = mod(v_WorldPos.xz, gridCellSizeLod2) / dudv;
	float lod2a = max2(vec2(1.0) - abs(satv(mod_div_dudv) * 2.0 - vec2(1.0)));

	float LOD_fade = fract(LOD);

	vec4 color;

	if (lod2a > 0.0) {
		color = gridColorThick;
	color.a *= lod2a;
	}
	else {
		if (lod1a > 0.0) { 
				color = mix(gridColorThick, gridColorThin, LOD_fade);
				color.a *= lod1a;
		}
		else {
			color = gridColorThin;
			color.a *= (lod0a * (1.0 - LOD_fade));
		}	
	}

	float opacityFalloff  = (1.0 - satf(length(v_WorldPos.xz - u_CameraPosition.xz) / gridSize));

	color.a *= opacityFalloff;

	if (color.a < 0.01) discard;
	FragColor = color; 
}