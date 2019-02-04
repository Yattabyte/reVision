/* UI Scrollbar Shader. */
#version 460

// Inputs
layout (location = 0) in vec3 vertex;

// Uniforms
layout (location = 0) uniform vec3 ElementTransform;

// Buffers
layout (std430, binding = 2) readonly coherent buffer ProjectionBuffer { 
	mat4 ScreenProjection;
};

// Outputs
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out int ObjIndex;


void main()
{
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = ScreenProjection * vec4(vertex.xyz + ElementTransform, 1);
}