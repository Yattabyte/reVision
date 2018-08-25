#version 460 
#pragma optionNV (unroll all)
#package "camera"

layout (location = 0) out vec4 SSAOColor; 

layout (binding = 1) uniform sampler2D ViewNormalMap;
layout (binding = 2) uniform sampler2D NoiseMap;
layout (binding = 3) uniform sampler2D DepthMap;

layout (location = 0) uniform float SSAORadius = 1.0f;
layout (location = 1) uniform int SSAOQuality = 1;
layout (location = 2) uniform vec4 SSAOKernel[128];

in vec2 TexCoord;

const uint sampleCounts[5] = uint[](8, 16, 32, 64, 128);

vec3 Convert2ViewSpace(vec3 rawPosition)
{	
	// Convert from (0, 1) range to (-1, 1)
	const vec4 ScreenSpacePosition	= vec4(rawPosition, 1) * 2.0f - 1.0f;

	// Undo Perspective transformations to bring into View space 
    const vec4 ViewPosition 		= cameraBuffer.pMatrix_Inverse * ScreenSpacePosition;
	
	// Perform perspective divide and return
    return 							(ViewPosition.xyz / ViewPosition.w);
}

/* Bring into View Space*/
vec3 CalcPositionFromDepth(vec2 TexCoord)
{
	// Get the depth of the pixel at the tex coordinates
    const vec3 rawPosition 			= vec3(TexCoord, texture(DepthMap, TexCoord).x);
	
	// Convert from (0, 1) range to (-1, 1)
	const vec4 ScreenSpacePosition	= vec4(rawPosition, 1) * 2.0f - 1.0f;

	// Undo Perspective transformations to bring into View space 
    const vec4 ViewPosition 		= cameraBuffer.pMatrix_Inverse * ScreenSpacePosition;
	
	// Perform perspective divide and return
    return 							(ViewPosition.xyz / ViewPosition.w);
}

void main()
{		
	const float Depth 			= texture(DepthMap, TexCoord).x;
	if (Depth >= 1.0f) 			discard; // Exit Early
	const vec3 ViewPos 			= Convert2ViewSpace(vec3(TexCoord, Depth));
	const vec3 ViewNormal 		= texture(ViewNormalMap, TexCoord).xyz;
	const vec3 RandomVec 		= texture(NoiseMap, TexCoord * (cameraBuffer.CameraDimensions / 4.0f)).xyz;  		
	const vec3 ViewTangent 		= normalize(RandomVec - ViewNormal * dot(RandomVec, ViewNormal));
	const vec3 ViewBitangent 	= cross(ViewNormal, ViewTangent);
	const mat3 TBN 				= mat3(ViewTangent, ViewBitangent, ViewNormal);  
	
	float occlusion = 0.0;	
	const uint sampleCount = sampleCounts[SSAOQuality];	
	for (int i = 0 ; i < sampleCount; ++i) {
		vec3 samplePos 			= TBN * SSAOKernel[i].xyz; 	// From tangent to view-space
		samplePos 				= ViewPos + samplePos * SSAORadius; 
		
		// project Sample position (to Sample texture) (to get position on screen/texture)
		vec4 offset 			= vec4(samplePos, 1.0);
		offset 					= cameraBuffer.pMatrix * offset; 		// from view to clip-space
		offset.xy 	   	   	   /= offset.w; 				// perspective divide
		offset.xy 				= offset.xy * 0.5 + 0.5; 	// transform to range 0.0 - 1.0
		
		// get Sample depth
		const float sampleDepth = CalcPositionFromDepth(offset.xy).z; // Get depth value of kernel Sample        
		const float rangeCheck 	= smoothstep(0.0, 1.0, SSAORadius / abs(ViewPos.z - sampleDepth));
		occlusion 			   += (sampleDepth >= samplePos.z + 0.025f ? 1.0f : 0.0f) * rangeCheck;  
	}

	occlusion 					= 1.0 - (occlusion / sampleCount);
	SSAOColor					= vec4(occlusion);
}