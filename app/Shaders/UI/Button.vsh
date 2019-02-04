/* UI Button Shader. */
#version 460

// Inputs
layout (location = 0) in vec3 vertex;

// Uniforms
layout (location = 0) uniform vec3 ElementTransform;

// Buffers
layout (std430, binding = 2) readonly coherent buffer ProjectionBuffer { 
	mat4 ScreenProjection;
};


void main()
{
	gl_Position = ScreenProjection * vec4(vertex.xyz + ElementTransform, 1);
}