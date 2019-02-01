/* UI List Shader. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in int objIndex;

layout (location = 1) flat out int Index;

layout (location = 0) uniform vec2 ElementTransform;
layout (location = 1) uniform vec4 Transforms[3];

layout (std430, binding = 2) readonly coherent buffer ProjectionBuffer { 
	mat4 ScreenProjection;
};


void main()
{
	Index = objIndex;
	const vec4 vert = vec4((vertex.xy * Transforms[objIndex].xy) + Transforms[objIndex].zw + ElementTransform, 0, 1);	
	gl_Position = ScreenProjection * vert;
}