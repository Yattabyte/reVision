/* Wireframe shader. */
#version 460

layout (location = 0) in vec2 UV;
layout (location = 0) out vec4 fragColor;

layout (location = 4) uniform vec3 Color;


void main()
{			
	fragColor = vec4(Color, 1.0f);
}
