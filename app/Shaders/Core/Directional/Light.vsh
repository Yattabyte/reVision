/* Directional light - lighting shader. */
#version 460
#define NUM_CASCADES 4

layout (location = 0) in vec3 vertex;

struct Light_Struct {
	mat4 lightV;	
	mat4 LightVP[NUM_CASCADES];
	mat4 InverseLightVP[NUM_CASCADES];	
	float CascadeEndClipSpace[NUM_CASCADES];
	vec4 LightColor;
	vec4 LightDirection;
	float LightIntensity;
	int Shadow_Spot;
};
layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	vec3 EyePosition;
	vec2 CameraDimensions;
};
layout (std430, binding = 3) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out vec3 LightColorInt;
layout (location = 2) flat out vec3 LightDirection;
layout (location = 3) flat out int Shadow_Spot;
layout (location = 4) flat out vec4 CascadeEndClipSpace;
layout (location = 5) flat out mat4 LightVP[NUM_CASCADES];
layout (location = 21) flat out mat4 CamPInverse;
layout (location = 25) flat out mat4 CamVInverse;
layout (location = 29) flat out vec3 CamEyePosition;

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	const int lightIndex = lightIndexes[gl_InstanceID];
	LightColorInt = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
	LightDirection = lightBuffers[lightIndex].LightDirection.xyz;
	Shadow_Spot = lightBuffers[lightIndex].Shadow_Spot;
	for (uint x = 0; x < NUM_CASCADES; ++x) {
		CascadeEndClipSpace[x] = lightBuffers[lightIndex].CascadeEndClipSpace[x];
		LightVP[x] = lightBuffers[lightIndex].LightVP[x];
	}
	CamPInverse = inverse(pMatrix);
	CamVInverse = inverse(vMatrix);
	CamEyePosition = EyePosition;
	gl_Position = vec4(vertex, 1);
}