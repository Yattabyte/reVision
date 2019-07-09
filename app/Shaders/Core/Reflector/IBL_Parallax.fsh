/* Reflector - lighting shader. */
#version 460
#pragma optionNV(fastmath on)
#pragma optionNV(ifcvt none)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)
layout (early_fragment_tests) in;
#define M_PI 3.14159f
#package "CameraBuffer"

layout (binding = 0) uniform sampler2DArray ColorMap;
layout (binding = 1) uniform sampler2DArray ViewNormalMap;
layout (binding = 2) uniform sampler2DArray SpecularMap;
layout (binding = 3) uniform sampler2DArray DepthMap;
layout (binding = 4) uniform samplerCubeArray ReflectionMap;

layout (location = 0) flat in mat4 pMatrixInverse;
layout (location = 4) flat in mat4 vMatrixInverse;
layout (location = 8) flat in vec2 CameraDimensions;
layout (location = 9) flat in mat4 boxMatrix;
layout (location = 13) flat in mat4 rotMatrix;
layout (location = 17) flat in vec4 BoxCamPos;
layout (location = 18) flat in vec4 BoxScale;
layout (location = 19) flat in int CubeSpot;

layout (location = 0) out vec3 ReflectionColor;

struct ViewData {
	vec4 World_Pos;
	vec4 View_Pos;
	vec3 World_Normal;
	vec3 View_Normal;
	vec3 Albedo;
	float Metalness;
	float Roughness;
	float View_Depth;
	float View_AO;
};


void GetFragmentData(in vec2 TexCoord, out ViewData data)
{
	const vec4 Texture1				= texture(ColorMap, vec3(TexCoord, gl_Layer));
	const vec4 Texture2				= texture(ViewNormalMap, vec3(TexCoord, gl_Layer));
	const vec4 Texture3				= texture(SpecularMap, vec3(TexCoord, gl_Layer));
	const vec4 Texture4				= texture(DepthMap, vec3(TexCoord, gl_Layer));
	
	data.Albedo 					= Texture1.rgb;
	data.View_Normal				= Texture2.rgb;
	data.Metalness					= Texture3.r;
	data.Roughness					= Texture3.g;
	float View_Height				= Texture3.b;
	data.View_AO					= Texture3.a;
	data.View_Depth					= Texture4.r;
	
	data.View_Pos					= pMatrixInverse * vec4(vec3(TexCoord, data.View_Depth) * 2.0f - 1.0f, 1.0f);
    data.World_Pos 					= vMatrixInverse * data.View_Pos;
	data.View_Pos 					= data.View_Pos / data.View_Pos.w;
	data.World_Pos 					= data.World_Pos / data.World_Pos.w;
    data.World_Normal 				= normalize((vMatrixInverse * vec4(data.View_Normal, 0))).xyz;
}

vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / CameraDimensions;
}

vec3 ParallaxCorrectCubemap(in vec3 ReflectDir, in mat4 bboxMat, in vec3 bboxPos, in vec3 WorldPos)
{   
	mat4 InvBBoxMat					= inverse(bboxMat);
	vec3 RayLS						= mat3(InvBBoxMat) * ReflectDir;
    vec3 PositionLS 				= (InvBBoxMat * vec4(WorldPos, 1)).xyz;
	
	vec3 MaxPlaneIntersection 		= (vec3(1) - PositionLS) / RayLS;
    vec3 MinPlaneIntersection 		= (vec3(-1) - PositionLS) / RayLS;
   
    vec3 LargestIntersection		= max(MaxPlaneIntersection, MinPlaneIntersection);	
	float ClosestPoint				= min(min(LargestIntersection.x, LargestIntersection.y), LargestIntersection.z);
	
    vec3 IntersectionPos 			= WorldPos.xyz + ReflectDir * ClosestPoint;
	
    return							(IntersectionPos - bboxPos);
}

vec3 CalculateReflections(in vec3 WorldPos, in vec3 ViewPos, in vec3 ViewNormal, in float Roughness)
{		
	vec3 ReflectDir					= reflect(ViewPos, ViewNormal);
		 ReflectDir 				= normalize(vMatrixInverse * vec4(ReflectDir, 0)).xyz;
	vec3 CorrectedDir				= ParallaxCorrectCubemap
									(	ReflectDir, 
										boxMatrix,
										BoxCamPos.xyz, 
										WorldPos.xyz	);	
	return							textureLod(ReflectionMap, vec4(CorrectedDir, CubeSpot), Roughness * 5.0f).xyz;		
}

void main(void)
{   
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
	
	vec4 rotWorldPos					= rotMatrix * vec4(data.World_Pos.xyz, 1.0f);
	rotWorldPos.xyz /= rotWorldPos.w;
	const vec3 BBoxMin 					= BoxCamPos.xyz - BoxScale.xyz;
	const vec3 BBoxMax 					= BoxCamPos.xyz + BoxScale.xyz;
	if (rotWorldPos.x < BBoxMin.x || rotWorldPos.y < BBoxMin.y || rotWorldPos.z < BBoxMin.z
	|| rotWorldPos.x > BBoxMax.x || rotWorldPos.y > BBoxMax.y || rotWorldPos.z > BBoxMax.z)
		discard;
	
	ReflectionColor						= CalculateReflections(data.World_Pos.xyz, data.View_Pos.xyz, data.View_Normal, data.Roughness);
}