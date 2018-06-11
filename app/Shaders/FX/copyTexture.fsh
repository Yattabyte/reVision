#version 460 

layout (location = 0) out vec3 destinationTexture;
layout (binding = 0) uniform sampler2D TargetTexture;

in vec2 TexCoord;

void main()
{		
	destinationTexture = texture(TargetTexture, TexCoord).rgb;	
}