/* UI DropList Shader. */
#version 460

// Inputs
layout (location = 0) in vec3 vertex;
layout (location = 1) in int objIndex;

// Uniforms
layout (location = 0) uniform vec3 ElementTransform;
layout (location = 1) uniform vec4 Transforms[2];

// Buffers
layout (std430, binding = 2) readonly coherent buffer ProjectionBuffer { 
	mat4 ScreenProjection;
};

// Outputs
layout (location = 1) flat out int ObjIndex;


void main()
{
	ObjIndex = objIndex;
	const vec4 vert = vec4((vertex.xy * Transforms[objIndex].xy) + Transforms[objIndex].zw + ElementTransform.xy, ElementTransform.z, 1);	
	gl_Position = ScreenProjection * vert;
}