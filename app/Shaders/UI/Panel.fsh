/* UI Panel Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in float AspectRatio;

layout (location = 0) out vec4 FragColor;


void main()
{	
	const float border_width = 0.005f;
	const float 
		maxY = 1.0f - border_width,
		minY = border_width,
		maxX = 1.0f - (border_width / AspectRatio),
		minX = border_width / AspectRatio;
	vec3 color = vec3(0.25f);
	if (TexCoord.x >= maxX || TexCoord.x <= minX || TexCoord.y >= maxY || TexCoord.y <= minY)		
		color = vec3(color / 3.0f);
	FragColor = vec4(color, 1);
}