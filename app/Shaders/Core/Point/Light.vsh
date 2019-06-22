/* Point light - lighting shader. */
#version 460

layout(location = 0) in vec3 vertex;

struct Light_Struct {
	mat4 lightV; 
	mat4 lightPV[6];
	mat4 inversePV[6];
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	float LightIntensity;
	float LightRadius;
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

layout (location = 0) flat out mat4 CamPInverse;
layout (location = 4) flat out mat4 CamVInverse;
layout (location = 8) flat out vec3 CamEyePosition;
layout (location = 9) flat out vec2 CamDimensions;
layout (location = 10) flat out vec3 LightColorInt;
layout (location = 11) flat out vec3 LightPosition;
layout (location = 12) flat out float LightRadius2;
layout (location = 13) flat out int ShadowSpotFinal;

void main()
{		
	CamPInverse = inverse(pMatrix);
	CamVInverse = inverse(vMatrix);
	CamEyePosition = EyePosition;
	CamDimensions = CameraDimensions;
	const int lightIndex = lightIndexes[gl_InstanceID];
	LightColorInt = lightBuffers[lightIndex].LightColor.xyz * lightBuffers[lightIndex].LightIntensity;
	LightPosition = lightBuffers[lightIndex].LightPosition.xyz;
	LightRadius2 = lightBuffers[lightIndex].LightRadius * lightBuffers[lightIndex].LightRadius;	
	ShadowSpotFinal = lightBuffers[lightIndex].Shadow_Spot/6;	
	gl_Position = pMatrix * vMatrix * lightBuffers[lightIndex].mMatrix * vec4(vertex, 1.0); 
}