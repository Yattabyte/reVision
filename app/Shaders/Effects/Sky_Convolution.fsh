/* Reflector - cubemap gaussian blurring shader. */
#version 460
#pragma optionNV(unroll all)
#define M_PI 3.14159265359

// Uniform Inputs
layout (binding = 0) uniform samplerCube CubeMapSampler;
layout (location = 1) uniform float roughness = 1.0f;

// Inputs
layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in int cubeFace;

// Outputs
layout (location = 0) out vec3 fragColor;  


// Hammersley function (return random low-discrepency points)
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i) / float(N), float(bitfieldReverse(i)) * 2.3283064365386963e-10);
}
 
vec3 ImportanceSampleGGX( in vec2 Xi, in vec3 Normal )
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
	const vec3 faces[6] 		= vec3[]( 
									vec3( 1, -TexCoord.y, -TexCoord.x),
									vec3( -1, -TexCoord.y, TexCoord.x),
									vec3( TexCoord.x, 1, TexCoord.y),
									vec3( TexCoord.x, -1, -TexCoord.y),
									vec3( TexCoord.x, -TexCoord.y, 1),
									vec3(-TexCoord.x, -TexCoord.y, -1) 
								);
	vec3 N 						= faces[cubeFace];
	vec3 R						= N;
	vec3 V 						= R;	
	vec3 PrefilteredColor 		= vec3(0.0f);
	float TotalWeight			= 0.0f;	
	const uint NumSamples		= 1024u;	
	for ( uint i = 0; i < NumSamples; ++i ) {	
		vec2 Xi 				= Hammersley( i, NumSamples );
		vec3 H 					= ImportanceSampleGGX( Xi, N );
		vec3 L 					= 2.0f * dot( V, H ) * H - V;
		float NdotL 			= max( dot( N, L ), 0.0f );
		
		if ( NdotL > 0.0f ) {
			PrefilteredColor   += textureLod(CubeMapSampler, L, 0.0f).rgb * NdotL;
			TotalWeight 	   += NdotL;
		}
	}	
	fragColor 					= PrefilteredColor / TotalWeight;			
}
