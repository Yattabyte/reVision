/* Merges post-process AO into GBuffer AO. */
#version 460 

layout (location = 0) out vec4 destinationTexture;
layout (binding = 0) uniform sampler2D SourceTexture;

layout (location = 0) in vec2 TexCoord;

void main()
{		
	destinationTexture.a = texture(SourceTexture, TexCoord).r;	
}