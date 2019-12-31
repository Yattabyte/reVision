/* Wireframe shader. */
#version 460

layout (location = 0) out vec4 fragColor;
layout (location = 3) uniform vec4 color;

void main()
{			
	fragColor = color;		
}
