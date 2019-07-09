/* Reflector - cubemap gaussian blurring shader. */
#version 460
#define M_PI 3.14159265359

// Uniform Inputs
layout (binding = 0) uniform sampler2DArray CubeMapSampler;
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

vec3 sampleCube(vec3 v)
{
	vec3 vAbs = abs(v);
	float ma;
	vec2 uv;
	float faceIndex;
	if(vAbs.z >= vAbs.x && vAbs.z >= vAbs.y) {
		faceIndex = v.z < 0.0 ? 5.0 : 4.0;
		ma = 0.5 / vAbs.z;
		uv = vec2(v.z < 0.0 ? -v.x : v.x, -v.y);
	}
	else if(vAbs.y >= vAbs.x) {
		faceIndex = v.y < 0.0 ? 3.0 : 2.0;
		ma = 0.5 / vAbs.y;
		uv = vec2(v.x, v.y < 0.0 ? -v.z : v.z);
	}
	else {
		faceIndex = v.x < 0.0 ? 1.0 : 0.0;
		ma = 0.5 / vAbs.x;
		uv = vec2(v.x < 0.0 ? v.z : -v.z, -v.y);
	}
	return vec3(uv * ma + 0.5, faceIndex);
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
			PrefilteredColor   += textureLod(CubeMapSampler, sampleCube(L) + vec3(0, 0, cubeOffset), 0.0f).rgb * NdotL;
			TotalWeight 	   += NdotL;
		}
	}	
	fragColor 					= PrefilteredColor / TotalWeight;
}
