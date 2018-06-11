#version 460

out vec3 FXAAColor;
layout (binding = 0) uniform sampler2D Screen;
/*
layout (location = ) uniform float FXAA_SPAN_MAX		= 10.0;
layout (location = ) uniform float FXAA_REDUCE_MIN 		= 1.0 / 128.0;
layout (location = ) uniform float FXAA_REDUCE_MUL		= 1.0 / 8.0;
*/
layout (location = 0) uniform float FXAA_SPAN_MAX		= 7.0;
layout (location = 1) uniform float FXAA_REDUCE_MIN 	= 1.0 / 15.0;
layout (location = 2) uniform float FXAA_REDUCE_MUL		= 1.0 / 7.0;
in vec2 TexOffset;
in vec2 TexCoord;

void main()
{			
	vec3 luma 				= vec3(0.299, 0.587, 0.114);
	float lumaTL 			= dot(luma, texture( Screen, TexCoord + (vec2(-1.0, -1.0) * TexOffset) ).xyz);
	float lumaTR 			= dot(luma, texture( Screen, TexCoord + (vec2(1.0, -1.0) * TexOffset) ).xyz);
	float lumaBL 			= dot(luma, texture( Screen, TexCoord + (vec2(-1.0, 1.0) * TexOffset) ).xyz);
	float lumaBR 			= dot(luma, texture( Screen, TexCoord + (vec2(1.0, 1.0) * TexOffset) ).xyz);
	float lumaM 			= dot(luma, texture( Screen, TexCoord ).xyz);
	
	// Blurring Direction
	vec2 dir; // Invert y not x???
	dir.x 					= -((lumaTL + lumaTR) - (lumaBL + lumaBR));	
	dir.y 					= ((lumaTL + lumaBL) - (lumaTR + lumaBR));	
	
	// Blurring Magnitude
	float dirReduce 		= max( (lumaTL + lumaTR + lumaBL + lumaBR) * (FXAA_REDUCE_MUL * 0.25), FXAA_REDUCE_MIN );
	float invDirAdjusment 	= 1.0 / (min( abs(dir.x), abs(dir.y) ) + dirReduce);
	dir 					= min(	
									vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), 
									max(
											vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), 
											dir * invDirAdjusment
										)
								) * TexOffset;
	// Blur
	vec3 result1 			= (1.0 / 2.0) * (
												texture( Screen, TexCoord + (dir * vec2(1.0/3.0 - 0.5)) ).xyz +
												texture( Screen, TexCoord + (dir * vec2(2.0/3.0 - 0.5)) ).xyz 
											);								
	vec3 result2 			= result1 * (1.0 / 2.0) + (1.0 / 4.0) * (
												texture( Screen, TexCoord + (dir * vec2(0.0/3.0 - 0.5)) ).xyz +
												texture( Screen, TexCoord + (dir * vec2(3.0/3.0 - 0.5)) ).xyz 
											);
											
	// if result2 is sampled too far away, (1/4th) use luma1 (1/6th, tighter)
	float lumaMin 			= min(lumaM, min(min(lumaTL, lumaTR), min(lumaBL, lumaBR)));
	float lumaMax 			= max(lumaM, max(max(lumaTL, lumaTR), max(lumaBL, lumaBR)));
	float lumaResult2 		= dot(luma, result2); 
	
	if (lumaResult2 < lumaMin || lumaResult2 > lumaMax)
		FXAAColor = result1;
	else
		FXAAColor = result2;
}