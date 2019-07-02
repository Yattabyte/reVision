/* Prop - Geometry culling shader for shadow maps. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable

struct PropAttributes {
	uint materialID;
	uint isStatic;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
layout (std430, binding = 2) readonly buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	vec3 EyePosition;
	vec2 CameraDimensions;
	float NearPlane;
	float FarPlane;
	float FOV;
};
layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Prop_Index_Buffer {
	uint propIndexes[];
};

layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;
layout (location = 0) uniform int layer = 0;

void main()
{	
	gl_Position = pMatrix * vMatrix * propBuffer[propIndexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);
	gl_Layer = layer;
	id = gl_DrawID;
}