/* Reflector - cubemap gaussian blurring shader. */
#version 460
#define M_PI 3.14159265359

// Uniform Inputs
layout (binding = 0) uniform samplerCubeArray CubeMapSampler;
layout (location = 1) uniform float roughness = 1.0f;

// Inputs
layout (location = 0) in vec3 TexCoord;
layout (location = 1) flat in int cubeOffset;

// Outputs
layout (location = 0) out vec3 fragColor;  


// Hammersley function (return random low-discrepency points)
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i) / float(N), float(bitfieldReverse(i)) * 2.3283064365386963e-10);
}
 
vec3 ImportanceSampleGGX(vec2 Xi, vec3 Normal)
{
	float a 			= roughness * roughness;
	
	float Phi 			= 2.0f * M_PI * Xi.x;
	float CosTheta 		= sqrt( (1.0f - Xi.y) / ( 1.0f + (a * a - 1.0f) * Xi.y ) );
	float SinTheta 		= sqrt( 1.0f - CosTheta * CosTheta );
	
	vec3 H 				= vec3( SinTheta * cos( Phi ), SinTheta * sin( Phi ), CosTheta);
	Normal 				= normalize(Normal);
	vec3 UpVector 		= abs(Normal.z) < 0.999f ? vec3(0, 0, 1) : vec3(1, 0, 0);
	vec3 Tangent 		= normalize( cross( UpVector, Normal ) );
	vec3 Bitangent 		= cross( Normal, Tangent );
	
	// Tangent to world space
	return 				normalize(Tangent * H.x + Bitangent * H.y + Normal * H.z);
} 

void main()
{
	vec3 PrefilteredColor 		= vec3(0.0f);
	float TotalWeight			= 0.0f;	
	for ( uint i = 0; i < 512; ++i ) {
		vec2 Xi 				= Hammersley( i, 512 );
		vec3 H 					= ImportanceSampleGGX( Xi, TexCoord );
		vec3 L 					= 2.0f * dot( TexCoord, H ) * H - TexCoord;
		float NdotL 			= max( dot( TexCoord, L ), 0.0f );
		
		if ( NdotL > 0.0f ) {
			PrefilteredColor   += textureLod(CubeMapSampler, vec4(L, cubeOffset), 0.0f).rgb * NdotL;
			TotalWeight 	   += NdotL;
		}
	}	
	fragColor 					= PrefilteredColor / TotalWeight;
}
