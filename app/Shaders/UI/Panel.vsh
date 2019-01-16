/* UI Panel Shader. */
#version 460

layout (location = 0) in vec3 vertex;
layout (location = 0) out vec2 TexCoord;
layout (location = 1) flat out float AspectRatio;

layout (location = 0) uniform mat4 ScreenProjection;
layout (location = 1) uniform vec2 ElementTransform;
layout (location = 2) uniform vec2 Scale;


void main()
{
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	AspectRatio = Scale.x / Scale.y;
	const mat4 transformMat = mat4(
		vec4(Scale.x, 0.0, 0.0, 0.0),
		vec4(0.0, Scale.y, 0.0, 0.0),
		vec4(0.0, 0.0, 1.0, 0.0),
		vec4(ElementTransform, 0.0, 1.0)
	);
	gl_Position = ScreenProjection * transformMat * vec4(vertex.xy, 0, 1);
}