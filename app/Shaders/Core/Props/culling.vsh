/* Prop - Geometry culling shader. */
#version 460
#package "camera"

struct PropAttributes {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};

layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Prop_Index_Buffer {
	uint propIndexes[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

void main()
{	
	gl_Position = cameraBuffer.pMatrix * cameraBuffer.vMatrix * propBuffer[propIndexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);		
	id = gl_DrawID;
}