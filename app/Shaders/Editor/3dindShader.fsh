/* 3D Indicator shader. */
#version 460

layout (location = 0) in vec2 UV;
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D ColorPalette;


void main()
{			
	fragColor = texture(ColorPalette, UV);
}