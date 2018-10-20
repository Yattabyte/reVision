/* Directional light - geometry culling shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : require
#define NUM_CASCADES 4

struct PropAttributes {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
struct Shadow_Struct {
	mat4 lightV;
	int Shadow_Spot;
	float CascadeEndClipSpace[NUM_CASCADES];
	mat4 LightVP[NUM_CASCADES];
	mat4 InverseLightVP[NUM_CASCADES];	
};

layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Visibility_Buffer {
	uint propIndexes[];
};
layout (std430, binding = 9) readonly buffer Shadow_Buffer {
	Shadow_Struct shadowBuffers[];
};
layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

layout (location = 0) uniform int LightIndex = 0;
layout (location = 1) uniform int ShadowIndex = 0;

void main()
{	
	gl_Position = shadowBuffers[ShadowIndex].LightVP[gl_InstanceID] * propBuffer[propIndexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);		
	gl_Layer = shadowBuffers[ShadowIndex].Shadow_Spot + gl_InstanceID;
	id = gl_DrawID;
}

