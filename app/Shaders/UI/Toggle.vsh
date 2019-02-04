/* UI Toggle Shader. */
#version 460

// Inputs
layout (location = 0) in vec3 vertex;
layout (location = 1) in int objIndex;

// Uniforms
layout (location = 0) uniform vec3 ElementTransform;
layout (location = 1) uniform float AnimAmount;

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
	ObjIndex = objIndex;
	vec4 vert = vec4(vertex.xyz + ElementTransform, 1);
	if (objIndex == 1)
		vert.x += AnimAmount;
	gl_Position = ScreenProjection * vert;
}