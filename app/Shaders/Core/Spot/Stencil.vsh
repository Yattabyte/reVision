/* Spot light - light stenciling shader. */
#version 460

layout(location = 0) in vec4 vertex;

struct Light_Struct {
	mat4 lightV;
	mat4 lightPV;
	mat4 InverseLightPV;	
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
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

void main()
{		
	gl_Position = pMatrix * vMatrix * lightBuffers[lightIndexes[gl_InstanceID]].mMatrix * vertex;
}

