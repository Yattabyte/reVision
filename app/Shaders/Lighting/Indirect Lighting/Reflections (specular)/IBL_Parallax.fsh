#version 460
#package "Lighting\lighting_pbr"

struct Reflection_Struct {
	mat4 mMatrix;
	mat4 rotMatrix;
	vec4 BoxCamPos;
	vec4 BoxScale;
	int CubeSpot;
};
layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};
layout (std430, binding = 5) readonly buffer Reflection_Buffer {
	Reflection_Struct buffers[];
};

layout (location = 0) flat in uint BufferIndex;

layout (binding = 4) uniform samplerCubeArray TemporaryMap;
layout (location = 0) out vec4 LightingColor;
layout (location = 0) uniform bool UseStencil = false;


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
		 ReflectDir 				= normalize(vMatrix_Inverse * vec4(ReflectDir, 0)).xyz;
	vec3 CorrectedDir				= ParallaxCorrectCubemap
									(	ReflectDir, 
										buffers[indexes[BufferIndex]].mMatrix,
										buffers[indexes[BufferIndex]].BoxCamPos.xyz, 
										WorldPos.xyz	);	
	//return							texture(TemporaryMap, vec4(CorrectedDir, Roughness * 5.0f)).xyz;	
	return							textureLod(TemporaryMap, vec4(CorrectedDir, buffers[indexes[BufferIndex]].CubeSpot), Roughness * 5.0f).xyz;		
}

void main(void)
{   
	LightingColor						= vec4(0.0f);	
	if (UseStencil) 					return;
	
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
	
	vec4 rotWorldPos					= buffers[indexes[BufferIndex]].rotMatrix * vec4(data.World_Pos.xyz, 1.0f);
	rotWorldPos.xyz /= rotWorldPos.w;
	const vec3 BBoxMin 					= buffers[indexes[BufferIndex]].BoxCamPos.xyz - buffers[indexes[BufferIndex]].BoxScale.xyz;
	const vec3 BBoxMax 					= buffers[indexes[BufferIndex]].BoxCamPos.xyz + buffers[indexes[BufferIndex]].BoxScale.xyz;
	if (rotWorldPos.x < BBoxMin.x || rotWorldPos.y < BBoxMin.y || rotWorldPos.z < BBoxMin.z
	|| rotWorldPos.x > BBoxMax.x || rotWorldPos.y > BBoxMax.y || rotWorldPos.z > BBoxMax.z)
		discard;
	
	const vec3 ReflectionColor			= CalculateReflections(data.World_Pos.xyz, data.View_Pos.xyz, data.View_Normal, data.Roughness);
	LightingColor						= vec4(ReflectionColor, 1.0f);	
}