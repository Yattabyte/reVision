/* UI Button Shader. */
#version 460

layout (location = 0) in vec2 TexCoord;
layout (location = 1) flat in float AspectRatio;
layout (location = 3) uniform bool Highlighted;
layout (location = 4) uniform bool Pressed;

layout (location = 0) out vec4 FragColor;


void main()
{		
	vec3 color = Highlighted ? vec3(225,200,25) : vec3(0.0f);
	if (Pressed)
		color = vec3(25,100,25);	
	else 
		color += vec3(50,200,50);
	FragColor = vec4(color, 255) / 255.0F;	
}