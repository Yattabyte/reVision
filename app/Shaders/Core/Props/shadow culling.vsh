/* Prop - Geometry culling shader for shadow maps. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable

struct CamAttributes {
	float farPlane;
	int layer;
	vec3 eyePosition;
	mat4 pvMatrix;		
};
struct PropAttributes {
	uint materialID;
	uint isStatic;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};

layout (std430, binding = 2) readonly buffer Cam_Buffer {
	CamAttributes camBuffer[];
};
layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Cam_Index_Buffer {
	uint camIndexes[];
};
layout (std430, binding = 5) readonly buffer Prop_Index_Buffer {
	uint propIndexes[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

void main()
{	
	gl_Position = camBuffer[camIndexes[gl_DrawID]].pvMatrix * propBuffer[propIndexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);
	gl_Layer = camBuffer[camIndexes[gl_DrawID]].layer;
	id = gl_DrawID;
}