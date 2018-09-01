/* Copies an image from binding 0, to location 0. */
#version 460 

layout (location = 0) out vec3 destinationTexture;
layout (binding = 0) uniform sampler2D TargetTexture;

layout (location = 0) in vec2 TexCoord;

void main()
{		
	destinationTexture = texture(TargetTexture, TexCoord).rgb;	
}