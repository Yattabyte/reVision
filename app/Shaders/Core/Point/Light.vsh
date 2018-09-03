/* Point light - lighting shader. */
#version 460

layout(location = 0) in vec3 vertex;

struct Light_Struct {
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	float LightIntensity;
	float LightRadius;
};
struct Shadow_Struct {
	mat4 lightV; 
	mat4 lightPV[6];
	mat4 inversePV[6];
	int Shadow_Spot;
};
layout (std430, binding = 2) readonly coherent buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	mat4 pMatrix_Inverse;
	mat4 vMatrix_Inverse;
	vec3 EyePosition;
	vec2 CameraDimensions;
};
layout (std430, binding = 3) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 4) readonly buffer Shadow_Index_Buffer {
	int shadowIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};
layout (std430, binding = 9) readonly buffer Shadow_Buffer {
	Shadow_Struct shadowBuffers[];
};

layout (location = 0) flat out mat4 CamPInverse;
layout (location = 4) flat out mat4 CamVInverse;
layout (location = 8) flat out vec3 CamEyePosition;
layout (location = 9) flat out vec2 CamDimensions;
layout (location = 10) flat out vec3 LightColorInt;
layout (location = 11) flat out vec3 LightPosition;
layout (location = 12) flat out float LightRadius2;
layout (location = 13) flat out int ShadowSpotFinal;
layout (location = 14) flat out float ShadowIndexFactor;

void main()
{		
	CamPInverse = pMatrix_Inverse;
	CamVInverse = vMatrix_Inverse;
	CamEyePosition = EyePosition;
	CamDimensions = CameraDimensions;
	const int lightIndex = lightIndexes[gl_InstanceID];
	const int shadowIndex = shadowIndexes[gl_InstanceID];	
	LightColorInt = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
	LightPosition = lightBuffers[lightIndex].LightPosition.xyz;
	LightRadius2 = lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius;	
	ShadowSpotFinal = shadowBuffers[shadowIndex].Shadow_Spot/6;	
	ShadowIndexFactor = shadowIndex != -1 ? 1.0f : 0.0f;	
	gl_Position = pMatrix * vMatrix * lightBuffers[lightIndex].mMatrix * vec4(vertex, 1.0); 
}