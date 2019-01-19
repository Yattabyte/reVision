/* UI Panel Shader. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 1) in int objIndex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out int Index;

layout (location = 0) uniform mat4 ScreenProjection;
layout (location = 1) uniform vec2 ElementTransform;


void main()
{
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	Index = objIndex;
	gl_Position = ScreenProjection * vec4(vertex.xy + ElementTransform, 0, 1);
}