#version 460
#package "Lighting\lighting_pbr"

struct Reflection_Struct {
	mat4 mMatrix;
	vec4 BoxCamPos;
	float Radius;
	int CubeSpot;
};
layout (std430, binding = 3) readonly buffer Visibility_Buffer {
	uint indexes[];
};
layout (std430, binding = 5) readonly buffer Reflection_Buffer {
	Reflection_Struct buffers[];
};

layout (location = 0) in vec3 CubeWorldPos;
layout (location = 1) flat in uint BufferIndex;

layout (binding = 4) uniform samplerCubeArray TemporaryMap;
layout (location = 0) out vec4 LightingColor;
layout (location = 0) uniform bool useStencil = false;


vec2 CalcTexCoord()
{
    return			 				gl_FragCoord.xy / CameraDimensions;
}

vec3 ParallaxCorrectCubemap(in vec3 ReflectDir, in mat4 bboxMat, in vec3 bboxPos, in vec3 WorldPos)
{   
	mat4 InvBBoxMat					= inverse(bboxMat);
	vec3 RayLS						= mat3(InvBBoxMat) * ReflectDir;
    vec3 PositionLS 				= (InvBBoxMat * vec4(WorldPos, 1)).xyz;
    vec3 Unitary					= vec3(1);
	
	vec3 MaxPlaneIntersection 		= (Unitary - PositionLS) / RayLS;
    vec3 MinPlaneIntersection 		= (-Unitary - PositionLS) / RayLS;
   
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
	return							texture(TemporaryMap, vec4(CorrectedDir, Roughness * 5.0f)).xyz;		
}

void main(void)
{   
	LightingColor						= vec4(0);	
	if (useStencil) 					return;
	
	ViewData data;
	GetFragmentData(CalcTexCoord(), data);
	
	const float Distance				= length(buffers[indexes[BufferIndex]].BoxCamPos.xyz - data.World_Pos.xyz);	
	const float range 					= (1.0f / buffers[indexes[BufferIndex]].Radius);
	const float Attenuation 			= 1.0f - (Distance * Distance) * (range * range);	
	if (Attenuation <= 0.0f) 			return;// Discard if outside of radius
	
	const vec3 ReflectionColor			= CalculateReflections(data.World_Pos.xyz, data.View_Pos.xyz, data.View_Normal, data.Roughness);
	LightingColor						= vec4(10, 0, 0, Attenuation);	
}