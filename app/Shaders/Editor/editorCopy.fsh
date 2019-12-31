/* Editor Screen Copy shader. */
#version 460

layout (location = 0) in vec2 UV;
layout (location = 0) out vec4 fragColor;

layout (binding = 0) uniform sampler2D ScreenTexture;


void main()
{			
	fragColor = texture(ScreenTexture, UV);
}
