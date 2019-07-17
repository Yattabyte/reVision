/* Spot light - light stenciling shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

layout(location = 0) in vec4 vertex;

struct Light_Struct {
	mat4 lightPV;
	mat4 mMatrix;
	vec4 LightColor;
	vec4 LightPosition;
	vec4 LightVector;
	float LightIntensity;
	float LightRadius;
	float LightCutoff;
	int Shadow_Spot;	
};
layout (std430, binding = 4) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};


void main()
{		
	const int CamIndex = camIndexes[gl_InstanceID].x;
	gl_Position = camBuffer[CamIndex].pvMatrix * lightBuffers[lightIndexes[gl_InstanceID]].mMatrix * vertex;
	gl_Layer = camIndexes[gl_InstanceID].y;
}

