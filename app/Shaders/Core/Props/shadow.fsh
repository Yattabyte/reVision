/* Prop - Geometry shadowing shader. */
#version 460

layout (std430, binding = 2) buffer Camera_Buffer {		
	mat4 pMatrix;
	mat4 vMatrix;
	vec3 EyePosition;
	vec2 CameraDimensions;
	float NearPlane;
	float FarPlane;
	float FOV;
};

layout (binding = 0) uniform sampler2DArray MaterialMap;

layout (location = 0) in vec3 WorldPos;
layout (location = 1) in mat3 WorldTBN;
layout (location = 5) in vec2 TexCoord0;
layout (location = 6) flat in uint MaterialOffset;

layout (location = 0) out vec3 WorldNormalOut; 
layout (location = 1) out vec3 RadiantFluxOut; 
layout (location = 2) out float LinearDistance; 


void main()									
{
	vec4 GColor					= texture(MaterialMap, vec3(TexCoord0, MaterialOffset + 0));
	if (GColor.a < 0.01f) 		discard;
	
	vec3 BumpMapNormal 			= normalize(texture(MaterialMap, vec3(TexCoord0, MaterialOffset + 1)).xyz * 2.0 - 1.0);
	vec3 WorldNormal			= normalize(WorldTBN * BumpMapNormal);	
	
	WorldNormalOut				= WorldNormal;	
	RadiantFluxOut				= GColor.xyz;
	LinearDistance 				= length(WorldPos - EyePosition) / FarPlane;
}