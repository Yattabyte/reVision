/* UI Border Shader. */
#version 460

layout (location = 0) in vec3 vertex;

layout (location = 0) uniform mat4 ScreenProjection;
layout (location = 1) uniform vec2 ElementTransform;


void main()
{
	gl_Position = ScreenProjection * vec4(vertex.xy + ElementTransform, 0, 1);
}