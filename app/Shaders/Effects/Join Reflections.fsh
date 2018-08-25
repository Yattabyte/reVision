#version 460 

layout (location = 0) out vec3 destinationTexture;
layout (binding = 0) uniform sampler2D TargetTexture;
layout (binding = 2) uniform sampler2D GBufferTexture2;

layout (location = 0) in vec2 TexCoord;

void main()
{		
	// Apply AO in this separate pass
	destinationTexture = texture(TargetTexture, TexCoord).rgb * texture(GBufferTexture2, TexCoord).a;	
}