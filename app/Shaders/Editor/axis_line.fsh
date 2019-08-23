/* Axis lines shader. */
#version 460

layout (location = 0) out vec4 fragColor;

layout (location = 4) uniform vec3 color;

void main()
{			
	fragColor = vec4(color, 1.0f);
}
