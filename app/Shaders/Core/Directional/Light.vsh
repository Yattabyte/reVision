/* Directional light - lighting shader. */
#version 460
#define NUM_CASCADES 4

layout (location = 0) in vec3 vertex;

struct Light_Struct {
	vec4 LightColor;
	vec4 LightDirection;
	float LightIntensity;
};
struct Shadow_Struct {
	mat4 lightV;	
	int Shadow_Spot;
	float CascadeEndClipSpace[NUM_CASCADES];
	mat4 LightVP[NUM_CASCADES];
};
layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition;
	vec2 CameraDimensions;
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};
layout (std430, binding = 9) readonly buffer Shadow_Buffer {
	Shadow_Struct shadowBuffers[];
};
layout (std430, binding = 3) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 4) readonly buffer Shadow_Index_Buffer {
	int shadowIndexes[];
};

layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out float HasShadow;
layout (location = 2) flat out vec3 LightColorInt;
layout (location = 3) flat out vec3 LightDirection;
layout (location = 4) flat out int Shadow_Spot;
layout (location = 5) flat out vec4 CascadeEndClipSpace;
layout (location = 6) flat out mat4 LightVP[NUM_CASCADES];
layout (location = 22) flat out mat4 CamPInverse;
layout (location = 26) flat out mat4 CamVInverse;
layout (location = 30) flat out vec3 CamEyePosition;

void main()
{	
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	const int lightIndex = lightIndexes[gl_InstanceID];
	const int shadowIndex = shadowIndexes[gl_InstanceID];
	HasShadow = shadowIndex != -1 ? 1.0f : 0.0f;	
	LightColorInt = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
	LightDirection = lightBuffers[lightIndex].LightDirection.xyz;
	Shadow_Spot = shadowBuffers[shadowIndex].Shadow_Spot;
	for (uint x = 0; x < NUM_CASCADES; ++x) {
		CascadeEndClipSpace[x] = shadowBuffers[shadowIndex].CascadeEndClipSpace[x];
		LightVP[x] = shadowBuffers[shadowIndex].LightVP[x];
	}
	CamPInverse = pMatrix_Inverse;
	CamVInverse = vMatrix_Inverse;
	CamEyePosition = EyePosition;
	gl_Position = vec4(vertex, 1);
}