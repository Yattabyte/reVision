/* Reflector - light stenciling shader. */
#version 460

layout (location = 0) in vec3 vertex;

struct Reflection_Struct {
	mat4 mMatrix;
	mat4 rotMatrix;
	vec4 BoxCamPos;
	vec4 BoxScale;
	int CubeSpot;
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
layout (std430, binding = 3) readonly buffer Reflection_Index_Buffer {
	uint reflectionIndexes[];
};
layout (std430, binding = 8) readonly buffer Reflection_Buffer {
	Reflection_Struct reflectorBuffers[];
};

void main(void)
{	
	gl_Position 		= pMatrix * vMatrix * reflectorBuffers[reflectionIndexes[gl_InstanceID]].mMatrix * vec4(vertex, 1);	
}

