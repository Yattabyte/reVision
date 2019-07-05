/* Point light - light stenciling shader. */
#version 460

layout(location = 0) in vec3 vertex;

struct Light_Struct {
	mat4 shadowVP[6];
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
	float NearPlane;
	float FarPlane;
	float FOV;
};
layout (std430, binding = 3) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};

void main()
{		
	gl_Position = pMatrix * vMatrix * lightBuffers[lightIndexes[gl_InstanceID]].mMatrix * vec4(vertex, 1.0); 
}

