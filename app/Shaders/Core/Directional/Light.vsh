/* Directional light - lighting shader. */
#version 460

layout (location = 0) in vec3 vertex;

layout (std430, binding = 3) readonly buffer Light_Index_Buffer {
	int lightIndexes[];
};
layout (std430, binding = 4) readonly buffer Shadow_Index_Buffer {
	int shadowIndexes[];
};

layout (location = 0) out vec2 TexCoord;

layout (location = 1) flat out int LightIndex;
layout (location = 2) flat out int ShadowIndex;

void main()
{	
	LightIndex = lightIndexes[gl_InstanceID];
	ShadowIndex = shadowIndexes[gl_InstanceID];
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = vec4(vertex, 1);
}