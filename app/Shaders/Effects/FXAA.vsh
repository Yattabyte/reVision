#version 460
#package "camera"

layout(location = 0) in vec3 vertex;

layout (location = 0) out vec2 TexOffset;
layout (location = 1) out vec2 TexCoord;

void main()
{		
	TexOffset = 1.0f / cameraBuffer.CameraDimensions;
	TexCoord = (vertex.xy + vec2(1.0)) / 2.0;
	gl_Position = vec4(vertex.xyz, 1);	
}