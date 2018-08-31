#version 460
#extension GL_ARB_bindless_texture : require

layout (location = 0) in mat3 WorldTBN;
layout (location = 4) in vec2 TexCoord0;
layout (location = 5) flat in vec3 ColorModifier;
layout (location = 6) flat in sampler2DArray MaterialMap;
layout (location = 7) flat in uint MaterialOffset;

layout (location = 0) out vec3 WorldNormalOut; 
layout (location = 1) out vec3 RadiantFluxOut; 

void main()									
{			
	vec4 GColor					= texture(MaterialMap, vec3(TexCoord0, MaterialOffset + 0));
	if (GColor.a < 0.01f) 		discard;
	
	vec3 BumpMapNormal 			= normalize(texture(MaterialMap, vec3(TexCoord0, MaterialOffset + 1)).xyz * 2.0 - 1.0);
	vec3 WorldNormal			= normalize(WorldTBN * BumpMapNormal);	
	
	gl_FragDepth 				= gl_FragCoord.z;
	WorldNormalOut				= WorldNormal;	
	RadiantFluxOut				= GColor.xyz * ColorModifier;
}