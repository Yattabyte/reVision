#version 460
#pragma optionNV(unroll all)
#define M_PI 3.1415926535897932384626433832795

layout (location = 0) out vec3 fragColor;  

layout (location = 0) in vec2 TexCoord;

// Hammersley function (return random low-discrepency points)
vec2 Hammersley(uint i, uint N)
{
	return vec2(float(i) / float(N), float(bitfieldReverse(i)) * 2.3283064365386963e-10);
}
 
vec3 ImportanceSampleGGX( in vec2 Xi, in float Roughness, in vec3 Normal )
{
	float a 			= Roughness * Roughness;
	
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

float Geometry_Schlick_Beckmann(in float Roughness, in float NdotL, in float NdotV)
{
	float k 						= Roughness * sqrt(2.0f / M_PI);
    float one_minus_k 				= 1.0f - k;
	return 							( NdotL / (NdotL * one_minus_k + k) ) * ( NdotV / (NdotV * one_minus_k + k) );
}

vec2 IntegrateBRDF( in float NoV, in float Roughness )
{
	vec3 N					= vec3(0, 0, 1.0f);
	vec3 V 					= vec3(sqrt( 1.0f - NoV * NoV ), 0, NoV);
	float A 				= 0;
	float B 				= 0;
	const uint NumSamples	= 1024;
	
	for ( uint i = 0; i < NumSamples; ++i ) {
	
		vec2 Xi 			= Hammersley( i, NumSamples );
		vec3 H 				= ImportanceSampleGGX( Xi, Roughness, N );
		vec3 L 				= 2.0f * dot( V, H ) * H - V;
		float NoL 			= max( L.z, 0.0f );
		float NoH 			= max( H.z, 0.0f );
		float VoH 			= max( dot( V, H ), 0.0f );
		
		if ( NoL > 0.0f ) {
			float G 		= Geometry_Schlick_Beckmann( Roughness, NoL, NoV );
			float G_Vis 	= (G * VoH) / (NoH * NoV);
			float Fc 		= pow( 1.0f - VoH, 5.0f );
			A 			   += (1.0f - Fc) * G_Vis;
			B 			   += Fc * G_Vis;
		}
	}
	
	return vec2( A, B ) / NumSamples;
}

void main()
{		
	fragColor		 		= vec3(IntegrateBRDF(TexCoord.x, TexCoord.y), 0.0f);
}
