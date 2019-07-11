/* Screen space reflection shader - part 1 - UV Lookup Creation. */
#version 460
#package "CameraBuffer"

// SSR Variables
const float rayStep = 0.01f;
const uint maxSteps = 25;
const uint maxBinarySteps = 5u;

// The screen texture
layout (binding = 0) uniform sampler2DArray ColorMap;
layout (binding = 1) uniform sampler2DArray ViewNormalMap;
layout (binding = 2) uniform sampler2DArray SpecularMap;
layout (binding = 3) uniform sampler2DArray DepthMap;
layout (binding = 6) uniform sampler2D BayerMatrix;

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in mat4 pMatrix;
layout (location = 5) flat in mat4 pMatrixInverse;
layout (location = 9) flat in mat4 vMatrixInverse;
layout (location = 13) flat in vec2 CameraDimensions;
layout (location = 14) flat in vec3 EyePosition;

layout (location = 0) out vec4 LightingColor;


float GetDepth(vec2 uv) 
{
	return texture(DepthMap, vec3(uv, gl_Layer)).r;
}

vec3 BinarySearch(vec3 dir, vec3 maxCoord, vec3 minCoord)
{
	float depth;
	vec3 midCoord;
	vec4 projectedCoord;
	for(int n = 0; n < maxBinarySteps; ++n) {
		midCoord = mix(minCoord, maxCoord, 0.5f);
				
		// Derive UV coordinates from the hit coordinate
		projectedCoord = pMatrix * vec4(midCoord, 1.0);
		projectedCoord.xy /= projectedCoord.w;
		projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

		// Look up the depth value for where the hit coordinate appears on the screen
		const vec4 viewPos = pMatrixInverse * vec4(2.0f * vec3(projectedCoord.xy, GetDepth(projectedCoord.xy)) - 1.0f, 1.0f);
        depth = viewPos.z / viewPos.w; 

		if(midCoord.z < depth)
			maxCoord = midCoord;
		else
			minCoord = midCoord;
	}
	return vec3(projectedCoord.xy, depth);
}

vec3 RayMarch(vec3 dir, vec3 hitCoord)
{
    dir *= rayStep; 
    for(int i = 0; i < maxSteps; ++i) {
		vec3 prevCoord = hitCoord;
        hitCoord += dir;
		
		// Derive UV coordinates from the hit coordinate
        vec4 projectedCoord = pMatrix * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;		

		// Look up the depth value for where the hit coordinate appears on the screen
		const vec4 viewPos = pMatrixInverse * vec4(2.0f * vec3(projectedCoord.xy, GetDepth(projectedCoord.xy)) - 1.0f, 1.0f);
        float depth = viewPos.z / viewPos.w; 
        float dDepth = hitCoord.z - depth;

		// Check if we intersected the depth buffer
        if((dir.z - dDepth) < 1.2 && dDepth <= 0.0)
			// Refine intersection point
			return BinarySearch(dir, hitCoord, prevCoord);		
    }

	// Failed to hit anything
	return vec3(0);
}

void main(void)
{   
	// Quit Early
	const float depth =  GetDepth(TexCoord);
	LightingColor = vec4(0);
	if (depth >= 1.0f)
		discard;
	
	const vec3 viewNormal = texture(ViewNormalMap, vec3(TexCoord, gl_Layer)).rgb;
	vec4 viewPos = pMatrixInverse * vec4(2.0f * vec3(TexCoord, depth) - 1.0f, 1.0f);
	viewPos.xyz /= viewPos.w;
	const vec3 viewPosUnit = normalize(viewPos.xyz);
    const vec3 reflected = normalize(reflect(viewPosUnit, normalize(viewNormal)));	

	const vec3 dir = reflected * max(0.01f, -viewPos.z);
	const float rDotV = dot(reflected, viewPosUnit);
	const vec3 ReflectionUVS = RayMarch(dir, viewPos.xyz);
	if (ReflectionUVS.x <= 1.0f && ReflectionUVS.x >= 0.0f && ReflectionUVS.y <= 1.0f && ReflectionUVS.y >= 0.0f) 
		LightingColor = vec4(ReflectionUVS, rDotV);
}