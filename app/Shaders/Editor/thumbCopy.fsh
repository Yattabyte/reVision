/* Thumbnail Copy shader - copies from texture array into a single texture. */
#version 460 

layout (location = 0) out vec3 destinationTexture;

layout (binding = 0) uniform sampler2DArray TargetTexture;
layout (location = 0) uniform int TargetLayer;

layout (location = 0) in vec2 TexCoord;


void main()
{		
	destinationTexture = texture(TargetTexture, vec3(TexCoord, TargetLayer)).rgb;	
}