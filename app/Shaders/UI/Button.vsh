/* UI Button Shader. */
#version 460

layout (location = 0) in vec3 vertex;

layout (location = 1) uniform vec2 ElementTransform;

layout (std430, binding = 2) readonly coherent buffer ProjectionBuffer { 
	mat4 ScreenProjection;
};


void main()
{
	gl_Position = ScreenProjection * vec4(vertex.xy + ElementTransform, 0, 1);
}