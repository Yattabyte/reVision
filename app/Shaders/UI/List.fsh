/* UI Separator Shader. */
#version 460

// Uniforms
layout(location = 0) uniform vec2 ElementPosition;
layout(location = 1) uniform vec2 ElementScale;

// Outputs
layout (location = 0) out vec4 FragColor;


void main()
{	
	const float distX = length(ElementPosition.x - gl_FragCoord.x);
	const float distY = length(ElementPosition.y - gl_FragCoord.y);
	const float rangeX = 1.0f / ElementScale.x;
	const float rangeY = 1.0f / ElementScale.y;
	const float atten = (distX * distX) * (rangeX * rangeX) * (distY * distY) * (rangeY * rangeY);
	FragColor = vec4(atten * 0.75f);
}