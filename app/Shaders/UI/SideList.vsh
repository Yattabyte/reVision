/* UI DropList Shader. */
#version 460

// Inputs
layout (location = 0) in vec3 vertex;
layout (location = 1) in int objIndex;

// Uniforms
layout (location = 0) uniform vec3 ElementTransform;

// Buffers
layout (std430, binding = 2) readonly coherent buffer ProjectionBuffer { 
	mat4 ScreenProjection;
};

// Outputs
layout (location = 1) flat out int ObjIndex;


void main()
{
	ObjIndex = objIndex;	
	gl_Position = ScreenProjection * vec4(vertex + ElementTransform, 1);
}