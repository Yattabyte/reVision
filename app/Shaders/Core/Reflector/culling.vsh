#version 460
#extension GL_ARB_shader_viewport_layer_array : require

struct PropAttributes {
	uint materialID;
	mat4 mMatrix;
	mat4 bBoxMatrix;
};
struct Reflection_Struct {
	mat4 mMatrix;
	mat4 rotMatrix;
	mat4 pMatrix;
	mat4 vMatrix[6];
	vec4 BoxCamPos;
	vec4 BoxScale;
	int CubeSpot;
};

layout (std430, binding = 3) readonly buffer Prop_Buffer {
	PropAttributes propBuffer[];
};
layout (std430, binding = 4) readonly buffer Prop_Index_Buffer {
	uint propIndex[];
};
layout (std430, binding = 8) readonly buffer Reflection_Buffer {
	Reflection_Struct reflectorBuffers[];
};

// Uniform Inputs
layout (location = 0) uniform int reflectorIndex = 0;
layout (location = 1) uniform int instance = 0;

layout (location = 0) in vec3 vertex;
layout (location = 0) flat out int id;

void main()
{	
	gl_Position = reflectorBuffers[reflectorIndex].pMatrix * reflectorBuffers[reflectorIndex].vMatrix[instance] * propBuffer[propIndex[gl_DrawID]].bBoxMatrix * vec4(vertex,1.0);		
	id = gl_DrawID;
	gl_Layer = reflectorBuffers[reflectorIndex].CubeSpot + instance;
}