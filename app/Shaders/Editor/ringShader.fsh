/* Ring shader for the rotation gizmo. */
#version 460

layout (location = 0) in vec2 UV;
layout (location = 1) flat in uint axisID;
layout (location = 2) in vec3 normal;
layout (location = 3) flat in mat3 usedMat;

layout (location = 0) out vec4 fragColor;

layout (location = 8) uniform uint SelectedAxes = 0;
layout (location = 9) uniform uint HoveredAxes = 0;
const vec3 ColorScheme[4] = {
	vec3(1,0,0),
	vec3(0,1,0),
	vec3(0,0,1),
	vec3(1,1,1)
};
const uint AXES[4] = {
	1u,	2u,	4u, 8u
};


void main()
{	
	// Starting Variables
	const vec3 P = vec3(0,0,-10);
	const vec3 E = normalize(P);
	const vec3 N = normalize(usedMat * normal);
	const vec3 R = reflect(-P, N);
	const float NdotE = clamp(dot(N, E), 0.0f, 1.0f);
	const float NdotR = clamp(dot(N, R), 0.0f, 1.0f);
		
	// Calculate Light
	const float metalness = 0.125f;
	const float radiance = 3.0f;	
	vec3 albedo = ColorScheme[axisID];	
	if (SelectedAxes == AXES[axisID])
		albedo = vec3(1, 0.8, 0);	
	const vec3 ambient = vec3(0.125f);
	const vec3 diffuse = albedo / 3.14159f;
	const vec3 specular = vec3(pow(clamp(dot(E, R), 0.0f, 1.0f), 5.0f) / (10.0f));

	// Derive brightness
	const vec3 F0 = mix(vec3(0.03f), albedo, metalness);
	const vec3 Fs = F0 + (1.0f - F0) * pow(1.0f - NdotE, 5.0f);
	const vec3 ratio = (1.0f - Fs) * (1.0f - metalness);
	vec3 result = ((ratio * diffuse + specular) * radiance) + ambient;
	result = 1.0f - ((1.0f - result) * (1.0f - result));
	result = result * result;
	result = result * result;	
	fragColor.rgb = result;
	fragColor.a = (HoveredAxes == AXES[axisID]) ? 0.66f : 1.0f;
}