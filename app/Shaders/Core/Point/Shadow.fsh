#version 460
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 WorldPos;
layout (location = 1) in vec3 LightPos;
layout (location = 2) in float FarPlane;
layout (location = 3) in mat3 WorldTBN;
layout (location = 7) in vec2 TexCoord0;
layout (location = 8) flat in vec3 ColorModifier;
layout (location = 9) flat in sampler2DArray MaterialMap;
layout (location = 10) flat in uint MaterialOffset;

layout (location = 0) out float LightDistance;
layout (location = 1) out vec3 WorldNormalOut; 
layout (location = 2) out vec3 RadiantFluxOut;

void main()									
{		
	vec4 GColor					= texture(MaterialMap, vec3(TexCoord0, MaterialOffset + 0));
	if (GColor.a < 0.01f) 		discard;
	
	vec3 BumpMapNormal 			= normalize(texture(MaterialMap, vec3(TexCoord0, MaterialOffset + 1)).xyz * 2.0 - 1.0);
	vec3 WorldNormal			= normalize(WorldTBN * BumpMapNormal);	
	
	LightDistance 				= length(WorldPos - LightPos) / FarPlane;
	WorldNormalOut				= WorldNormal;
	RadiantFluxOut				= GColor.xyz * ColorModifier;
}