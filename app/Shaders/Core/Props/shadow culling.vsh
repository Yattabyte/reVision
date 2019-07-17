/* Prop - Geometry culling shader for shadow maps. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#package "CameraBuffer"

struct PropAttributes {
	uint materialID;
	uint skinID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
layout (std430, binding = 4) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 5) readonly buffer Prop_Index_Buffer {
	uint propIndexes[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;


void main()
{	
	const int CamIndex = camIndexes[gl_DrawID].x;
	gl_Position = camBuffer[CamIndex].pvMatrix * propBuffer[propIndexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);
	gl_Layer = camIndexes[gl_DrawID].y;
	id = gl_DrawID;
}