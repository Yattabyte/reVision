#version 460
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec2 TexCoord;
layout (location = 1) in mat3 ViewTBN;
layout (location = 5) flat in sampler2DArray MaterialMap;

layout (location = 0) out vec4 FirstTexture; 
layout (location = 1) out vec4 SecondTexture; 
layout (location = 2) out vec4 ThirdTexture; 

void main()									
{		
	const vec4 Texture1			= texture(MaterialMap, vec3(TexCoord, 0));	
	if (Texture1.a < 0.01f) 	discard;
	
	FirstTexture.xyz			= pow(Texture1.rgb, vec3(2.2f));	
	SecondTexture.xyz       	= normalize(ViewTBN * normalize(texture(MaterialMap, vec3(TexCoord, 1)).xyz * 2.0f - 1.0f));				
	ThirdTexture.xyz        	= texture(MaterialMap, vec3(TexCoord, 2)).rgb;
}