/* Directional light - geometry culling shader. */
#version 460
#extension GL_ARB_shader_viewport_layer_array : enable
#define NUM_CASCADES 4

struct PropAttributes {
	uint materialID;
	uint isStatic;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
struct Light_Struct {
	mat4 lightV;	
	mat4 LightVP[NUM_CASCADES];
	mat4 InverseLightVP[NUM_CASCADES];	
	float CascadeEndClipSpace[NUM_CASCADES];
	vec4 LightColor;
	vec4 LightDirection;
	float LightIntensity;
	int Shadow_Spot;
};

layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Visibility_Buffer {
	uint propIndexes[];
};
layout (std430, binding = 8) readonly buffer Light_Buffer {
	Light_Struct lightBuffers[];
};
layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

layout (location = 0) uniform int LightIndex = 0;

void main()
{	
	gl_Position = lightBuffers[LightIndex].LightVP[gl_InstanceID] * propBuffer[propIndexes[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);		
	gl_Layer = lightBuffers[LightIndex].Shadow_Spot + gl_InstanceID;
	id = gl_DrawID;
}

